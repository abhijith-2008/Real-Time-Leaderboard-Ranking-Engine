// OWNER: Rajaneesh. Query operations: getRank, getByRank, topK, neighbours, rangeByRank, rangeByScore.
#include <algorithm>

#include "IndexableSkipList.h"

namespace lb {

size_t IndexableSkipList::getRank(const Entry& e) const {
    size_t rank = 0;
    const Node* x = head_;
    for (int i = level_ - 1; i >= 0; --i) {
        // Advance while the next node is at or before e, adding up the spans.
        while (x->level[i].forward &&
               (before(x->level[i].forward->entry, e) || sameEntry(x->level[i].forward->entry, e))) {
            rank += x->level[i].span;
            x = x->level[i].forward;
        }
    }
    if (x != head_ && sameEntry(x->entry, e)) return rank;
    return 0;
}

bool IndexableSkipList::getByRank(size_t rank, Entry& out) const {
    if (rank == 0 || rank > length_) return false;
    size_t traversed = 0;
    const Node* x = head_;
    for (int i = level_ - 1; i >= 0; --i) {
        // Follow a link only if it does not overshoot the target rank.
        while (x->level[i].forward && traversed + x->level[i].span <= rank) {
            traversed += x->level[i].span;
            x = x->level[i].forward;
        }
        if (traversed == rank) break;
    }
    out = x->entry;
    return true;
}

std::vector<Entry> IndexableSkipList::topK(size_t k) const {
    std::vector<Entry> out;
    size_t n = std::min(k, length_);
    out.reserve(n);
    const Node* x = head_->level[0].forward;
    while (x && out.size() < n) {
        out.push_back(x->entry);
        x = x->level[0].forward;
    }
    return out;
}

std::vector<Entry> IndexableSkipList::neighbours(const Entry& e, size_t radius) const {
    size_t r = getRank(e);
    if (r == 0) return {};
    size_t lo = (r > radius) ? r - radius : 1;
    size_t hi = (radius >= length_ - r) ? length_ : r + radius;  // avoids overflow for huge radius
    return rangeByRank(lo, hi);
}

std::vector<Entry> IndexableSkipList::rangeByRank(size_t from, size_t to) const {
    std::vector<Entry> out;
    if (from == 0) from = 1;
    if (to > length_) to = length_;
    if (from > to) return out;

    // Jump to the node at rank "from" using spans, then walk level 0.
    size_t traversed = 0;
    const Node* x = head_;
    for (int i = level_ - 1; i >= 0; --i) {
        while (x->level[i].forward && traversed + x->level[i].span <= from) {
            traversed += x->level[i].span;
            x = x->level[i].forward;
        }
    }
    out.reserve(to - from + 1);
    for (size_t r = from; r <= to && x; ++r) {
        out.push_back(x->entry);
        x = x->level[0].forward;
    }
    return out;
}

std::vector<Entry> IndexableSkipList::rangeByScore(Score minScore, Score maxScore) const {
    std::vector<Entry> out;
    if (minScore > maxScore) return out;

    // Entries are sorted by score descending, so skip everything above maxScore first.
    const Node* x = head_;
    for (int i = level_ - 1; i >= 0; --i) {
        while (x->level[i].forward && x->level[i].forward->entry.score > maxScore) {
            x = x->level[i].forward;
        }
    }
    x = x->level[0].forward;
    while (x && x->entry.score >= minScore) {
        out.push_back(x->entry);
        x = x->level[0].forward;
    }
    return out;
}

}  // namespace lb
