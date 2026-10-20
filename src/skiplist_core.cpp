// OWNER: Abhijith. Core structure: constructor, destructor, randomLevel, insert, remove, size.
#include <cstdint>

#include "IndexableSkipList.h"

namespace lb {

IndexableSkipList::IndexableSkipList(unsigned seed)
    : head_(new Node(Entry{}, MAX_LEVEL)), level_(1), length_(0), rng_(seed) {}

IndexableSkipList::~IndexableSkipList() {
    Node* cur = head_;
    while (cur) {
        Node* next = cur->level[0].forward;
        delete cur;
        cur = next;
    }
}

// Geometric height: each extra level is added with probability P.
int IndexableSkipList::randomLevel() {
    static const std::uint32_t threshold =
        static_cast<std::uint32_t>(P * 4294967296.0);  // P * 2^32
    int lvl = 1;
    while (lvl < MAX_LEVEL && rng_() < threshold) ++lvl;
    return lvl;
}

bool IndexableSkipList::insert(const Entry& e) {
    Node* update[MAX_LEVEL];   // last node before the insertion point, per level
    size_t rank[MAX_LEVEL];    // position of update[i] (sum of spans travelled)

    Node* x = head_;
    for (int i = level_ - 1; i >= 0; --i) {
        rank[i] = (i == level_ - 1) ? 0 : rank[i + 1];
        while (x->level[i].forward && before(x->level[i].forward->entry, e)) {
            rank[i] += x->level[i].span;
            x = x->level[i].forward;
        }
        update[i] = x;
    }

    Node* next = x->level[0].forward;
    if (next && sameEntry(next->entry, e)) return false;  // duplicate

    int h = randomLevel();
    if (h > level_) {
        // New top levels: head points straight to nil, spanning the whole list.
        for (int i = level_; i < h; ++i) {
            rank[i] = 0;
            update[i] = head_;
            head_->level[i].span = length_;
        }
        level_ = h;
    }

    Node* n = new Node(e, h);
    for (int i = 0; i < h; ++i) {
        n->level[i].forward = update[i]->level[i].forward;
        update[i]->level[i].forward = n;

        // rank[0] - rank[i] = how many level 0 steps lie between update[i] and update[0].
        n->level[i].span = update[i]->level[i].span - (rank[0] - rank[i]);
        update[i]->level[i].span = (rank[0] - rank[i]) + 1;
    }
    // Links above the new node's height now jump over one more node.
    for (int i = h; i < level_; ++i) update[i]->level[i].span++;

    ++length_;
    return true;
}

bool IndexableSkipList::remove(const Entry& e) {
    Node* update[MAX_LEVEL];
    Node* x = head_;
    for (int i = level_ - 1; i >= 0; --i) {
        while (x->level[i].forward && before(x->level[i].forward->entry, e)) {
            x = x->level[i].forward;
        }
        update[i] = x;
    }

    Node* target = x->level[0].forward;
    if (!target || !sameEntry(target->entry, e)) return false;

    for (int i = 0; i < level_; ++i) {
        if (update[i]->level[i].forward == target) {
            // Unlink: the predecessor now jumps over the target's span, minus the target itself.
            update[i]->level[i].span = update[i]->level[i].span + target->level[i].span - 1;
            update[i]->level[i].forward = target->level[i].forward;
        } else {
            // This link jumps over the target without touching it.
            update[i]->level[i].span -= 1;
        }
    }
    delete target;

    while (level_ > 1 && head_->level[level_ - 1].forward == nullptr) --level_;
    --length_;
    return true;
}

size_t IndexableSkipList::size() const { return length_; }

}  // namespace lb
