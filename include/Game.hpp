#pragma once

#include "Board.hpp"
#include "GameRules.hpp"
#include "Types.hpp"
#include <vector>

namespace ttt {

class Game {
public:
    explicit Game(const GameConfig& config = {});

    const Board&     board() const { return board_; }
    const GameRules& rules() const { return rules_; }
    Cell             currentPlayer() const { return currentPlayer_; }
    GameStatus       status() const { return status_; }
    bool             isOver() const { return status_ != GameStatus::InProgress; }

    const std::vector<Move>& winningLine() const { return winningLine_; }

    /// Play a move at (row, col) for the current player.
    /// Returns true if the move was valid and applied.
    bool play(int row, int col);

    /// Reset the game with the same config.
    void reset();

    /// Reset the game with a new config.
    void reset(const GameConfig& config);

private:
    Board              board_;
    GameRules          rules_;
    Cell               currentPlayer_ = Cell::X;
    GameStatus         status_        = GameStatus::InProgress;
    std::vector<Move>  winningLine_;

    static GameConfig sanitize(const GameConfig& config);
};

} // namespace ttt
