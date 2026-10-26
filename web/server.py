#!/usr/bin/env python3
"""Local web interface for the repository's original C++ leaderboard CLI.

Only Python's standard library is used. Rankings always come from the
IndexableSkipList-powered C++ executable, never from Python sorting.
"""
import argparse
import json
import re
import shutil
import subprocess
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, urlsplit

ROOT = Path(__file__).resolve().parent.parent
INDEX = Path(__file__).with_name("index.html")
PLAYER = re.compile(r"[A-Za-z0-9_-]{1,24}\Z")
ROW = re.compile(r"^(\d+)\.\s+(\S+)\s+(-?\d+)\s*$")
RANK = re.compile(r"^(.+?) is ranked #(\d+) with score (-?\d+)$")
MAX_SAFE_SCORE = 9007199254740991  # keep JSON integer precision in browsers
SAMPLE = [
    ("Nova", 9800), ("Blaze", 9350), ("Cypher", 9150), ("Vortex", 9150),
    ("Shadow", 8870), ("Titan", 8720), ("Pixel", 8600), ("Echo", 8400),
    ("Rogue", 8210), ("Zenith", 8030), ("Ghost", 7910), ("Falcon", 7850),
    ("Nexus", 7600), ("Phoenix", 7520), ("Orion", 7400), ("Pulse", 7350),
    ("Storm", 7000), ("Saber", 6900),
]


def compile_if_needed():
    exe = ROOT / "build" / ("leaderboard.exe" if __import__("os").name == "nt" else "leaderboard")
    sources = list((ROOT / "src").glob("*.cpp"))
    headers = list((ROOT / "include").glob("*.h"))
    if not sources or not headers:
        raise RuntimeError("Run this server from inside the complete leaderboard repository.")
    if not exe.exists() or any(p.stat().st_mtime > exe.stat().st_mtime for p in sources + headers):
        compiler = shutil.which("g++")
        if not compiler:
            raise RuntimeError("g++ was not found. Install a C++17 compiler (or use WSL with g++).")
        exe.parent.mkdir(parents=True, exist_ok=True)
        command = [compiler, "-std=c++17", "-O2", "-Iinclude", *[str(s.relative_to(ROOT)) for s in sources], "-o", str(exe)]
        print("Building original C++ leaderboard...", flush=True)
        subprocess.run(command, cwd=ROOT, check=True)
    return exe


class BadRequest(Exception):
    def __init__(self, message, status=400):
        super().__init__(message)
        self.status = status


def player_id(value):
    if not isinstance(value, str) or not PLAYER.fullmatch(value):
        raise BadRequest("Player ID must be 1–24 characters: letters, numbers, '_' or '-'.")
    return value


def whole_number(value, field, low, high):
    if isinstance(value, bool) or not isinstance(value, int) or not low <= value <= high:
        raise BadRequest(f"{field} must be an integer between {low} and {high}.")
    return value


def get_int(query, key, default, low, high):
    raw = query.get(key, [str(default)])[0]
    if not re.fullmatch(r"[0-9]+", raw):
        raise BadRequest(f"{key} must be a non-negative integer.")
    return whole_number(int(raw), key, low, high)


class CliEngine:
    def __init__(self, executable):
        self.executable = executable
        self.lock = threading.RLock()
        self.proc = None
        self.start()

    def start(self):
        self.proc = subprocess.Popen(
            [str(self.executable)], cwd=ROOT, stdin=subprocess.PIPE,
            stdout=subprocess.PIPE, stderr=None, bufsize=0
        )
        welcome = self._until_prompt()
        if "Leaderboard ready" not in welcome:
            raise RuntimeError(f"Unexpected CLI startup: {welcome!r}")

    def _until_prompt(self):
        response = bytearray()
        while not response.endswith(b"> "):
            char = self.proc.stdout.read(1)
            if not char:
                raise RuntimeError("C++ leaderboard process stopped unexpectedly.")
            response.extend(char)
            if len(response) > 160_000:
                raise RuntimeError("C++ leaderboard response exceeded safety limit.")
        return response[:-2].decode("utf-8", errors="replace").strip()

    def command(self, command):
        # Call with self.lock held from the HTTP handler.
        if not self.proc or self.proc.poll() is not None:
            raise RuntimeError("C++ leaderboard is no longer running.")
        self.proc.stdin.write((command + "\n").encode("utf-8"))
        self.proc.stdin.flush()
        return self._until_prompt()

    def stop(self):
        if self.proc and self.proc.poll() is None:
            self.proc.terminate()
            try:
                self.proc.wait(timeout=2)
            except subprocess.TimeoutExpired:
                self.proc.kill()
                self.proc.wait()
        if self.proc:
            self.proc.stdin.close()
            self.proc.stdout.close()
        self.proc = None

    def restart(self):
        self.stop()
        self.start()


def parse_rows(output):
    rows = []
    for line in output.splitlines():
        match = ROW.fullmatch(line.strip())
        if match:
            rank, name, score = match.groups()
            rows.append({"rank": int(rank), "id": name, "score": int(score)})
    return rows


