// OWNER: Haripranav. Leaderboard wrapper around the skip list and the hash map.
#include "Leaderboard.h"

namespace lb {

Leaderboard::Leaderboard(unsigned seed) : list_(seed) {}

// TODO(Haripranav)
size_t Leaderboard::updateScore(const PlayerId&, Score) { return 0; }

// TODO(Haripranav)
bool Leaderboard::removePlayer(const PlayerId&) { return false; }

// TODO(Haripranav)
size_t Leaderboard::getRank(const PlayerId&) const { return 0; }

// TODO(Haripranav)
bool Leaderboard::getScore(const PlayerId&, Score&) const { return false; }

// TODO(Haripranav)
bool Leaderboard::getByRank(size_t, Entry&) const { return false; }

// TODO(Haripranav)
std::vector<Entry> Leaderboard::topK(size_t) const { return {}; }

// TODO(Haripranav)
std::vector<Entry> Leaderboard::neighbours(const PlayerId&, size_t) const { return {}; }

// TODO(Haripranav)
size_t Leaderboard::size() const { return 0; }

}  // namespace lb
