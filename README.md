# Real-Time Leaderboard Ranking Engine Using an Indexable Skip List

## About the Project

This project implements a real-time leaderboard system using an **Indexable Skip List** in C++.

The idea is to make leaderboard operations efficient even when the number of players becomes large. Instead of sorting all the players every time someone's score changes, the system maintains players in ranked order and uses rank spans to navigate the list efficiently.

The leaderboard supports score updates, player removal, rank lookups, top-K queries, and queries to find players around a particular player.

Most of these operations take **O(log n) expected time**, making the skip list a suitable choice for applications where scores change frequently.

**Project:** Data Structures and Algorithms
**Team:** Abhijith, Rajaneesh, Haripranav Rajesh, Santhosh Reddy
**Institution:** Amrita Vishwa Vidyapeetham, Coimbatore

---

## Table of Contents

1. [Problem Statement](#problem-statement)
2. [Why an Indexable Skip List?](#why-an-indexable-skip-list)
3. [How It Works](#how-it-works)
4. [Features](#features)
5. [Time and Space Complexity](#time-and-space-complexity)
6. [Project Structure](#project-structure)
7. [Getting Started](#getting-started)
8. [Usage Example](#usage-example)
9. [Testing and Benchmarks](#testing-and-benchmarks)
10. [Limitations and Future Work](#limitations-and-future-work)
11. [References](#references)
12. [Contributors](#contributors)

---

## Problem Statement

In online games, a leaderboard needs to keep track of player scores and display their rankings. When there are thousands or even millions of players, updating scores and calculating ranks efficiently becomes important.

The system should be able to:

* Update a player's score without having to sort the entire leaderboard again.
* Find a player's rank without checking every player individually.
* Display the top K players.
* Show the players ranked just above and below a particular player.

A straightforward sorted array or linked list can become inefficient when updates are frequent or rank queries are needed.

For example, inserting a new entry into a sorted array may require shifting many elements. Similarly, finding a player's position in an ordinary linked list requires traversing the list.

To address these issues, we use an **Indexable Skip List**, which supports efficient updates and rank-related queries through multiple levels of forward pointers and rank spans.

---

## Why an Indexable Skip List?

We considered different data structures for maintaining the leaderboard.

| Data Structure                       | Insert / Update | Get Rank     | Top-K            | Notes                                                         |
| ------------------------------------ | --------------- | ------------ | ---------------- | ------------------------------------------------------------- |
| Sorted array                         | O(n)            | O(log n)     | O(K)             | Insertions and updates may require shifting elements          |
| Unsorted array                       | O(1)            | O(n log n)   | O(n log K)       | Finding ranks requires additional processing                  |
| Balanced BST without augmentation    | O(log n)        | O(n)         | O(K + log n)     | Does not directly support efficient rank queries              |
| Augmented BST (order-statistic tree) | O(log n)        | O(log n)     | O(K + log n)     | Supports rank queries but requires additional balancing logic |
| **Indexable Skip List**              | **O(log n)**    | **O(log n)** | **O(K + log n)** | Uses randomized levels and does not require rotations         |

A skip list uses multiple levels of linked lists to make searching faster. Unlike a balanced binary search tree, it relies on randomization rather than rotations to maintain its structure.

We extended the skip list by storing a **span** with each forward pointer. These spans help the system calculate player ranks and move directly to a particular position.

This makes the structure useful for a leaderboard where both score updates and ranking queries need to be handled efficiently.

---

## How It Works

### 1. Understanding the Skip List

A skip list consists of multiple levels of linked lists.

The bottom level, Level 0, contains every player. Higher levels contain selected nodes and allow the algorithm to skip over several entries while searching.

For example:

```text
Level 3:  HEAD ─────────────────────────────▶ 90 ─────────────────▶ NIL
Level 2:  HEAD ─────────────▶ 55 ───────────▶ 90 ─────────────────▶ NIL
Level 1:  HEAD ───▶ 30 ─────▶ 55 ───▶ 72 ───▶ 90 ────▶ 95 ────────▶ NIL
Level 0:  HEAD ─▶ 10 ─▶ 30 ─▶ 55 ─▶ 72 ─▶ 90 ─▶ 95 ─▶ 99 ───────▶ NIL
```

Each node has a randomly generated height. A node is promoted to the next level with probability `p`, which is typically `0.25` or `0.5`.

This arrangement gives the skip list an expected height of O(log n), allowing searches to skip over many nodes instead of visiting them one by one.

### 2. Rank Spans: What Makes the Skip List Indexable?

A regular skip list makes searching efficient, but it does not automatically tell us the rank of a node.

To support rank-based operations, we store a span alongside each forward pointer.

```text
node.level[i].forward  →  Next node at level i
node.level[i].span     →  Number of bottom-level steps covered by the pointer
```

The spans allow the system to keep track of how far it has moved through the leaderboard.

They are used in the following operations:

* **Finding a rank:** The algorithm adds the spans of the forward pointers it follows. The total gives the player's rank.
* **Finding a player by rank:** The algorithm moves through the levels while ensuring that the accumulated span does not exceed the requested rank.
* **Insertion and deletion:** The spans of affected pointers are updated whenever a player is added or removed.

Maintaining these spans correctly is important because the query operations depend on them.

### 3. How Players Are Ranked

Players are sorted using two rules:

1. Players with higher scores appear first.
2. If two players have the same score, their player IDs are compared in ascending order.

Using the player ID as a tie-breaker ensures that the ordering remains deterministic when scores are equal.

### 4. Updating a Player's Score

When a player's score changes, the system removes the old entry from the skip list and inserts a new entry with the updated score.

A hash map stores each player's current score alongside the skip list.

This allows the system to look up a player's existing score in expected O(1) time before updating their position in the leaderboard.

---

## Features

The leaderboard currently supports the following operations:

* **Add or update a player:** Insert a new player or update an existing player's score.
* **Remove a player:** Delete a player from the leaderboard.
* **Get rank:** Find a player's current rank, using 1-based indexing.
* **Get player by rank:** Retrieve the player occupying a particular position.
* **Top-K query:** Display the highest-ranked K players.
* **Neighbour query:** Find players ranked around a particular player, such as the three players above and below them.
* **Range queries:** Retrieve players within a specified rank range or score range.
* **Structural validation:** Check whether the skip list's ordering and span information are consistent.
* **Stress testing:** Compare the implementation against a simpler sorted-vector baseline.
* **Benchmarking:** Measure the performance of the skip list and compare it with a sorted-array approach.

---

## Time and Space Complexity

Let `n` represent the number of players in the leaderboard.

The expected time complexities of the main operations are:

| Operation                    | Expected Time | Worst Case |
| ---------------------------- | ------------- | ---------- |
| `updateScore(player, score)` | O(log n)      | O(n)       |
| `removePlayer(player)`       | O(log n)      | O(n)       |
| `getRank(player)`            | O(log n)      | O(n)       |
| `getByRank(k)`               | O(log n)      | O(n)       |
| `topK(k)`                    | O(log n + k)  | O(n)       |
| `neighbours(player, r)`      | O(log n + r)  | O(n)       |

The skip list provides these expected bounds because its levels are generated randomly. The worst-case time can still reach O(n).

**Space complexity:** O(n) expected.

Each node contains an average of `1 / (1 - p)` forward pointers, where `p` is the promotion probability. The overall space requirement grows linearly with the number of players.

---

## Project Structure

The project is organized into separate files for the skip list implementation, leaderboard interface, tests, and benchmarks.

```text
.
├── include/
│   ├── types.h                 # Entry, ordering rule, and constants
│   ├── IndexableSkipList.h     # Skip list class and rank-span invariants
│   └── Leaderboard.h           # Leaderboard API
├── src/
│   ├── skiplist_core.cpp       # Node levels, random levels, insert, remove
│   ├── skiplist_query.cpp      # Rank, top-K, neighbours, and range queries
│   ├── skiplist_validate.cpp   # Ordering and span validation
│   ├── Leaderboard.cpp         # Hash map and skip list integration
│   └── main.cpp                # Command-line interface
├── tests/
│   ├── test_core.cpp           # Insertion, removal, and span checks
│   ├── test_query.cpp          # Query results compared with a sorted vector
│   ├── test_leaderboard.cpp    # Leaderboard API, ties, and removals
│   ├── naive_leaderboard.h     # Sorted-vector reference implementation
│   └── stress_test.cpp         # 100,000 random operations against the baseline
├── benchmarks/
│   └── benchmark.cpp           # Skip list vs. sorted-array benchmark
├── Makefile
├── OWNERS.md
└── README.md
```

---

## Getting Started

### Prerequisites

Before running the project, make sure you have:

* A C++17 compiler, such as `g++` version 9 or newer, or `clang++`.
* `make`.
* On Windows, MSYS2/MinGW or WSL can be used to provide the required build tools.

### Clone the Repository

Open a terminal and run:

```bash
git clone https://github.com/abhijith-2008/Real-Time-Leaderboard-Ranking-Engine.git
cd Real-Time-Leaderboard-Ranking-Engine
```

### Build and Run

Build the project using:

```bash
make
```

This builds the leaderboard executable at `build/leaderboard`.

To start the interactive command-line interface, run:

```bash
./build/leaderboard
```

You can type `help` inside the program to see the available commands.

To run the program using commands stored in a file, use:

```bash
./build/leaderboard --no-prompt < commands.txt
```

To run the tests:

```bash
make test
```

To run the benchmarks:

```bash
make bench
```

---

## Usage Example

The following example shows how the leaderboard behaves when players are added, queried, and updated.

### Adding Players

```text
> add Alice 1500
Alice added at #1 with score 1500

> add Bob 2300
Bob added at #1 with score 2300

> add Charlie 1800
Charlie added at #2 with score 1800

> add Dave 2300
Dave added at #2 with score 2300
```

Bob and Dave have the same score, so their order is determined by their player IDs.

### Displaying the Top Players

```text
> top 3
1. Bob         2300
2. Dave        2300
3. Charlie     1800
```

The `top` command returns the highest-ranked players without requiring the entire leaderboard to be sorted again.

### Finding a Player's Rank

```text
> rank Alice
Alice is ranked #4 with score 1500
```

### Updating a Score

```text
> update Alice 2000
Alice moved from #4 to #3
```

Alice's new score changes her position in the leaderboard.

### Finding Nearby Players

```text
> around Alice 1
2. Dave        2300
3. Alice       2000
4. Charlie     1800
```

The `around` command displays the players immediately surrounding Alice in the ranking.

Other available commands include:

* `remove` — Remove a player.
* `score` — Find a player's score.
* `at <rank>` — Retrieve the player at a particular rank.
* `around <name> [radius]` — Display players around a given player.
* `size` — Display the number of players.
* `help` — Display available commands.
* `quit` — Exit the program.

---

## Testing and Benchmarks

We use both correctness tests and performance benchmarks to evaluate the implementation.

### Running the Tests

Run the following commands from the project directory:

```bash
make test
```

This runs the correctness tests and the stress test.

To run the benchmarks separately:

```bash
make bench
```

### Correctness Tests

The project includes several tests to check the behaviour of the skip list and the leaderboard.

**`test_core`**

Tests insertion and removal of thousands of entries, including cases where many players have the same score. The span validator is run after every operation to check the structure.

**`test_query`**

Checks the results of `getRank`, `getByRank`, `topK`, `neighbours`, `rangeByRank`, and `rangeByScore` against a plain sorted vector.

It also covers edge cases such as an empty list, invalid ranks, and unknown players.

**`test_leaderboard`**

Checks the leaderboard interface, including score updates, tie-breaking, player removal, and requests involving unknown players.

**`stress_test`**

Runs 100,000 randomized operations against both the skip list and a sorted-vector reference implementation.

The results are compared to check whether both implementations produce the same answers. The skip list's structural invariants are also checked throughout the test.

Using a simpler reference implementation gives us a way to verify the results independently of the skip list's internal logic.

### Benchmark Setup

The benchmarks compare the skip list against a sorted-array implementation for different leaderboard sizes.

The tests were measured using:

* **Environment:** WSL2 (Ubuntu)
* **Compiler:** `g++`
* **Language standard:** C++17
* **Optimization:** `-O2`
* **Reported measurement:** Average microseconds per operation
* **Top-K size:** K = 100

Lower execution times indicate better performance.

### Benchmark Results

| Players (n) | Skip List Update (µs) | Sorted Array Update (µs) | Skip List Rank (µs) | Sorted Array Rank (µs) | Skip List Top-K (µs) | Sorted Array Top-K (µs) |
| ----------- | --------------------: | -----------------------: | ------------------: | ---------------------: | -------------------: | ----------------------: |
| 10,000      |                  1.30 |                    43.82 |                0.56 |                   0.25 |                 1.29 |                    0.64 |
| 100,000     |                  3.57 |                   428.97 |                1.62 |                   0.49 |                 0.99 |                    0.64 |
| 1,000,000   |                  9.33 |                  4480.93 |                3.92 |                   1.31 |                 1.08 |                    0.63 |

### What the Results Show

The main difference appears in score updates.

At 10,000 players, the skip list update takes an average of 1.30 µs, compared with 43.82 µs for the sorted array.

At 1,000,000 players, the times are 9.33 µs and 4480.93 µs, respectively.

This happens because updating an entry in a sorted array can require shifting O(n) elements. The skip list, on the other hand, has an expected update complexity of O(log n).

The benchmark also shows that the sorted-array implementation is faster for rank lookups and top-K queries in these measurements.

This is partly because arrays store elements contiguously in memory, making binary search and copying a block of elements relatively efficient. Skip lists rely on pointer traversal, which can introduce additional overhead.

Both implementations have O(log n) rank-query complexity in the comparison used here, but their actual execution times differ.

The sorted-array update averages use 20,000, 2,000, and 200 operations for the three respective leaderboard sizes because each update can require O(n) work.

Overall, the results highlight the main trade-off: **the skip list handles score updates much more efficiently as the leaderboard grows, while the sorted-array approach performs better for some read operations in these benchmarks.**

---

## Limitations and Future Work

The current implementation covers the main leaderboard operations, but there are several areas that could be developed further.

* **Concurrency:** The implementation is currently single-threaded. A lock-based or lock-free version could make it more suitable for game servers handling simultaneous updates.
* **Persistence:** The leaderboard stores its data in memory. Persistent storage, snapshots, or a write-ahead log could help preserve the leaderboard across restarts.
* **Sharded leaderboards:** Separate leaderboards could be maintained for different regions or seasons, with a merge step used to produce a global ranking.
* **Time-based leaderboards:** Daily or weekly leaderboards could be added with automatic resets.
* **Parameter tuning:** The promotion probability `p` and maximum level could be adjusted based on the expected number of players.

These improvements would extend the current implementation beyond a single-process, in-memory leaderboard.

---

## References

1. William Pugh, *Skip Lists: A Probabilistic Alternative to Balanced Trees*, Communications of the ACM, 1990.
2. Redis Sorted Sets (`ZSET`), which use skip lists with spans to support rank-related operations.
3. Thomas H. Cormen et al., *Introduction to Algorithms*, for a comparison with order-statistic trees.

---

## Contributors

This project was divided into four main areas: the core data structure, query operations, application integration, and testing.

| Name              | Roll Number        | Role                                   | Responsibilities                                                                                    |
| ----------------- | ------------------ | -------------------------------------- | --------------------------------------------------------------------------------------------------- |
| Ch. Abhijith      | `CB.SC.U4CSE25316` | Core Data Structure                    | Node and level design, random-level generation, rank-span maintenance, insertion, and deletion      |
| V. Rajaneesh      | `CB.SC.U4CSE25362` | Rank and Query Operations              | `getRank`, `getByRank`, `topK`, neighbour queries, and range queries                                |
| Haripranav Rajesh | `CB.SC.U4CSE25322` | Leaderboard API and Demo               | Leaderboard wrapper, hash map integration, score-update logic, tie-breaking, and CLI                |
| Santhosh Reddy    | `CB.SC.U4CSE25327` | Testing, Benchmarks, and Documentation | Unit tests, stress testing, benchmark comparison against a sorted array, README, and project report |

---

*Developed as part of the Data Structures and Algorithms course at Amrita Vishwa Vidyapeetham, Coimbatore.*
