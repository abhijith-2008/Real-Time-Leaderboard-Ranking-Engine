// OWNER: Rajaneesh. Tests for getRank, getByRank, topK, neighbours, rangeByRank, rangeByScore.
// Every answer is compared with a plain sorted std::vector.
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "IndexableSkipList.h"

using namespace lb;

static int failures = 0;
#define CHECK(cond)                                                                \
    do {                                                                           \
        if (!(cond)) {                                                             \
            std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << "  " #cond "\n"; \
            ++failures;                                                            \
        }                                                                          \
    } while (0)

static std::string id(int i) {
    std::string s = std::to_string(i);
    return "p" + std::string(6 - s.size(), '0') + s;
}

static bool sameVec(const std::vector<Entry>& a, const std::vector<Entry>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) if (!sameEntry(a[i], b[i])) return false;
    return true;
}

static std::vector<Entry> slice(const std::vector<Entry>& v, size_t from, size_t to) {  // 1 indexed, inclusive
    size_t n = v.size();
    if (from == 0) from = 1;
    if (to > n) to = n;
    if (from > to) return {};
    return std::vector<Entry>(v.begin() + (from - 1), v.begin() + to);
}

static void runCase(int n, unsigned seed, Score scoreRange) {
    std::mt19937 rng(seed);
    IndexableSkipList s(seed);
    std::vector<Entry> ref;
    for (int i = 0; i < n; ++i) {
        Entry e{id(i), static_cast<Score>(rng() % scoreRange)};
        s.insert(e);
        ref.push_back(e);
    }
    std::sort(ref.begin(), ref.end(), [](const Entry& a, const Entry& b) { return before(a, b); });
    const size_t N = ref.size();

    // getRank and getByRank for every position.
    for (size_t i = 0; i < N; ++i) {
        CHECK(s.getRank(ref[i]) == i + 1);
        Entry out;
        CHECK(s.getByRank(i + 1, out));
        CHECK(sameEntry(out, ref[i]));
    }
    // Absent entries and out of range ranks.
    CHECK(s.getRank(Entry{"zzz-not-here", 5}) == 0);
    if (N > 0) CHECK(s.getRank(Entry{ref[0].id, ref[0].score + 1}) == 0);  // right id, wrong score
    Entry out;
    CHECK(!s.getByRank(0, out));
    CHECK(!s.getByRank(N + 1, out));
    CHECK(!s.getByRank(N + 1000, out));

    // topK
    for (size_t k : {size_t(0), size_t(1), size_t(2), size_t(10), N ? N - 1 : 0, N, N + 1, N + 1000}) {
        CHECK(sameVec(s.topK(k), slice(ref, 1, k)));
    }

    // neighbours
    CHECK(s.neighbours(Entry{"zzz-not-here", 5}, 3).empty());
    for (int t = 0; t < 40 && N > 0; ++t) {
        size_t idx = rng() % N;
        for (size_t radius : {size_t(0), size_t(1), size_t(3), size_t(7), N, SIZE_MAX}) {
            size_t r = idx + 1;
            size_t rr = std::min(radius, N);
            size_t lo = (r > rr) ? r - rr : 1;
            size_t hi = std::min(N, r + rr);
            CHECK(sameVec(s.neighbours(ref[idx], radius), slice(ref, lo, hi)));
        }
    }

    // rangeByRank, including nonsense ranges
    for (int t = 0; t < 200; ++t) {
        size_t from = rng() % (N + 4), to = rng() % (N + 4);
        CHECK(sameVec(s.rangeByRank(from, to), slice(ref, from, to)));
    }

    // rangeByScore, including min > max
    for (int t = 0; t < 200; ++t) {
        Score a = static_cast<Score>(rng() % (scoreRange + 10)) - 5;
        Score b = static_cast<Score>(rng() % (scoreRange + 10)) - 5;
        std::vector<Entry> expect;
        for (const Entry& e : ref) if (e.score >= a && e.score <= b) expect.push_back(e);
        CHECK(sameVec(s.rangeByScore(a, b), expect));
    }
}

int main() {
    runCase(0, 1, 10);      // empty
    runCase(1, 2, 10);
    runCase(2, 3, 2);       // two entries, heavy ties
    runCase(50, 4, 5);      // many ties
    runCase(1000, 5, 40);   // many ties
    runCase(2000, 6, 1000000);
    if (failures) { std::cerr << failures << " check(s) failed\n"; return 1; }
    std::cout << "test_query: all passed\n";
    return 0;
}
