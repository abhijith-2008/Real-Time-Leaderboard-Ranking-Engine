// OWNER: Haripranav. CLI demo driver.
// Usage: ./build/leaderboard            (interactive)
//        ./build/leaderboard --no-prompt < commands.txt   (scripted)
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Leaderboard.h"

using namespace lb;

namespace {

bool parseScore(const std::string& s, Score& out) {
    try {
        size_t used = 0;
        long long v = std::stoll(s, &used);
        if (used != s.size()) return false;
        out = v;
        return true;
    } catch (...) {
        return false;
    }
}

bool parseCount(const std::string& s, size_t& out) {
    Score v;
    if (!parseScore(s, v) || v < 0) return false;
    out = static_cast<size_t>(v);
    return true;
}

void printRows(const std::vector<Entry>& rows, size_t firstRank) {
    for (size_t i = 0; i < rows.size(); ++i) {
        std::cout << firstRank + i << ". " << std::left << std::setw(12) << rows[i].id << " " << rows[i].score << "\n";
    }
}

void printHelp() {
    std::cout << "Commands (player names are a single word):\n"
                 "  add <name> <score>        add a new player\n"
                 "  update <name> <score>     change a score (adds the player if new)\n"
                 "  remove <name>             remove a player\n"
                 "  rank <name>               show a player's rank and score\n"
                 "  score <name>              show a player's score\n"
                 "  top <k>                   show the top k players\n"
                 "  around <name> [radius]    show players ranked near a player (default radius 5)\n"
                 "  at <rank>                 show the player at a rank\n"
                 "  size                      number of players\n"
                 "  help, quit\n";
}

}  // namespace

int main(int argc, char** argv) {
    bool prompt = !(argc > 1 && std::string(argv[1]) == "--no-prompt");
    Leaderboard board;
    if (prompt) std::cout << "Leaderboard ready. Type 'help' for commands.\n";

    std::string line;
    while (true) {
        if (prompt) std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) break;

        std::istringstream in(line);
        std::string cmd;
        if (!(in >> cmd)) continue;
        std::vector<std::string> args;
        for (std::string a; in >> a;) args.push_back(a);

        if (cmd == "quit" || cmd == "exit") {
            break;
        } else if (cmd == "help") {
            printHelp();
        } else if (cmd == "add" || cmd == "update") {
            Score score;
            if (args.size() != 2 || !parseScore(args[1], score)) {
                std::cout << "Usage: " << cmd << " <name> <score>\n";
                continue;
            }
            const std::string& name = args[0];
            size_t oldRank = board.getRank(name);
            if (cmd == "add" && oldRank != 0) {
                std::cout << name << " already exists, use 'update' to change the score\n";
                continue;
            }
            size_t newRank = board.updateScore(name, score);
            if (oldRank == 0) {
                std::cout << name << " added at #" << newRank << " with score " << score << "\n";
            } else if (oldRank == newRank) {
                std::cout << name << " stays at #" << newRank << " with score " << score << "\n";
            } else {
                std::cout << name << " moved from #" << oldRank << " to #" << newRank << "\n";
            }
        } else if (cmd == "remove") {
            if (args.size() != 1) { std::cout << "Usage: remove <name>\n"; continue; }
            if (board.removePlayer(args[0])) std::cout << "Removed " << args[0] << "\n";
            else std::cout << "Unknown player " << args[0] << "\n";
        } else if (cmd == "rank") {
            if (args.size() != 1) { std::cout << "Usage: rank <name>\n"; continue; }
            Score s;
            if (!board.getScore(args[0], s)) { std::cout << "Unknown player " << args[0] << "\n"; continue; }
            std::cout << args[0] << " is ranked #" << board.getRank(args[0]) << " with score " << s << "\n";
        } else if (cmd == "score") {
            if (args.size() != 1) { std::cout << "Usage: score <name>\n"; continue; }
            Score s;
            if (!board.getScore(args[0], s)) std::cout << "Unknown player " << args[0] << "\n";
            else std::cout << args[0] << " has score " << s << "\n";
        } else if (cmd == "top") {
            size_t k;
            if (args.size() != 1 || !parseCount(args[0], k)) { std::cout << "Usage: top <k>\n"; continue; }
            if (board.size() == 0) { std::cout << "Leaderboard is empty\n"; continue; }
            printRows(board.topK(k), 1);
        } else if (cmd == "around") {
            size_t radius = 5;
            if (args.empty() || args.size() > 2 || (args.size() == 2 && !parseCount(args[1], radius))) {
                std::cout << "Usage: around <name> [radius]\n";
                continue;
            }
            size_t r = board.getRank(args[0]);
            if (r == 0) { std::cout << "Unknown player " << args[0] << "\n"; continue; }
            size_t firstRank = (r > radius) ? r - radius : 1;
            printRows(board.neighbours(args[0], radius), firstRank);
        } else if (cmd == "at") {
            size_t rank;
            if (args.size() != 1 || !parseCount(args[0], rank)) { std::cout << "Usage: at <rank>\n"; continue; }
            Entry e;
            if (!board.getByRank(rank, e)) std::cout << "No player at rank " << rank << "\n";
            else printRows({e}, rank);
        } else if (cmd == "size") {
            std::cout << board.size() << " players\n";
        } else {
            std::cout << "Unknown command '" << cmd << "'. Type 'help'.\n";
        }
    }
    return 0;
}
