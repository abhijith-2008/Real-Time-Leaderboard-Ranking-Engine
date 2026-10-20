#pragma once
// SHARED CONTRACT. Do not edit without telling the whole team.
#include <cstddef>
#include <random>
#include <string>
#include <vector>

#include "types.h"

namespace lb {

// Skip list ordered by before() (see types.h), augmented with rank spans.
//
// POSITIONS AND RANKS
//   Head is position 0. Real nodes occupy positions 1..length at level 0.
//   A rank is a position. Ranks are 1 indexed and always unique (ties are
//   broken by id, so two players with equal scores still get different ranks).
//
// SPAN INVARIANT (checked by validate())
//   For a node u at position pos and a level i < level_:
//     if u.level[i].forward != nullptr, with the forward node at position fpos:
//         u.level[i].span == fpos - pos
//     if u.level[i].forward == nullptr:
//         u.level[i].span == length_ - pos     (number of nodes after u)
//   Consequences: level 0 spans are 1 for every node except the last, whose
//   span is 0. Summing spans along any search path gives the rank reached.
//
// OTHER INVARIANTS
//   * Level 0 is strictly increasing under before().
//   * Each higher level is a sublist of the level below it.
//   * level_ is the highest level in use (1..MAX_LEVEL). The head always
//     has MAX_LEVEL levels allocated, but only levels < level_ are meaningful.
//   * length_ equals the number of real nodes.
class IndexableSkipList {
public:
    explicit IndexableSkipList(unsigned seed = 12345);
    ~IndexableSkipList();
    IndexableSkipList(const IndexableSkipList&) = delete;
    IndexableSkipList& operator=(const IndexableSkipList&) = delete;

    // ---- core: src/skiplist_core.cpp ----
    // Inserts e. Returns false (and changes nothing) if an entry with the same
    // id AND score already exists.
    bool insert(const Entry& e);
    // Removes the entry equal to e (same id AND score). Returns false if absent.
    bool remove(const Entry& e);
    size_t size() const;

    // ---- queries: src/skiplist_query.cpp ----
    // 1 indexed rank of the entry equal to e, or 0 if it is not present.
    size_t getRank(const Entry& e) const;
    // Fills out with the entry at 1 indexed rank. Returns false if rank is 0
    // or greater than size().
    bool getByRank(size_t rank, Entry& out) const;
    // First min(k, size()) entries in rank order.
    std::vector<Entry> topK(size_t k) const;
    // Entries with ranks in [max(1, r - radius), min(size(), r + radius)] where
    // r is the rank of e. Includes e itself. Empty if e is not present.
    std::vector<Entry> neighbours(const Entry& e, size_t radius) const;
    // Entries with ranks in [from, to], inclusive, 1 indexed, clamped to
    // [1, size()]. Empty if from > to or from > size().
    std::vector<Entry> rangeByRank(size_t from, size_t to) const;
    // Entries with minScore <= score <= maxScore, in rank order (best first).
    std::vector<Entry> rangeByScore(Score minScore, Score maxScore) const;

    // ---- debug: src/skiplist_validate.cpp ----
    // Checks every invariant listed above. On failure returns false and, if err
    // is not null, writes a human readable reason into *err.
    bool validate(std::string* err = nullptr) const;

private:
    struct Node;
    struct Level {
        Node* forward = nullptr;
        size_t span = 0;
    };
    struct Node {
        Entry entry;
        std::vector<Level> level;
        Node(const Entry& e, int height) : entry(e), level(height) {}
    };

    int randomLevel();  // returns 1..MAX_LEVEL, geometric with probability P

    Node* head_;
    int level_;
    size_t length_;
    std::mt19937 rng_;
};

}  // namespace lb
