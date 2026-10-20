// OWNER: Rajaneesh. Query operations: getRank, getByRank, topK, neighbours, rangeByRank, rangeByScore.
#include "IndexableSkipList.h"

namespace lb {

// TODO(Rajaneesh)
size_t IndexableSkipList::getRank(const Entry&) const { return 0; }

// TODO(Rajaneesh)
bool IndexableSkipList::getByRank(size_t, Entry&) const { return false; }

// TODO(Rajaneesh)
std::vector<Entry> IndexableSkipList::topK(size_t) const { return {}; }

// TODO(Rajaneesh)
std::vector<Entry> IndexableSkipList::neighbours(const Entry&, size_t) const { return {}; }

// TODO(Rajaneesh)
std::vector<Entry> IndexableSkipList::rangeByRank(size_t, size_t) const { return {}; }

// TODO(Rajaneesh)
std::vector<Entry> IndexableSkipList::rangeByScore(Score, Score) const { return {}; }

}  // namespace lb
