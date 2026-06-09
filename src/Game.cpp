#include "Game.hpp"

#include <algorithm>

namespace ttt {

// ─── sanitize ───────────────────────────────────────────────────────────

GameConfig Game::sanitize(const GameConfig& config) {
    GameConfig c = config;
    c.boardSize = std::max(c.boardSize, 3);
    c.winLength = std::clamp(c.winLength, 3, c.boardSize);
    return c;
}

// ─── Construction ───────────────────────────────────────────────────────

Game::Game(const GameConfig& config)
    : board_(sanitize(config).boardSize)
    , rules_(sanitize(config).winLength)
{}

// ─── play ───────────────────────────────────────────────────────────────

bool Game::play(int row, int col) {
    if (isOver()) return false;
    if (!board_.inBounds(row, col)) return false;
    if (!board_.isEmpty(row, col)) return false;

    board_.set(row, col, currentPlayer_);

    Move lastMove{row, col};
    status_ = rules_.status(board_, lastMove);

    if (status_ == GameStatus::XWins || status_ == GameStatus::OWins) {
        winningLine_ = rules_.winningLine(board_, row, col);
    }

    if (status_ == GameStatus::InProgress) {
        currentPlayer_ = opponent(currentPlayer_);
    }

    return true;
}

// ─── reset ──────────────────────────────────────────────────────────────

void Game::reset() {
    board_.clear();
    currentPlayer_ = Cell::X;
    status_        = GameStatus::InProgress;
    winningLine_.clear();
}

void Game::reset(const GameConfig& config) {
    auto c = sanitize(config);
    board_ = Board(c.boardSize);
    rules_ = GameRules(c.winLength);
    currentPlayer_ = Cell::X;
    status_        = GameStatus::InProgress;
    winningLine_.clear();
}

} // namespace ttt