def find_player(engine, name):
    response = engine.command("rank " + player_id(name))
    match = RANK.fullmatch(response)
    if not match:
        if response.startswith("Unknown player"):
            raise BadRequest("Player not found.", 404)
        raise RuntimeError(f"Unexpected C++ rank output: {response}")
    return {"id": name, "rank": int(match.group(2)), "score": int(match.group(3))}


class Handler(BaseHTTPRequestHandler):
    server_version = "LeaderboardUI/1.0"

    def send_json(self, status, payload):
        data = json.dumps(payload, separators=(",", ":")).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Cache-Control", "no-store")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        url = urlsplit(self.path)
        if url.path in ("/", "/index.html"):
            data = INDEX.read_bytes()
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Cache-Control", "no-store")
            self.send_header("X-Content-Type-Options", "nosniff")
            self.send_header("Content-Length", str(len(data)))
            self.end_headers()
            self.wfile.write(data)
            return
        try:
            query = parse_qs(url.query)
            engine = self.server.engine
            with engine.lock:
                if url.path == "/api/state":
                    limit = get_int(query, "limit", 25, 1, 100)
                    total_text = engine.command("size")
                    size_match = re.fullmatch(r"(\d+) players", total_text)
                    if not size_match:
                        raise RuntimeError(f"Unexpected C++ size output: {total_text}")
                    size = int(size_match.group(1))
                    rows = parse_rows(engine.command(f"top {limit}")) if size else []
                    response = {"size": size, "players": rows, "limit": limit}
                elif url.path == "/api/player":
                    response = find_player(engine, query.get("id", [""])[0])
                elif url.path == "/api/nearby":
                    name = player_id(query.get("id", [""])[0])
                    radius = get_int(query, "radius", 2, 0, 20)
                    target = find_player(engine, name)
                    response = {"target": target, "players": parse_rows(engine.command(f"around {name} {radius}"))}
                elif url.path == "/api/at":
                    rank = get_int(query, "rank", 1, 1, 10_000_000)
                    response = {"players": parse_rows(engine.command(f"at {rank}"))}
                    if not response["players"]:
                        raise BadRequest("No player exists at that rank.", 404)
                else:
                    raise BadRequest("Unknown endpoint.", 404)
            self.send_json(200, response)
        except BadRequest as error:
            self.send_json(error.status, {"error": str(error)})
        except Exception as error:
            print(f"Request failed: {error}", flush=True)
            self.send_json(500, {"error": "The C++ engine could not complete the request."})

    def do_POST(self):
        try:
            size = int(self.headers.get("Content-Length", "0"))
            if size < 1 or size > 8192:
                raise BadRequest("Request body must be a JSON object under 8 KB.")
            try:
                body = json.loads(self.rfile.read(size))
            except (ValueError, UnicodeDecodeError):
                raise BadRequest("Invalid JSON.")
            if not isinstance(body, dict):
                raise BadRequest("Expected a JSON object.")
            engine = self.server.engine
            with engine.lock:
                if self.path == "/api/upsert":
                    name = player_id(body.get("id"))
                    score = whole_number(body.get("score"), "score", -MAX_SAFE_SCORE, MAX_SAFE_SCORE)
                    old = engine.command("rank " + name)
                    previous = RANK.fullmatch(old)
                    message = engine.command(f"update {name} {score}")
                    target = find_player(engine, name)
                    response = {"message": message, "oldRank": int(previous.group(2)) if previous else None, "player": target}
                elif self.path == "/api/remove":
                    name = player_id(body.get("id"))
                    message = engine.command("remove " + name)
                    if message.startswith("Unknown player"):
                        raise BadRequest("Player not found.", 404)
                    response = {"message": message, "id": name}
                elif self.path == "/api/demo":
                    count = whole_number(body.get("count", 18), "count", 1, 18)
                    for name, score in SAMPLE[:count]:
                        engine.command(f"update {name} {score}")
                    response = {"message": f"Loaded {count} demo players using C++ insert/update operations."}
                elif self.path == "/api/reset":
                    engine.restart()
                    response = {"message": "Leaderboard reset. C++ engine restarted with an empty skip list."}
                else:
                    raise BadRequest("Unknown endpoint.", 404)
            self.send_json(200, response)
        except BadRequest as error:
            self.send_json(error.status, {"error": str(error)})
        except Exception as error:
            print(f"Request failed: {error}", flush=True)
            self.send_json(500, {"error": "The C++ engine could not complete the request."})


def main():
    parser = argparse.ArgumentParser(description="Interactive browser UI for the C++ skip list leaderboard")
    parser.add_argument("--port", type=int, default=8765)
    parser.add_argument("--host", default="127.0.0.1", help="Defaults to localhost for safety")
    args = parser.parse_args()
    engine = CliEngine(compile_if_needed())
    httpd = ThreadingHTTPServer((args.host, args.port), Handler)
    httpd.engine = engine
    print(f"Open http://{args.host}:{args.port} in your browser (Ctrl+C to stop).", flush=True)
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        httpd.server_close()
        with engine.lock:
            engine.stop()


if __name__ == "__main__":
    main()
