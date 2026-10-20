// OWNER: Abhijith. Tests for insert, remove and span maintenance.
// Uses only insert, remove, size and validate, so it does not depend on the query code.
#include <algorithm>
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

static void checkValid(const IndexableSkipList& s, const char* where) {
    std::string err;
    if (!s.validate(&err)) {
        std::cerr << "INVALID after " << where << ": " << err << "\n";
        ++failures;
    }
}

static std::string id(int i) {
    std::string s = std::to_string(i);
    return "p" + std::string(6 - s.size(), '0') + s;
}

static void testEmpty() {
    IndexableSkipList s;
    CHECK(s.size() == 0);
    CHECK(!s.remove(Entry{"nobody", 10}));
    checkValid(s, "empty list");
}

static void testDuplicates() {
    IndexableSkipList s;
    CHECK(s.insert(Entry{"alice", 100}));
    CHECK(!s.insert(Entry{"alice", 100}));  // exact duplicate rejected
    CHECK(s.size() == 1);
    CHECK(s.insert(Entry{"alice", 200}));   // same id, different score is a different entry here
    CHECK(s.insert(Entry{"bob", 100}));     // same score, different id
    CHECK(s.size() == 3);
    checkValid(s, "duplicates");
    CHECK(!s.remove(Entry{"alice", 999}));  // present id, wrong score
    CHECK(s.remove(Entry{"alice", 100}));
    CHECK(s.size() == 2);
    checkValid(s, "remove after duplicates");
}

static void testOrders() {
    for (int mode = 0; mode < 3; ++mode) {  // ascending, descending, random
        IndexableSkipList s(7 + mode);
        std::vector<int> v(300);
        for (int i = 0; i < 300; ++i) v[i] = i;
        if (mode == 1) std::reverse(v.begin(), v.end());
        if (mode == 2) { std::mt19937 g(5); std::shuffle(v.begin(), v.end(), g); }
        for (int x : v) {
            CHECK(s.insert(Entry{id(x), x}));
            checkValid(s, "ordered insert");
        }
        CHECK(s.size() == 300);
        std::mt19937 g(9);
        std::shuffle(v.begin(), v.end(), g);
        size_t left = 300;
        for (int x : v) {
            CHECK(s.remove(Entry{id(x), x}));
            CHECK(s.size() == --left);
            checkValid(s, "ordered remove");
        }
        CHECK(s.size() == 0);
    }
}

// Many operations on a small id pool with few distinct scores, so ties are everywhere.
static void testRandomMixed(unsigned seed) {
    IndexableSkipList s(seed);
    std::mt19937 rng(seed * 31 + 1);
    std::vector<Entry> ref;  // unordered reference, only used for membership and size
    auto find = [&](const Entry& e) {
        return std::find_if(ref.begin(), ref.end(), [&](const Entry& r) { return sameEntry(r, e); });
    };
    for (int op = 0; op < 20000; ++op) {
        Entry e{id(static_cast<int>(rng() % 400)), static_cast<Score>(rng() % 30)};
        auto it = find(e);
        if (rng() % 100 < 55) {
            bool inserted = s.insert(e);
            CHECK(inserted == (it == ref.end()));
            if (it == ref.end()) ref.push_back(e);
        } else {
            bool removed = s.remove(e);
            CHECK(removed == (it != ref.end()));
            if (it != ref.end()) ref.erase(it);
        }
        CHECK(s.size() == ref.size());
        if (op % 10 == 0) checkValid(s, "random mixed");
    }
    checkValid(s, "random mixed end");
}

static void testLarge() {
    IndexableSkipList s(123);
    std::mt19937 rng(77);
    const int N = 100000;
    std::vector<Entry> all;
    all.reserve(N);
    for (int i = 0; i < N; ++i) {
        Entry e{id(i), static_cast<Score>(rng() % 1000000)};
        CHECK(s.insert(e));
        all.push_back(e);
    }
    CHECK(s.size() == static_cast<size_t>(N));
    checkValid(s, "large insert");

    std::shuffle(all.begin(), all.end(), rng);
    for (const Entry& e : all) CHECK(s.remove(e));
    CHECK(s.size() == 0);
    checkValid(s, "large remove all");
}

int main() {
    testEmpty();
    testDuplicates();
    testOrders();
    for (unsigned seed = 1; seed <= 5; ++seed) testRandomMixed(seed);
    testLarge();
    if (failures) { std::cerr << failures << " check(s) failed\n"; return 1; }
    std::cout << "test_core: all passed\n";
    return 0;
}
