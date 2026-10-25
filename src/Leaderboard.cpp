// OWNER: Haripranav. Leaderboard wrapper around the skip list and the hash map.
#include "Leaderboard.h"

namespace lb {

Leaderboard::Leaderboard(unsigned seed) : list_(seed) {}

size_t Leaderboard::updateScore(const PlayerId& id, Score score) {
    auto it = scores_.find(id);
    if (it != scores_.end()) {
        if (it->second == score) return list_.getRank(Entry{id, score});  // nothing to change
        list_.remove(Entry{id, it->second});                               // delete the old entry first
        it->second = score;
    } else {
        scores_.emplace(id, score);
    }
    list_.insert(Entry{id, score});
    return list_.getRank(Entry{id, score});
}

bool Leaderboard::removePlayer(const PlayerId& id) {
    auto it = scores_.find(id);
    if (it == scores_.end()) return false;
    list_.remove(Entry{id, it->second});
    scores_.erase(it);
    return true;
}

size_t Leaderboard::getRank(const PlayerId& id) const {
    auto it = scores_.find(id);
    if (it == scores_.end()) return 0;
    return list_.getRank(Entry{id, it->second});
}

bool Leaderboard::getScore(const PlayerId& id, Score& out) const {
    auto it = scores_.find(id);
    if (it == scores_.end()) return false;
    out = it->second;
    return true;
}

bool Leaderboard::getByRank(size_t rank, Entry& out) const { return list_.getByRank(rank, out); }

std::vector<Entry> Leaderboard::topK(size_t k) const { return list_.topK(k); }

std::vector<Entry> Leaderboard::neighbours(const PlayerId& id, size_t radius) const {
    auto it = scores_.find(id);
    if (it == scores_.end()) return {};
    return list_.neighbours(Entry{id, it->second}, radius);
}

size_t Leaderboard::size() const { return scores_.size(); }

}  // namespace lb
