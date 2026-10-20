# File ownership

Only the owner edits a file. Open a pull request for every change. Nobody pushes to main directly.

| Owner | Files |
|---|---|
| Abhijith | src/skiplist_core.cpp, tests/test_core.cpp |
| Rajaneesh | src/skiplist_query.cpp, tests/test_query.cpp |
| Haripranav | src/Leaderboard.cpp, src/main.cpp, tests/test_leaderboard.cpp |
| Santhosh | src/skiplist_validate.cpp, tests/naive_leaderboard.h, tests/stress_test.cpp, benchmarks/benchmark.cpp, README.md, Makefile, .gitignore |
| Frozen (change only by team agreement) | include/types.h, include/IndexableSkipList.h, include/Leaderboard.h |

The Makefile uses wildcards, so adding a new .cpp file under src/, tests/ or benchmarks/ needs no Makefile edit.
