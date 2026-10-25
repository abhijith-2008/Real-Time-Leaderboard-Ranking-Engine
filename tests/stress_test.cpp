// OWNER: Santhosh. Randomised stress test: Leaderboard versus the naive sorted vector.
// Usage: ./build/stress_test [seed] [operations]
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "Leaderboard.h"
#include "naive_leaderboard.h"

using namespace lb;

static std::string id(int i) {
    std::string s = std::to_string(i);
    return "p" + std::string(5 - s.size(), '0') + s;
}

static bool sameVec(const std::vector<Entry>& a, const std::vector<Entry>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) if (!sameEntry(a[i], b[i])) return false;
    return true;
}

int main(int argc, char** argv) {
    unsigned seed = (argc > 1) ? static_cast<unsigned>(std::strtoul(argv[1], nullptr, 10)) : 42;
    int ops = (argc > 2) ? std::atoi(argv[2]) : 100000;
    std::cout << "stress_test seed=" << seed << " ops=" << ops << "\n";

    std::mt19937 rng(seed);
    Leaderboard board(seed);
    NaiveLeaderboard naive;
    const int POOL = 3000;       // distinct players
    const int SCORES = 300;      // few distinct scores, so lots of ties
    int counts[8] = {0};

    auto fail = [&](int op, const char* what) {
        std::cerr << "MISMATCH at op " << op << " (" << what << "), seed " << seed << "\n";
        return 1;
    };

    for (int op = 0; op < ops; ++op) {
        int kind = static_cast<int>(rng() % 100);
        PlayerId p = id(static_cast<int>(rng() % POOL));

        if (kind < 45) {
            ++counts[0];
            Score sc = static_cast<Score>(rng() % SCORES);
            if (board.updateScore(p, sc) != naive.updateScore(p, sc)) return fail(op, "updateScore rank");
        } else if (kind < 55) {
            ++counts[1];
            if (board.removePlayer(p) != naive.removePlayer(p)) return fail(op, "removePlayer");
        } else if (kind < 65) {
            ++counts[2];
            if (board.getRank(p) != naive.getRank(p)) return fail(op, "getRank");
        } else if (kind < 75) {
            ++counts[3];
            size_t r = rng() % (naive.size() + 3);
            Entry a, b;
            bool ra = board.getByRank(r, a), rb = naive.getByRank(r, b);
            if (ra != rb || (ra && !sameEntry(a, b))) return fail(op, "getByRank");
        } else if (kind < 85) {
            ++counts[4];
            size_t k = rng() % (naive.size() + 3);
            if (!sameVec(board.topK(k), naive.topK(k))) return fail(op, "topK");
        } else if (kind < 95) {
            ++counts[5];
            size_t radius = rng() % 12;
            if (!sameVec(board.neighbours(p, radius), naive.neighbours(p, radius))) return fail(op, "neighbours");
        } else {
            ++counts[6];
            Score a = -1, b = -1;
            bool ra = board.getScore(p, a), rb = naive.getScore(p, b);
            if (ra != rb || (ra && a != b)) return fail(op, "getScore");
        }

        if (board.size() != naive.size()) return fail(op, "size");

        if (op < 5000 || op % 1000 == 0) {
            std::string err;
            if (!board.list().validate(&err)) {
                std::cerr << "INVALID structure at op " << op << ": " << err << " (seed " << seed << ")\n";
                return 1;
            }
        }
    }

    std::string err;
    if (!board.list().validate(&err)) { std::cerr << "INVALID at end: " << err << "\n"; return 1; }
    std::cout << "stress_test: passed. updates=" << counts[0] << " removes=" << counts[1]
              << " ranks=" << counts[2] << " byRank=" << counts[3] << " topK=" << counts[4]
              << " neighbours=" << counts[5] << " scores=" << counts[6]
              << " final size=" << board.size() << "\n";
    return 0;
}
