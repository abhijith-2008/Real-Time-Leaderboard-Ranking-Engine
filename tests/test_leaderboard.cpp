// OWNER: Haripranav. Tests for the Leaderboard wrapper (score updates, ties, removals).
#include <iostream>
#include <string>
#include <vector>

#include "Leaderboard.h"

using namespace lb;

static int failures = 0;
#define CHECK(cond)                                                                \
    do {                                                                           \
        if (!(cond)) {                                                             \
            std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << "  " #cond "\n"; \
            ++failures;                                                            \
        }                                                                          \
    } while (0)

static void checkValid(const Leaderboard& b, const char* where) {
    std::string err;
    if (!b.list().validate(&err)) {
        std::cerr << "INVALID after " << where << ": " << err << "\n";
        ++failures;
    }
}

static std::vector<std::string> names(const std::vector<Entry>& v) {
    std::vector<std::string> out;
    for (const Entry& e : v) out.push_back(e.id);
    return out;
}

using Names = std::vector<std::string>;

int main() {
    Leaderboard b;

    // The example from the README.
    CHECK(b.updateScore("Alice", 1500) == 1);
    CHECK(b.updateScore("Bob", 2300) == 1);
    CHECK(b.updateScore("Charlie", 1800) == 2);
    CHECK(b.updateScore("Dave", 2300) == 2);  // ties with Bob, "Bob" < "Dave" so Bob stays first
    CHECK(b.size() == 4);
    CHECK(names(b.topK(3)) == (Names{"Bob", "Dave", "Charlie"}));
    CHECK(b.getRank("Alice") == 4);
    checkValid(b, "initial adds");

    // Score update moves a player.
    CHECK(b.updateScore("Alice", 2000) == 3);
    CHECK(b.getRank("Alice") == 3);
    CHECK(b.getRank("Charlie") == 4);
    CHECK(names(b.neighbours("Charlie", 1)) == (Names{"Alice", "Charlie"}));
    CHECK(names(b.neighbours("Alice", 1)) == (Names{"Dave", "Alice", "Charlie"}));
    CHECK(names(b.neighbours("Alice", 100)) == (Names{"Bob", "Dave", "Alice", "Charlie"}));
    checkValid(b, "update");

    // Updating to the same score changes nothing.
    CHECK(b.updateScore("Alice", 2000) == 3);
    CHECK(b.size() == 4);

    // Score lookups.
    Score s = 0;
    CHECK(b.getScore("Alice", s) && s == 2000);
    CHECK(!b.getScore("Nobody", s));

    // getByRank.
    Entry e;
    CHECK(b.getByRank(1, e) && e.id == "Bob" && e.score == 2300);
    CHECK(!b.getByRank(0, e));
    CHECK(!b.getByRank(5, e));

    // Unknown players.
    CHECK(b.getRank("Nobody") == 0);
    CHECK(!b.removePlayer("Nobody"));
    CHECK(b.neighbours("Nobody", 2).empty());

    // Removing a player shifts everybody below them up, and re adding works.
    CHECK(b.removePlayer("Dave"));
    CHECK(b.size() == 3);
    CHECK(b.getRank("Dave") == 0);
    CHECK(b.getRank("Alice") == 2);
    CHECK(b.updateScore("Dave", 100) == 4);  // lowest score, so last
    CHECK(b.getRank("Dave") == 4);
    checkValid(b, "remove and re add");

    // k larger than size, k = 0.
    CHECK(b.topK(100).size() == 4);
    CHECK(b.topK(0).empty());

    // Empty leaderboard.
    Leaderboard empty;
    CHECK(empty.size() == 0);
    CHECK(empty.topK(5).empty());
    CHECK(empty.getRank("x") == 0);
    CHECK(!empty.getByRank(1, e));

    // Many players with the same score are ordered by id ascending.
    Leaderboard ties;
    for (char c = 'j'; c >= 'a'; --c) ties.updateScore(std::string(1, c), 50);
    CHECK(names(ties.topK(10)) == (Names{"a", "b", "c", "d", "e", "f", "g", "h", "i", "j"}));
    CHECK(ties.getRank("e") == 5);
    checkValid(ties, "ties");

    if (failures) { std::cerr << failures << " check(s) failed\n"; return 1; }
    std::cout << "test_leaderboard: all passed\n";
    return 0;
}
