// OWNER: Abhijith. Core structure: constructor, destructor, randomLevel, insert, remove, size.
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

// TODO(Abhijith)
int IndexableSkipList::randomLevel() { return 1; }

// TODO(Abhijith)
bool IndexableSkipList::insert(const Entry&) { return false; }

// TODO(Abhijith)
bool IndexableSkipList::remove(const Entry&) { return false; }

size_t IndexableSkipList::size() const { return length_; }

}  // namespace lb
