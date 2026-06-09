#include "GameRules.hpp"

namespace ttt {

GameRules::GameRules(int winLength)
    : winLength_(winLength)
{}

// ─── countInLine ────────────────────────────────────────────────────────

int GameRules::countInLine(const Board& board, int r, int c,
                           int dr, int dc, Cell mark) const {
    int count = 0;
    int nr = r + dr;
    int nc = c + dc;
    while (board.inBounds(nr, nc) && board.at(nr, nc) == mark) {
        ++count;
        nr += dr;
        nc += dc;
    }
    return count;
}

// ─── hasWon (full scan) ─────────────────────────────────────────────────

bool GameRules::hasWon(const Board& board, Cell mark) const {
    const int n = board.size();
    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            if (board.at(r, c) == mark && hasWonAt(board, r, c)) {
                return true;
            }
        }
    }
    return false;
}

// ─── hasWonAt (optimized for last move) ─────────────────────────────────

bool GameRules::hasWonAt(const Board& board, int lastRow, int lastCol) const {
    Cell mark = board.at(lastRow, lastCol);
    if (mark == Cell::Empty) return false;

    for (const auto& dir : kDirections) {
        int total = 1 + countInLine(board, lastRow, lastCol, dir[0], dir[1], mark)
                      + countInLine(board, lastRow, lastCol, -dir[0], -dir[1], mark);
        if (total >= winLength_) return true;
    }
    return false;
}

// ─── winningLine ────────────────────────────────────────────────────────

std::vector<Move> GameRules::winningLine(const Board& board, int row, int col) const {
    Cell mark = board.at(row, col);
    if (mark == Cell::Empty) return {};

    for (const auto& dir : kDirections) {
        int fwd = countInLine(board, row, col, dir[0], dir[1], mark);
        int bwd = countInLine(board, row, col, -dir[0], -dir[1], mark);
        int total = 1 + fwd + bwd;

        if (total >= winLength_) {
            std::vector<Move> line;
            // Start from the backward-most cell
            int sr = row - bwd * dir[0];
            int sc = col - bwd * dir[1];
            for (int i = 0; i < total; ++i) {
                line.push_back({sr + i * dir[0], sc + i * dir[1]});
            }
            return line;
        }
    }
    return {};
}

// ─── status ─────────────────────────────────────────────────────────────

GameStatus GameRules::status(const Board& board, Move lastMove) const {
    if (lastMove.isValid()) {
        // Optimized path: only check lines through the last move.
        Cell mark = board.at(lastMove.row, lastMove.col);
        if (mark != Cell::Empty && hasWonAt(board, lastMove.row, lastMove.col)) {
            return mark == Cell::X ? GameStatus::XWins : GameStatus::OWins;
        }
    } else {
        // Full scan fallback.
        if (hasWon(board, Cell::X)) return GameStatus::XWins;
        if (hasWon(board, Cell::O)) return GameStatus::OWins;
    }

    if (board.isFull()) return GameStatus::Draw;
    return GameStatus::InProgress;
}

} // namespace ttt
