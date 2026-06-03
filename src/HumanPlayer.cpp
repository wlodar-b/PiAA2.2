#include "HumanPlayer.h"
#include "Board.h"

#include <iostream>
#include <limits>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

HumanPlayer::HumanPlayer(char symbol)
    : Player(symbol)
{
}

// ---------------------------------------------------------------------------
// Move input
// ---------------------------------------------------------------------------

std::pair<int, int> HumanPlayer::getMove(const Board& board)
{
    int row = -1;
    int col = -1;

    while (true) {
        std::cout << "Player " << symbol_
                  << ", enter your move (row col): ";

        if (!(std::cin >> row >> col)) {
            // Non-numeric input — clear the error state and discard the line.
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter two numbers.\n";
            continue;
        }

        // Validate range.
        if (row < 0 || row >= board.getSize() ||
            col < 0 || col >= board.getSize()) {
            std::cout << "Coordinates out of range. "
                      << "Enter values between 0 and "
                      << (board.getSize() - 1) << ".\n";
            continue;
        }

        // Validate cell availability.
        if (board.getCell(row, col) != '.') {
            std::cout << "That cell is already occupied. Try again.\n";
            continue;
        }

        break;  // Valid move obtained.
    }

    return {row, col};
}

// ---------------------------------------------------------------------------
// Player type
// ---------------------------------------------------------------------------

bool HumanPlayer::isHuman() const
{
    return true;
}
