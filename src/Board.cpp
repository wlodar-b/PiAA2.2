#include "Board.h"

#include <stdexcept>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

Board::Board(int size, int winCondition)
    : size_(size)
    , winCondition_(winCondition)
    , grid_(size, std::vector<char>(size, '.'))
{
    if (size <= 0) {
        throw std::invalid_argument("Board size must be positive.");
    }
    if (winCondition <= 0 || winCondition > size) {
        throw std::invalid_argument(
            "Win condition must be between 1 and the board size.");
    }
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------

char Board::getCell(int row, int col) const
{
    if (row < 0 || row >= size_ || col < 0 || col >= size_) {
        throw std::out_of_range("Cell coordinates are out of range.");
    }
    return grid_[row][col];
}

int Board::getSize() const { return size_; }

int Board::getWinCondition() const { return winCondition_; }

// ---------------------------------------------------------------------------
// Move management
// ---------------------------------------------------------------------------

bool Board::makeMove(int row, int col, char symbol)
{
    // Validate bounds and check that the cell is empty.
    if (row < 0 || row >= size_ || col < 0 || col >= size_) {
        return false;
    }
    if (grid_[row][col] != '.') {
        return false;
    }

    grid_[row][col] = symbol;
    return true;
}

void Board::undoMove(int row, int col)
{
    if (row >= 0 && row < size_ && col >= 0 && col < size_) {
        grid_[row][col] = '.';
    }
}

// ---------------------------------------------------------------------------
// Board queries
// ---------------------------------------------------------------------------

bool Board::isFull() const
{
    for (int r = 0; r < size_; ++r) {
        for (int c = 0; c < size_; ++c) {
            if (grid_[r][c] == '.') {
                return false;
            }
        }
    }
    return true;
}

std::vector<std::pair<int, int>> Board::getAvailableMoves() const
{
    std::vector<std::pair<int, int>> moves;
    moves.reserve(size_ * size_);

    for (int r = 0; r < size_; ++r) {
        for (int c = 0; c < size_; ++c) {
            if (grid_[r][c] == '.') {
                moves.emplace_back(r, c);
            }
        }
    }
    return moves;
}

// ---------------------------------------------------------------------------
// Win detection
// ---------------------------------------------------------------------------

bool Board::checkWin(char symbol) const
{
    // Check every possible starting position and direction for a winning line.

    // Horizontal lines (left to right).
    for (int r = 0; r < size_; ++r) {
        for (int c = 0; c <= size_ - winCondition_; ++c) {
            if (checkLine(r, c, 0, 1, symbol)) return true;
        }
    }

    // Vertical lines (top to bottom).
    for (int r = 0; r <= size_ - winCondition_; ++r) {
        for (int c = 0; c < size_; ++c) {
            if (checkLine(r, c, 1, 0, symbol)) return true;
        }
    }

    // Diagonal lines (top-left to bottom-right).
    for (int r = 0; r <= size_ - winCondition_; ++r) {
        for (int c = 0; c <= size_ - winCondition_; ++c) {
            if (checkLine(r, c, 1, 1, symbol)) return true;
        }
    }

    // Anti-diagonal lines (top-right to bottom-left).
    for (int r = 0; r <= size_ - winCondition_; ++r) {
        for (int c = winCondition_ - 1; c < size_; ++c) {
            if (checkLine(r, c, 1, -1, symbol)) return true;
        }
    }

    return false;
}

std::vector<std::pair<int, int>> Board::getWinningLine(char symbol) const
{
    std::vector<std::pair<int, int>> line;

    // Same scanning order as checkWin — returns the first winning line found.

    // Horizontal.
    for (int r = 0; r < size_; ++r) {
        for (int c = 0; c <= size_ - winCondition_; ++c) {
            if (checkLine(r, c, 0, 1, symbol, &line)) return line;
        }
    }

    // Vertical.
    for (int r = 0; r <= size_ - winCondition_; ++r) {
        for (int c = 0; c < size_; ++c) {
            if (checkLine(r, c, 1, 0, symbol, &line)) return line;
        }
    }

    // Diagonal.
    for (int r = 0; r <= size_ - winCondition_; ++r) {
        for (int c = 0; c <= size_ - winCondition_; ++c) {
            if (checkLine(r, c, 1, 1, symbol, &line)) return line;
        }
    }

    // Anti-diagonal.
    for (int r = 0; r <= size_ - winCondition_; ++r) {
        for (int c = winCondition_ - 1; c < size_; ++c) {
            if (checkLine(r, c, 1, -1, symbol, &line)) return line;
        }
    }

    return {};  // No winning line found.
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

bool Board::checkLine(int startRow, int startCol, int dRow, int dCol,
                      char symbol, std::vector<std::pair<int, int>>* outLine) const
{
    std::vector<std::pair<int, int>> coords;
    coords.reserve(winCondition_);

    for (int i = 0; i < winCondition_; ++i) {
        const int r = startRow + i * dRow;
        const int c = startCol + i * dCol;

        if (grid_[r][c] != symbol) {
            return false;
        }
        coords.emplace_back(r, c);
    }

    // All cells matched — this is a winning line.
    if (outLine) {
        *outLine = std::move(coords);
    }
    return true;
}
