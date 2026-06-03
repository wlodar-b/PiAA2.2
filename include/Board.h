#pragma once

#include <vector>
#include <utility>

/**
 * @brief Represents an NxN Tic-Tac-Toe board with a configurable win condition.
 *
 * The board uses '.' to represent empty cells, and player symbols (typically
 * 'X' and 'O') for occupied cells. The win condition specifies how many
 * consecutive symbols in a row/column/diagonal are needed to win.
 */
class Board {
public:
    /**
     * @brief Constructs a board of the given size with the specified win condition.
     * @param size      The number of rows and columns (NxN).
     * @param winCondition  Number of consecutive symbols required to win.
     */
    Board(int size, int winCondition);

    /** @brief Returns the symbol at the given cell, or '.' if empty. */
    char getCell(int row, int col) const;

    /**
     * @brief Places a symbol on the board at (row, col).
     * @return true if the move was valid and placed, false otherwise.
     */
    bool makeMove(int row, int col, char symbol);

    /** @brief Removes a symbol from the board at (row, col), resetting it to '.'. */
    void undoMove(int row, int col);

    /**
     * @brief Checks whether the given symbol has achieved a winning line.
     * Scans all rows, columns, and diagonals for winCondition consecutive symbols.
     */
    bool checkWin(char symbol) const;

    /** @brief Returns true if every cell on the board is occupied. */
    bool isFull() const;

    /** @brief Returns the board dimension (N for an NxN board). */
    int getSize() const;

    /** @brief Returns the number of consecutive symbols required to win. */
    int getWinCondition() const;

    /**
     * @brief Returns a list of all empty cell coordinates.
     * @return Vector of (row, col) pairs for every cell containing '.'.
     */
    std::vector<std::pair<int, int>> getAvailableMoves() const;

    /**
     * @brief Returns the coordinates of the winning line for the given symbol.
     *
     * If the symbol has won, returns the coordinates of the first winning line
     * found. If no winning line exists, returns an empty vector.
     */
    std::vector<std::pair<int, int>> getWinningLine(char symbol) const;

private:
    int size_;                                ///< Board dimension (N).
    int winCondition_;                        ///< Consecutive symbols needed to win.
    std::vector<std::vector<char>> grid_;     ///< 2D grid of cells.

    /**
     * @brief Helper: checks for winCondition consecutive symbols along a direction.
     *
     * Scans from (startRow, startCol) in the direction (dRow, dCol) and stores
     * matching coordinates in outLine.
     *
     * @return true if a winning line of the required length was found.
     */
    bool checkLine(int startRow, int startCol, int dRow, int dCol,
                   char symbol, std::vector<std::pair<int, int>>* outLine = nullptr) const;
};
