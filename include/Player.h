#pragma once

#include <utility>

class Board;

/**
 * @brief Abstract base class for all player types (human, AI, etc.).
 *
 * Each player has a symbol ('X' or 'O') and must implement getMove()
 * to decide where to place their symbol on the board.
 */
class Player {
protected:
    char symbol_;   ///< The symbol this player places on the board.

public:
    /**
     * @brief Constructs a player with the given symbol.
     * @param s The symbol character (typically 'X' or 'O').
     */
    explicit Player(char s) : symbol_(s) {}

    virtual ~Player() = default;

    /** @brief Returns this player's symbol. */
    char getSymbol() const { return symbol_; }

    /**
     * @brief Determines the player's next move.
     * @param board The current board state.
     * @return A (row, col) pair indicating where to place the symbol.
     */
    virtual std::pair<int, int> getMove(const Board& board) = 0;

    /** @brief Returns true if this player is controlled by a human. */
    virtual bool isHuman() const = 0;
};
