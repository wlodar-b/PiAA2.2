#pragma once

namespace ttt {

enum class Cell { Empty, X, O };

enum class GameStatus { InProgress, XWins, OWins, Draw };

struct Move {
    int row = -1;
    int col = -1;
    bool isValid() const { return row >= 0 && col >= 0; }
};

struct GameConfig {
    int boardSize = 3;
    int winLength = 3;
};

inline Cell opponent(Cell c) {
    if (c == Cell::X) return Cell::O;
    if (c == Cell::O) return Cell::X;
    return Cell::Empty;
}

} // namespace ttt
