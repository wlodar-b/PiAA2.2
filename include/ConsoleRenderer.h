#pragma once

#include "Renderer.h"

/**
 * @brief Text-based renderer that outputs the board to the console.
 *
 * Draws the board with row/column numbers, grid lines (| and ---),
 * and reads human moves from std::cin.
 */
class ConsoleRenderer : public Renderer {
public:
    ConsoleRenderer() = default;

    /** @brief Prints the board to stdout with nice formatting. */
    void render(const Board& board) override;

    /**
     * @brief Reads a (row, col) move from std::cin with input validation.
     *
     * Re-prompts the user on invalid input (out-of-range, already occupied,
     * or non-numeric input).
     *
     * @param board The current board state for validation.
     * @return A valid (row, col) pair.
     */
    std::pair<int, int> getHumanMove(const Board& board) override;

    /** @brief Prints the result message to stdout. */
    void showResult(const std::string& message) override;

    /** @brief Always returns false — the console never closes early. */
    bool shouldClose() const override;
};
