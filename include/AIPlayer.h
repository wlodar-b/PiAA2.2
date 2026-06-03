#pragma once

#include "Player.h"
#include <utility>

class Board;

/**
 * @brief AI difficulty levels controlling search depth and strategy.
 */
enum class Difficulty {
    EASY,    ///< Random legal moves.
    MEDIUM,  ///< Minimax with depth limit 4.
    HARD     ///< Minimax with depth limit 8 (unlimited for boards <= 4x4).
};

/**
 * @brief An AI-controlled player using Minimax with alpha-beta pruning.
 *
 * The AI evaluates the board using a heuristic function that considers:
 *  - Immediate wins and losses (±10000).
 *  - Partial sequences of 2, 3, 4+ in a row with open/blocked weighting.
 *  - Center and near-center position preference.
 *
 * On EASY difficulty the AI simply picks a random legal move.
 * On MEDIUM it uses Minimax limited to depth 4.
 * On HARD it uses depth 8 (or full depth for small boards <= 4x4).
 */
class AIPlayer : public Player {
public:
    /**
     * @brief Constructs an AI player.
     * @param symbol         The AI's own symbol ('X' or 'O').
     * @param opponentSymbol The opponent's symbol.
     * @param difficulty     The difficulty level (EASY, MEDIUM, HARD).
     */
    AIPlayer(char symbol, char opponentSymbol, Difficulty difficulty);

    /**
     * @brief Computes and returns the best move for the current board state.
     * @param board The current board (passed by const ref; internally copied for search).
     * @return A (row, col) pair for the chosen move.
     */
    std::pair<int, int> getMove(const Board& board) override;

    /** @brief Returns false — this is an AI player. */
    bool isHuman() const override;

private:
    char opponentSymbol_;   ///< The opponent's symbol.
    Difficulty difficulty_; ///< Current difficulty setting.

    /**
     * @brief Minimax search with alpha-beta pruning.
     *
     * @param board        Mutable board used for making/undoing moves.
     * @param depth        Remaining search depth.
     * @param isMaximizing True if it's the AI's turn (maximizing), false for opponent.
     * @param alpha        Alpha bound for pruning.
     * @param beta         Beta bound for pruning.
     * @return The heuristic score of the board position.
     */
    int minimax(Board& board, int depth, bool isMaximizing, int alpha, int beta);

    /**
     * @brief Evaluates the board heuristically.
     *
     * Scores are computed by scanning all rows, columns, and diagonals
     * for partial sequences. A win returns +10000, a loss returns -10000.
     * Otherwise the score reflects the relative strength of each player's
     * position.
     *
     * @param board The board to evaluate.
     * @return A heuristic score (positive favours the AI).
     */
    int evaluate(const Board& board) const;

    /**
     * @brief Returns the maximum search depth for the current difficulty and board.
     * @param boardSize The board dimension.
     */
    int getMaxDepth(int boardSize) const;

    /**
     * @brief Scores a single line (row/col/diagonal window) for the heuristic.
     *
     * Counts how many of the AI's and opponent's symbols appear in the window,
     * and assigns a weighted score based on sequence length and openness.
     *
     * @param window A sequence of characters from the board.
     * @return A heuristic score contribution for this window.
     */
    int scoreWindow(const std::vector<char>& window) const;
};
