// OWNER: Santhosh. Brute force baseline: a sorted std::vector.
// Same API and semantics as lb::Leaderboard. Used as the reference in stress_test
// and as the competitor in the benchmark.
//   update: O(n)   (shifting elements in the vector)
//   rank:   O(log n) (binary search)
//   top-K:  O(K)
#pragma once
#include <algorithm>
#include <unordered_map>
#include <vector>

#include "types.h"

namespace lb {

class NaiveLeaderboard {
public:
    size_t updateScore(const PlayerId& id, Score score) {
        auto it = scores_.find(id);
        if (it != scores_.end()) {
            if (it->second == score) return rankOf(Entry{id, score});
            v_.erase(v_.begin() + indexOf(Entry{id, it->second}));
            it->second = score;
        } else {
            scores_.emplace(id, score);
        }
        size_t idx = lowerBound(Entry{id, score});
        v_.insert(v_.begin() + idx, Entry{id, score});
        return idx + 1;
    }

    bool removePlayer(const PlayerId& id) {
        auto it = scores_.find(id);
        if (it == scores_.end()) return false;
        v_.erase(v_.begin() + indexOf(Entry{id, it->second}));
        scores_.erase(it);
        return true;
    }

    size_t getRank(const PlayerId& id) const {
        auto it = scores_.find(id);
        if (it == scores_.end()) return 0;
        return rankOf(Entry{id, it->second});
    }

    bool getScore(const PlayerId& id, Score& out) const {
        auto it = scores_.find(id);
        if (it == scores_.end()) return false;
        out = it->second;
        return true;
    }

    bool getByRank(size_t rank, Entry& out) const {
        if (rank == 0 || rank > v_.size()) return false;
        out = v_[rank - 1];
        return true;
    }

    std::vector<Entry> topK(size_t k) const {
        size_t n = std::min(k, v_.size());
        return std::vector<Entry>(v_.begin(), v_.begin() + n);
    }

    std::vector<Entry> neighbours(const PlayerId& id, size_t radius) const {
        size_t r = getRank(id);
        if (r == 0) return {};
        size_t n = v_.size();
        size_t rr = std::min(radius, n);
        size_t lo = (r > rr) ? r - rr : 1;
        size_t hi = std::min(n, r + rr);
        return std::vector<Entry>(v_.begin() + (lo - 1), v_.begin() + hi);
    }

    size_t size() const { return v_.size(); }

    // Fast initial load for benchmarks (one sort instead of n slow inserts).
    void bulkLoad(std::vector<Entry> entries) {
        v_ = std::move(entries);
        std::sort(v_.begin(), v_.end(), [](const Entry& a, const Entry& b) { return before(a, b); });
        scores_.clear();
        for (const Entry& e : v_) scores_[e.id] = e.score;
    }

private:
    size_t lowerBound(const Entry& e) const {
        return std::lower_bound(v_.begin(), v_.end(), e,
                                [](const Entry& a, const Entry& b) { return before(a, b); }) -
               v_.begin();
    }
    size_t indexOf(const Entry& e) const { return lowerBound(e); }  // e must be present
    size_t rankOf(const Entry& e) const {
        size_t idx = lowerBound(e);
        return (idx < v_.size() && sameEntry(v_[idx], e)) ? idx + 1 : 0;
    }

    std::vector<Entry> v_;
    std::unordered_map<PlayerId, Score> scores_;
};

}  // namespace lb
