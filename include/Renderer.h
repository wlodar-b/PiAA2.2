#pragma once

#include <string>
#include <utility>

class Board;

/**
 * @brief Abstract rendering interface for the Tic-Tac-Toe game.
 *
 * Concrete implementations include ConsoleRenderer (text mode) and
 * SFMLRenderer (graphical mode). The Game class uses this interface
 * to remain independent of the presentation layer.
 */
class Renderer {
public:
    virtual ~Renderer() = default;

    /** @brief Renders the current board state to the output. */
    virtual void render(const Board& board) = 0;

    /**
     * @brief Obtains a move from a human player via the renderer's input method.
     *
     * For a console renderer this reads from stdin; for a GUI renderer this
     * waits for a mouse click on the board.
     *
     * @param board The current board state for validation.
     * @return A valid (row, col) pair.
     */
    virtual std::pair<int, int> getHumanMove(const Board& board) = 0;

    /**
     * @brief Displays an end-of-game result message.
     * @param message The message to display (e.g. "X wins!" or "Draw!").
     */
    virtual void showResult(const std::string& message) = 0;

    /**
     * @brief Returns true if the renderer's window/session should be closed.
     *
     * For console renderers this always returns false. For GUI renderers
     * this returns true when the user closes the window.
     */
    virtual bool shouldClose() const = 0;
};
