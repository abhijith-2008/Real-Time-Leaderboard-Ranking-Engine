"""Run integration tests against the real C++ leaderboard + web/server.py.

Copy this file to the repository root, then run:
    python verify_web_ui.py
No third-party Python dependencies.
"""
import json
import socket
import subprocess
import sys
import time
import urllib.error
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent
if not (ROOT / "web" / "server.py").exists():
    raise SystemExit("Copy verify_web_ui.py to the repository root (next to web/).")

with socket.socket() as sock:
    sock.bind(("127.0.0.1", 0))
    PORT = sock.getsockname()[1]
BASE = f"http://127.0.0.1:{PORT}"


def request(path, body=None, expected=200):
    data = json.dumps(body).encode("utf-8") if body is not None else None
    req = urllib.request.Request(BASE + path, data=data,
                                headers={"Content-Type": "application/json"},
                                method="POST" if body is not None else "GET")
    try:
        with urllib.request.urlopen(req, timeout=15) as resp:
            code, content = resp.status, resp.read().decode("utf-8")
    except urllib.error.HTTPError as exc:
        code, content = exc.code, exc.read().decode("utf-8")
    assert code == expected, f"{path}: Expected HTTP {expected}, got {code} ({content})"
    return content if path == "/" else json.loads(content)


checks = []
def check(title, action):
    try:
        action()
        checks.append((title, "PASS"))
    except Exception as exc:
        checks.append((title, f"FAIL: {exc!r}"))


def must(condition, message):
    assert condition, message


process = subprocess.Popen([sys.executable, "web/server.py", "--port", str(PORT)],
                           cwd=ROOT, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
try:
    for attempt in range(200):
        if process.poll() is not None:
            raise RuntimeError(f"Server exited with code {process.returncode}")
        try:
            request("/")
            break
        except (OSError, AssertionError):
            time.sleep(0.15)
    else:
        raise RuntimeError("Server failed to start")

    check("Dashboard HTML loads", lambda: must("RankForge" in request("/"), "Missing dashboard"))
    check("Starts empty", lambda: must(request("/api/state")["size"] == 0, "Not empty"))
    check("Unknown player 404", lambda: must("error" in request("/api/player?id=Nobody", expected=404), "Missing 404"))
    check("Load 18 sample players", lambda: (request("/api/demo", {}), must(request("/api/state")["size"] == 18, "Wrong size")))
    check("Correct Top-4 sorting", lambda: must([x["id"] for x in request("/api/state")["players"][:4]] == ["Nova", "Blaze", "Cypher", "Vortex"], "Incorrect order"))
    check("Player rank lookup", lambda: must(request("/api/player?id=Vortex")["rank"] == 4, "Wrong rank"))
    check("Neighbour range", lambda: must([p["rank"] for p in request("/api/nearby?id=Vortex&radius=1")["players"]] == [3,4,5], "Wrong neighbours"))
    check("Update causes rank movement", lambda: must(request("/api/upsert", {"id":"Vortex","score":9900})["player"]["rank"] == 1, "Not rank #1"))
    def check_ties():
        request("/api/upsert", {"id":"Alpha","score":9150})
        must([p["id"] for p in request("/api/state")["players"] if p["score"] == 9150] == ["Alpha", "Cypher"], "Tie-break incorrect")
    check("Tie-breaking by player ID", check_ties)
    check("Find player at rank", lambda: must(request("/api/at?rank=1")["players"][0]["id"] == "Vortex", "Wrong player"))
    def check_remove():
        request("/api/remove", {"id":"Vortex"})
        must(request("/api/state")["players"][0]["id"] == "Nova", "Failed to rerank")
    check("Remove and rerank", check_remove)
    check("Unknown remove 404", lambda: must("error" in request("/api/remove", {"id":"Missing"}, expected=404), "Missing error"))
    check("Negative score", lambda: must(request("/api/upsert", {"id":"Neg","score":-100})["player"]["score"] == -100, "Wrong score"))
    check("Bad player ID rejected", lambda: must("error" in request("/api/upsert", {"id":"bad name","score":123}, expected=400), "Bad ID accepted"))
    check("Decimal score rejected", lambda: must("error" in request("/api/upsert", {"id":"Valid","score":1.5}, expected=400), "Decimal accepted"))
    check("Top limit capped", lambda: must("error" in request("/api/state?limit=101", expected=400), "Bad limit accepted"))
    check("Nonexistent rank 404", lambda: must("error" in request("/api/at?rank=1000", expected=404), "Missing error"))
    def check_reset():
        request("/api/reset", {})
        must(request("/api/state")["size"] == 0, "Reset didn't clear")
    check("Reset clears all players", check_reset)
    def check_long():
        request("/api/upsert", {"id":"VeryLongPlayerName123","score":1500})
        must(any(p["id"] == "VeryLongPlayerName123" for p in request("/api/state")["players"]), "Long ID missing from Top-K")
    check("Long player ID in leaderboard", check_long)
    check("Long player ID by rank", lambda: must(request("/api/at?rank=1")["players"][0]["id"] == "VeryLongPlayerName123", "Long ID missing by rank"))

    for title, outcome in checks:
        print(f"{'[PASS]' if outcome == 'PASS' else '[FAIL]'} {title}" + ("" if outcome == "PASS" else f" — {outcome}"))
    passed = sum(outcome == "PASS" for _, outcome in checks)
    print(f"\nResult: {passed}/{len(checks)} tests passed")
    sys.exit(0 if passed == len(checks) else 1)
finally:
    process.terminate()
    try:
        process.wait(timeout=3)
    except subprocess.TimeoutExpired:
        process.kill()
