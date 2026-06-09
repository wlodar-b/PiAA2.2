#pragma once

#include "Board.hpp"
#include "Types.hpp"
#include <vector>

namespace ttt {

class GameRules {
public:
    explicit GameRules(int winLength);

    int winLength() const { return winLength_; }

    /// Full-board scan: has `mark` formed a winning line anywhere?
    bool hasWon(const Board& board, Cell mark) const;

    /// Optimized: only check lines passing through (lastRow, lastCol).
    bool hasWonAt(const Board& board, int lastRow, int lastCol) const;

    /// Return the winning line cells through (row, col), or empty if none.
    std::vector<Move> winningLine(const Board& board, int row, int col) const;

    /// Determine overall game status.  If lastMove is valid, uses hasWonAt
    /// for efficiency; otherwise falls back to full scan.
    GameStatus status(const Board& board, Move lastMove = {}) const;

private:
    int winLength_;

    static constexpr int kDirections[4][2] = {
        {0, 1}, {1, 0}, {1, 1}, {1, -1}
    };

    /// Count consecutive `mark` cells starting from (r,c) stepping by (dr,dc),
    /// NOT including (r,c) itself.
    int countInLine(const Board& board, int r, int c,
                    int dr, int dc, Cell mark) const;
};

} // namespace ttt
