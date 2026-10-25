# 🏆 Real-Time Leaderboard Ranking Engine using an Indexable Skip List

A real-time leaderboard engine for large-scale online games, built on an **Indexable Skip List** (a skip list augmented with *rank spans*). It supports fast score updates, rank lookups, top-K queries, and "players around me" queries, all in **O(log n)** expected time.

> **DSA Project** · Data Structures & Algorithms
> **Team:** Abhijith, Rajaneesh, Haripranav Rajesh, Santhosh Reddy
> **Institution / Course:** Amrita Vishwa Vidyapeetham, Coimbatore · Data Structures & Algorithms

---

## 📌 Table of Contents

1. [Problem Statement](#-problem-statement)
2. [Why an Indexable Skip List?](#-why-an-indexable-skip-list)
3. [How It Works](#-how-it-works)
4. [Features](#-features)
5. [Time & Space Complexity](#-time--space-complexity)
6. [Project Structure](#-project-structure)
7. [Getting Started](#-getting-started)
8. [Usage Example](#-usage-example)
9. [Testing & Benchmarks](#-testing--benchmarks)
10. [Limitations & Future Work](#-limitations--future-work)
11. [References](#-references)
12. [Contributors](#-contributors)

---

## 🎯 Problem Statement

Online games with millions of players need a leaderboard that:

- **Updates instantly** when a player's score changes (thousands of updates per second).
- **Answers rank queries** like *"What is my rank?"* without scanning every player.
- **Returns the top K players** for the global scoreboard.
- **Shows neighbours**: the players ranked just above and below a given player.

A naive approach (sorted array or list) makes either updates or rank lookups **O(n)**, which does not scale. This project solves the problem using an **Indexable Skip List** that keeps everything at **O(log n)**.

---

## 💡 Why an Indexable Skip List?

| Approach | Insert / Update | Get Rank | Top-K | Notes |
|---|---|---|---|---|
| Sorted array | O(n) | O(log n) | O(K) | Shifting elements is slow |
| Unsorted array | O(1) | O(n log n) | O(n log K) | Must sort/scan for ranks |
| Balanced BST (no augmentation) | O(log n) | O(n) | O(K + log n) | No cheap rank |
| Augmented BST (order-statistic tree) | O(log n) | O(log n) | O(K + log n) | Complex rotations |
| **Indexable Skip List** | **O(log n)** | **O(log n)** | **O(K + log n)** | Simple, no rotations, easy to extend |

A skip list gives us balanced-tree performance using **randomization instead of rotations**. That makes it simpler to implement, easier to reason about, and friendly to concurrent updates. Adding **rank spans** to each link turns it into an *indexable* structure, so we can jump straight to the k-th element or compute any element's rank.

---

## ⚙️ How It Works

### 1. Skip List basics

A skip list is a stack of sorted linked lists. The bottom level (Level 0) holds **every** player; each higher level holds a random subset that acts as an "express lane".

```
Level 3:  HEAD ─────────────────────────────▶ 90 ─────────────────▶ NIL
Level 2:  HEAD ─────────────▶ 55 ───────────▶ 90 ─────────────────▶ NIL
Level 1:  HEAD ───▶ 30 ─────▶ 55 ───▶ 72 ───▶ 90 ────▶ 95 ────────▶ NIL
Level 0:  HEAD ─▶ 10 ─▶ 30 ─▶ 55 ─▶ 72 ─▶ 90 ─▶ 95 ─▶ 99 ───────▶ NIL
```

Each node is promoted to the next level with probability `p` (typically `0.25` or `0.5`), giving an expected height of **O(log n)**.

### 2. Rank spans (the "indexable" part)

Every forward pointer stores a **span**: the number of Level-0 nodes it skips over.

```
node.level[i].forward  →  next node at level i
node.level[i].span     →  how many bottom-level steps that jump covers
```

- **Get rank:** while searching for a player, add up the spans of every link you follow. The total is the player's rank.
- **Get by rank:** walk down the levels, following a link only if `traversed + span ≤ target rank`.
- **Insert/Delete:** update spans of the links that cross the changed position.

### 3. Ordering rule

Players are ordered by **score (descending)**, with ties broken by **player ID (ascending)** so that ordering is always deterministic.

### 4. Updating a score

A score update is performed as **delete old entry → insert new entry**. A `HashMap<playerId, score>` is kept alongside the skip list so we can locate a player's current score in O(1) before the delete.

---

## ✨ Features

- ➕ **Add / update player score** in O(log n)
- ❌ **Remove player** from the leaderboard
- 🔢 **Get rank** of any player (1-indexed)
- 🎯 **Get player by rank**
- 🥇 **Top-K query** for the global leaderboard
- 👥 **Neighbour query**: players ranked around a given player (e.g., ±5)
- 📊 **Range query** by rank or by score
- 🧪 **Stress tests and benchmarks** against a naive sorted-array baseline

---

## ⏱️ Time & Space Complexity

| Operation | Expected Time | Worst Case |
|---|---|---|
| `updateScore(player, score)` | O(log n) | O(n) *(very unlikely)* |
| `removePlayer(player)` | O(log n) | O(n) |
| `getRank(player)` | O(log n) | O(n) |
| `getByRank(k)` | O(log n) | O(n) |
| `topK(k)` | O(log n + k) | O(n) |
| `neighbours(player, r)` | O(log n + r) | O(n) |

**Space:** O(n) expected. Each node has on average `1 / (1 − p)` forward pointers.

---

## 📁 Project Structure

```
.
├── include/
│   ├── types.h                 # Entry, ordering rule (before), constants
│   ├── IndexableSkipList.h     # Skip list class + rank span invariants
│   └── Leaderboard.h           # Game-facing API
├── src/
│   ├── skiplist_core.cpp       # Node/levels, random level, insert, remove
│   ├── skiplist_query.cpp      # getRank, getByRank, topK, neighbours, ranges
│   ├── skiplist_validate.cpp   # Debug checker for ordering and span invariants
│   ├── Leaderboard.cpp         # Hash map + skip list wrapper
│   └── main.cpp                # CLI demo
├── tests/
│   ├── test_core.cpp           # insert/remove + span validation
│   ├── test_query.cpp          # every query vs. a sorted vector
│   ├── test_leaderboard.cpp    # wrapper behaviour, ties, removals
│   ├── naive_leaderboard.h     # Sorted-vector baseline
│   └── stress_test.cpp         # 100,000 random ops vs. the baseline
├── benchmarks/
│   └── benchmark.cpp           # Skip list vs. sorted array
├── Makefile
├── OWNERS.md
└── README.md
```

---

## 🚀 Getting Started

### Prerequisites

- A C++17 compiler (`g++` 9 or newer, or `clang++`)
- `make` (on Windows use MSYS2/MinGW or WSL)

### Clone the repository

```bash
git clone https://github.com/abhijith-2008/Real-Time-Leaderboard-Ranking-Engine.git
cd Real-Time-Leaderboard-Ranking-Engine
```

### Build & run

```bash
make                                   # builds build/leaderboard
./build/leaderboard                    # interactive CLI (type "help")
./build/leaderboard --no-prompt < commands.txt   # scripted run

make test                              # all tests
make bench                             # benchmark table
```

---

## 🕹️ Usage Example

```text
> add Alice 1500
Alice added at #1 with score 1500
> add Bob 2300
Bob added at #1 with score 2300
> add Charlie 1800
Charlie added at #2 with score 1800
> add Dave 2300
Dave added at #2 with score 2300

> top 3
1. Bob         2300
2. Dave        2300
3. Charlie     1800

> rank Alice
Alice is ranked #4 with score 1500

> update Alice 2000
Alice moved from #4 to #3

> around Alice 1
2. Dave        2300
3. Alice       2000
4. Charlie     1800
```

Other commands: `remove`, `score`, `at <rank>`, `around <name> [radius]`, `size`, `help`, `quit`.

---

## 🧪 Testing & Benchmarks

Run everything with:

```bash
make test    # all correctness tests and the stress test
make bench   # benchmark table
```

**Correctness:**

- `test_core` inserts and removes thousands of entries (including heavy score ties) and runs the span validator after every operation.
- `test_query` compares `getRank`, `getByRank`, `topK`, `neighbours`, `rangeByRank` and `rangeByScore` against a plain sorted vector, including edge cases (empty list, out-of-range ranks, unknown players).
- `test_leaderboard` checks the game-facing API: updates, ties, removals and unknown players.
- `stress_test` runs 100,000 random operations on the skip list leaderboard and on a brute-force sorted-vector baseline, comparing every result, and validates the structure's spans throughout.

**Benchmarks:** measured with `make bench` on WSL2 (Ubuntu), `g++ -std=c++17 -O2`. Times are average microseconds per operation (lower is better). Top-K uses K = 100.

| Players (n) | Skip List update | Sorted Array update | Skip List rank | Sorted Array rank | Skip List top-K | Sorted Array top-K |
|---|---|---|---|---|---|---|
| 10,000 | 1.30 | 43.82 | 0.56 | 0.25 | 1.29 | 0.64 |
| 100,000 | 3.57 | 428.97 | 1.62 | 0.49 | 0.99 | 0.64 |
| 1,000,000 | 9.33 | 4480.93 | 3.92 | 1.31 | 1.08 | 0.63 |

**Reading the results:** score updates are where the skip list wins decisively, about 34x faster at 10,000 players and about 480x faster at 1,000,000, because the sorted array must shift O(n) elements on every update. The sorted array stays faster for rank lookups and top-K, since binary search and copying a contiguous block are very cache friendly, while a skip list follows pointers. Both rank lookups are O(log n), so that gap is a constant factor. The skip list is the only structure here that keeps updates, rank queries and top-K all fast as the player count grows. Sorted array update averages use 20,000, 2,000 and 200 operations for the three sizes, because each update is O(n).

---

## 🔮 Limitations & Future Work

- Currently **single-threaded**; a lock-based or lock-free concurrent version would suit real game servers.
- **In-memory only**; persistence (snapshots / write-ahead log) is not included.
- Add **sharded leaderboards** (per region / per season) with a merge step for the global view.
- Support **time-windowed leaderboards** (daily / weekly resets).
- Tune the promotion probability `p` and max level based on expected player count.

---

## 📚 References

1. William Pugh, *"Skip Lists: A Probabilistic Alternative to Balanced Trees"*, Communications of the ACM, 1990.
2. Redis Sorted Sets (`ZSET`): production use of a skip list with spans for rank queries.
3. Cormen et al., *Introduction to Algorithms*: order-statistic trees (for comparison).

---

## 👥 Contributors

| Name | Roll No. | Role | Responsibilities |
|---|---|---|---|
| `Abhijith` | `CB.SC.U4CSE25316` | Core Data Structure | Node and level design, random level generation, rank-span maintenance, insert and delete |
| `Rajaneesh` | `CB.SC.U4CSE25362` | Rank & Query Operations | `getRank`, `getByRank`, `topK`, neighbour queries, range queries |
| `Haripranav Rajesh` | `CB.SC.U4CSE25322` | Leaderboard API & Demo | Leaderboard wrapper with hash map, score update logic, tie-breaking, CLI / demo driver |
| `Santhosh Reddy` | `CB.SC.U4CSE25327` | Testing, Benchmarks & Docs | Unit and stress tests, benchmark vs. sorted array, README and project report |

---


