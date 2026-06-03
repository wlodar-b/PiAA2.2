#pragma once

#include "Player.h"

/**
 * @brief A human-controlled player that reads moves from standard input.
 *
 * This player is intended for console (text) mode only. In GUI mode,
 * moves are typically obtained through the Renderer instead.
 */
class HumanPlayer : public Player {
public:
    /** @brief Constructs a human player with the given symbol. */
    explicit HumanPlayer(char symbol);

    /**
     * @brief Reads a move (row, col) from std::cin.
     *
     * Prompts the user and validates the input. If the entered position
     * is invalid or already occupied, the user is asked again.
     *
     * @param board The current board state used for validation.
     * @return A valid (row, col) pair.
     */
    std::pair<int, int> getMove(const Board& board) override;

    /** @brief Returns true — this is a human player. */
    bool isHuman() const override;
};
