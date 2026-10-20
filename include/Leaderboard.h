#pragma once
// SHARED CONTRACT. Do not edit without telling the whole team.
#include <cstddef>
#include <unordered_map>
#include <vector>

#include "IndexableSkipList.h"
#include "types.h"

namespace lb {

// Game facing API. Keeps a hash map id -> score in sync with the skip list.
// Every function here is a thin layer over IndexableSkipList.
class Leaderboard {
public:
    explicit Leaderboard(unsigned seed = 12345);

    // Adds the player or changes their score. If the player exists with the
    // same score nothing changes. Otherwise: remove old entry, insert new one.
    // Returns the player's new 1 indexed rank.
    size_t updateScore(const PlayerId& id, Score score);
    // Returns false if the player is unknown.
    bool removePlayer(const PlayerId& id);
    // 1 indexed rank, or 0 if the player is unknown.
    size_t getRank(const PlayerId& id) const;
    // Returns false if the player is unknown.
    bool getScore(const PlayerId& id, Score& out) const;
    bool getByRank(size_t rank, Entry& out) const;
    std::vector<Entry> topK(size_t k) const;
    // Same meaning as IndexableSkipList::neighbours. Empty if unknown player.
    std::vector<Entry> neighbours(const PlayerId& id, size_t radius) const;
    size_t size() const;

    // Read only access so tests can call list().validate().
    const IndexableSkipList& list() const { return list_; }

private:
    IndexableSkipList list_;
    std::unordered_map<PlayerId, Score> scores_;
};

}  // namespace lb
