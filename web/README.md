# Interactive leaderboard dashboard

This dashboard talks to the **existing C++17 `Leaderboard` and `IndexableSkipList`** through the project's CLI. No sorting, score storage, or ranking is performed by the browser or Python. The Python server simply translates button actions into C++ CLI commands, then parses the results into JSON. No third-party Python/JS packages are necessary.

## Run locally

From the repository root (in **WSL/Ubuntu** or any shell with Python 3 and `g++`):

```bash
python3 web/server.py
```

On native Windows with `python` and MinGW `g++` available in PATH:

```powershell
python web/server.py
```

The server recompiles the original C++ application when source/header files change. Visit **http://127.0.0.1:8765** in a browser. Use `Ctrl+C` to stop the server.

## Demo sequence

1. Choose **Load demo players** to populate the real C++ skip list.
2. Search for **Vortex** to display their rank and adjacent players.
3. Update `Vortex` to `9900` and watch the leaderboard reorder.
4. Add `Alpha` with a score of `9150` and demonstrate deterministic tie-breaking (score descending, ID ascending).
5. Delete `Vortex`; show ranks adjust automatically.
6. Use the **Top 10 / 25 / 50 / 100** selector and explain expected O(log n) rank updates.
7. Reset to start a clean demonstration.

## API routes

`GET /api/state?limit=25`, `GET /api/player?id=Nova`, `GET /api/nearby?id=Nova&radius=2`, `GET /api/at?rank=1`, `POST /api/upsert` with `{ "id": "Nova", "score": 9000 }`, `POST /api/remove` with `{ "id": "Nova" }`, `POST /api/demo`, `POST /api/reset`.

The UI accepts single-word player IDs containing letters, numbers, `_` and `-` (consistent with the existing CLI). Scores must be safe JSON integers. Player data is **in-memory only**; it resets when the server is restarted. This is a single-process, localhost-only college demo, not a production deployment. Python serializes access to the one C++ process with a lock.

All files are isolated in `web/`; **existing C++ code, headers, tests and Makefile are not modified**. The leaderboard CLI and `make test` remain usable as before.
