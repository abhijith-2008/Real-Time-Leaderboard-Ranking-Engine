#pragma once
// SHARED CONTRACT. Do not edit without telling the whole team.
#include <cstddef>
#include <string>

namespace lb {

using PlayerId = std::string;
using Score = long long;

constexpr int MAX_LEVEL = 32;       // maximum skip list height
constexpr double P = 0.25;          // promotion probability

struct Entry {
    PlayerId id;
    Score score = 0;
};

// THE ONLY ORDERING RULE in the whole project.
// Score descending, ties broken by id ascending.
// before(a, b) is true when a must appear strictly before b (a has the better rank).
inline bool before(const Entry& a, const Entry& b) {
    if (a.score != b.score) return a.score > b.score;
    return a.id < b.id;
}

inline bool sameEntry(const Entry& a, const Entry& b) {
    return a.score == b.score && a.id == b.id;
}

}  // namespace lb
