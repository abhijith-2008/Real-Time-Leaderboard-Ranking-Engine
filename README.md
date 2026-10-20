# 🏆 Real-Time Leaderboard Ranking Engine using an Indexable Skip List

A real-time leaderboard engine for large-scale online games, built on an **Indexable Skip List** (a skip list augmented with *rank spans*). It supports fast score updates, rank lookups, top-K queries, and "players around me" queries, all in **O(log n)** expected time.

> **DSA Project** · Data Structures & Algorithms
> **Team:** `<Member 1>`, `<Member 2>`, `<Member 3>`
> **Institution / Course:** `<College Name>` · `<Course Code>`

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

> Adjust this to match your actual repo layout.

```
.
├── src/
│   ├── IndexableSkipList.*     # Core data structure (nodes, levels, spans)
│   ├── Leaderboard.*           # Game-facing API built on top of the skip list
│   └── main.*                  # Demo / CLI driver
├── tests/
│   ├── correctness_tests.*     # Unit tests for insert/delete/rank/top-K
│   └── stress_test.*           # Randomized tests vs. brute-force baseline
├── benchmarks/
│   └── benchmark.*             # Skip list vs. sorted array performance
├── docs/
│   └── report.pdf              # Project report / presentation (optional)
└── README.md
```

---

## 🚀 Getting Started

### Prerequisites

- `<Language / compiler / runtime, e.g. g++ 11+, Java 17, Python 3.10+>`
- `<Build tool if any, e.g. make, cmake, maven>`

### Clone the repository

```bash
git clone https://github.com/abhijith-2008/Real-Time-Leaderboard-Ranking-Engine.git
cd Real-Time -Leaderboard-Ranking-Engine
```

### Build & run

```bash
# Replace with the commands for your language, for example:

# C++
g++ -std=c++17 -O2 src/*.cpp -o leaderboard
./leaderboard

# Java
javac src/*.java -d out && java -cp out Main

# Python
python src/main.py
```

---

## 🕹️ Usage Example

```text
> add Alice 1500
> add Bob 2300
> add Charlie 1800
> add Dave 2300

> top 3
1. Bob      2300
2. Dave     2300
3. Charlie  1800

> rank Alice
Alice is ranked #4 with score 1500

> update Alice 2000
Alice moved from #4 to #3

> around Charlie 1
2. Dave     2300
3. Alice    2000
4. Charlie  1800
```

*(Replace with real output from your implementation.)*

---

## 🧪 Testing & Benchmarks

**Correctness:** every operation is cross-checked against a brute-force sorted-list implementation over thousands of random operations, verifying ranks, top-K output, and span consistency after each insert/delete.

**Benchmarks:** compare the skip list against a sorted array as player count grows.

| Players (n) | Skip List (update) | Sorted Array (update) |
|---|---|---|
| 10,000 | `<fill in>` | `<fill in>` |
| 100,000 | `<fill in>` | `<fill in>` |
| 1,000,000 | `<fill in>` | `<fill in>` |

*(Fill in with your measured results.)*

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


