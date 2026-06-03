#pragma once

#include <string>

class Board;
class Player;
class Renderer;

/**
 * @brief Possible states of the game.
 */
enum class GameState {
    PLAYING,  ///< Game is still in progress.
    X_WON,   ///< Player X has won.
    O_WON,   ///< Player O has won.
    DRAW     ///< The board is full with no winner.
};

/**
 * @brief Main game controller that orchestrates the game loop.
 *
 * The Game object ties together a Board, two Players, and a Renderer.
 * It manages turn alternation, win/draw detection, and delegates
 * rendering and input to the Renderer.
 */
class Game {
public:
    /**
     * @brief Constructs the game with all dependencies.
     * @param board    Reference to the game board.
     * @param p1       Reference to player 1 (plays first).
     * @param p2       Reference to player 2.
     * @param renderer Reference to the renderer for display and input.
     */
    Game(Board& board, Player& p1, Player& p2, Renderer& renderer);

    /**
     * @brief Runs the main game loop until the game ends or the window closes.
     *
     * Each iteration:
     *  1. Renders the board.
     *  2. Gets a move from the current player.
     *  3. Places the move on the board.
     *  4. Checks for win or draw.
     *  5. Switches to the other player.
     */
    void run();

    /** @brief Returns the symbol of the player whose turn it is. */
    char getCurrentPlayerSymbol() const;

    /** @brief Returns the current game state. */
    GameState getState() const;

    /**
     * @brief Resets the game to its initial state.
     *
     * Clears the board, sets the state back to PLAYING, and makes
     * player 1 the current player.
     */
    void reset();

private:
    Board& board_;          ///< The game board.
    Player& player1_;       ///< Player 1 (goes first).
    Player& player2_;       ///< Player 2.
    Renderer& renderer_;    ///< The renderer for display and input.
    Player* currentPlayer_; ///< Pointer to the player whose turn it is.
    GameState state_;       ///< Current game state.

    /** @brief Switches currentPlayer_ to the other player. */
    void switchPlayer();

    /**
     * @brief Updates the game state after a move.
     *
     * Checks if the current player has won or if the board is full (draw).
     */
    void updateState();

    /**
     * @brief Builds a human-readable result string for the current state.
     * @return A message like "Player X wins!" or "It's a draw!".
     */
    std::string getResultMessage() const;
};
