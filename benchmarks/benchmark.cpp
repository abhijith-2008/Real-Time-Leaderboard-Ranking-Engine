// OWNER: Santhosh. Benchmark: Indexable Skip List leaderboard versus a sorted vector.
// Usage: ./build/benchmark
// Prints a markdown table that can be pasted into the README.
#include <chrono>
#include <cstdio>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "Leaderboard.h"
#include "naive_leaderboard.h"

using namespace lb;
using Clock = std::chrono::steady_clock;

static std::string id(int i) {
    std::string s = std::to_string(i);
    return "p" + std::string(8 - s.size(), '0') + s;
}

static double microsSince(Clock::time_point t0) {
    return std::chrono::duration<double, std::micro>(Clock::now() - t0).count();
}

static volatile size_t sink;  // stops the compiler from deleting the measured work

struct Row {
    int n;
    double slUpdate, nvUpdate, slRank, nvRank, slTop, nvTop, slBuild;
    int nvUpdateOps;
};

int main() {
    const std::vector<int> sizes = {10000, 100000, 1000000};
    const int SL_UPDATES = 100000;
    const int RANK_OPS = 100000;
    const int TOPK_OPS = 1000;
    const size_t K = 100;

    std::vector<Row> rows;
    for (int n : sizes) {
        std::mt19937 rng(2024);
        std::vector<Entry> initial;
        initial.reserve(n);
        for (int i = 0; i < n; ++i) initial.push_back(Entry{id(i), static_cast<Score>(rng() % 1000000000)});

        // Fixed query workloads so both structures do identical work.
        std::vector<int> rankIds(RANK_OPS), updIds(SL_UPDATES);
        std::vector<Score> updScores(SL_UPDATES);
        for (int& x : rankIds) x = static_cast<int>(rng() % n);
        for (int& x : updIds) x = static_cast<int>(rng() % n);
        for (Score& x : updScores) x = static_cast<Score>(rng() % 1000000000);

        Row r{};
        r.n = n;
        std::cout << "n = " << n << " ..." << std::endl;

        // ---- Skip list ----
        {
            Leaderboard board;
            auto t0 = Clock::now();
            for (const Entry& e : initial) board.updateScore(e.id, e.score);
            r.slBuild = microsSince(t0) / n;

            t0 = Clock::now();
            for (int i = 0; i < SL_UPDATES; ++i) sink = board.updateScore(id(updIds[i]), updScores[i]);
            r.slUpdate = microsSince(t0) / SL_UPDATES;

            t0 = Clock::now();
            for (int i = 0; i < RANK_OPS; ++i) sink = board.getRank(id(rankIds[i]));
            r.slRank = microsSince(t0) / RANK_OPS;

            t0 = Clock::now();
            for (int i = 0; i < TOPK_OPS; ++i) sink = board.topK(K).size();
            r.slTop = microsSince(t0) / TOPK_OPS;
        }

        // ---- Sorted vector (updates are O(n), so run fewer of them for large n) ----
        {
            NaiveLeaderboard naive;
            naive.bulkLoad(initial);
            int nvOps = std::max(100, std::min(SL_UPDATES, 200000000 / n));
            r.nvUpdateOps = nvOps;

            auto t0 = Clock::now();
            for (int i = 0; i < nvOps; ++i) sink = naive.updateScore(id(updIds[i]), updScores[i]);
            r.nvUpdate = microsSince(t0) / nvOps;

            t0 = Clock::now();
            for (int i = 0; i < RANK_OPS; ++i) sink = naive.getRank(id(rankIds[i]));
            r.nvRank = microsSince(t0) / RANK_OPS;

            t0 = Clock::now();
            for (int i = 0; i < TOPK_OPS; ++i) sink = naive.topK(K).size();
            r.nvTop = microsSince(t0) / TOPK_OPS;
        }
        rows.push_back(r);
    }

    std::printf("\nAverage time per operation in microseconds (lower is better).\n");
    std::printf("Updates are random score changes for existing players. Top-K uses K = %zu.\n\n", K);
    std::printf("| Players (n) | Skip List update | Sorted Array update | Skip List rank | Sorted Array rank | Skip List top-K | Sorted Array top-K |\n");
    std::printf("|---|---|---|---|---|---|---|\n");
    for (const Row& r : rows) {
        std::printf("| %d | %.2f | %.2f | %.2f | %.2f | %.2f | %.2f |\n", r.n, r.slUpdate, r.nvUpdate,
                    r.slRank, r.nvRank, r.slTop, r.nvTop);
    }
    std::printf("\nSorted array update averages use %d, %d and %d operations for the three sizes ",
                rows[0].nvUpdateOps, rows[1].nvUpdateOps, rows[2].nvUpdateOps);
    std::printf("(each update is O(n), so fewer runs keep the benchmark short).\n");
    return 0;
}
