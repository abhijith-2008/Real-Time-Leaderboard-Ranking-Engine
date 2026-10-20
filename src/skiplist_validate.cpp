// OWNER: Santhosh. Debug invariant checker used by every test.
#include <string>
#include <unordered_map>

#include "IndexableSkipList.h"

namespace lb {

bool IndexableSkipList::validate(std::string* err) const {
    auto fail = [&](const std::string& msg) {
        if (err) *err = msg;
        return false;
    };

    if (level_ < 1 || level_ > MAX_LEVEL) return fail("level_ out of range: " + std::to_string(level_));
    if (head_->level.size() != static_cast<size_t>(MAX_LEVEL)) return fail("head does not have MAX_LEVEL levels");

    // (a) Level 0: assign positions, check strict ordering and length.
    std::unordered_map<const Node*, size_t> pos;
    pos[head_] = 0;
    size_t n = 0;
    const Node* prev = nullptr;
    for (const Node* x = head_->level[0].forward; x; x = x->level[0].forward) {
        ++n;
        if (n > length_) return fail("level 0 has more nodes than length_ (" + std::to_string(length_) + ")");
        if (prev && !before(prev->entry, x->entry)) {
            return fail("level 0 not strictly increasing at position " + std::to_string(n));
        }
        if (x->level.empty() || static_cast<int>(x->level.size()) > level_) {
            return fail("node at position " + std::to_string(n) + " has height outside 1..level_");
        }
        pos[x] = n;
        prev = x;
    }
    if (n != length_) {
        return fail("level 0 has " + std::to_string(n) + " nodes but length_ is " + std::to_string(length_));
    }

    // (c) The top level in use must not be empty.
    if (level_ > 1 && head_->level[level_ - 1].forward == nullptr) {
        return fail("level_ is " + std::to_string(level_) + " but that level is empty");
    }

    // (b) and (d) Every level: spans must match positions, and the level must contain
    // exactly the nodes tall enough to be on it.
    for (int i = 0; i < level_; ++i) {
        size_t expectedCount = 0;
        for (const Node* x = head_->level[0].forward; x; x = x->level[0].forward) {
            if (static_cast<int>(x->level.size()) > i) ++expectedCount;
        }

        size_t visited = 0;
        const Node* x = head_;
        size_t xp = 0;
        while (true) {
            const Node* f = x->level[i].forward;
            size_t expectedSpan;
            size_t fp = 0;
            if (f) {
                auto it = pos.find(f);
                if (it == pos.end()) return fail("level " + std::to_string(i) + " links to a node not on level 0");
                fp = it->second;
                if (static_cast<int>(f->level.size()) <= i) {
                    return fail("level " + std::to_string(i) + " links to a node that is too short");
                }
                if (fp <= xp) return fail("level " + std::to_string(i) + " is not increasing");
                expectedSpan = fp - xp;
            } else {
                expectedSpan = length_ - xp;
            }
            if (x->level[i].span != expectedSpan) {
                return fail("bad span at level " + std::to_string(i) + ", position " + std::to_string(xp) +
                            ": has " + std::to_string(x->level[i].span) + ", expected " +
                            std::to_string(expectedSpan));
            }
            if (!f) break;
            ++visited;
            x = f;
            xp = fp;
        }
        if (visited != expectedCount) {
            return fail("level " + std::to_string(i) + " has " + std::to_string(visited) +
                        " nodes but " + std::to_string(expectedCount) + " nodes are tall enough");
        }
    }
    return true;
}

}  // namespace lb
