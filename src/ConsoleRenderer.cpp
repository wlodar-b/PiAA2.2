#include "ConsoleRenderer.h"
#include "Board.h"

#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

// ---------------------------------------------------------------------------
// Board rendering
// ---------------------------------------------------------------------------

void ConsoleRenderer::render(const Board& board)
{
    const int size = board.getSize();

    std::cout << "\n";

    // Column header.
    std::cout << "    ";
    for (int c = 0; c < size; ++c) {
        std::cout << " " << c << "  ";
    }
    std::cout << "\n";

    // Top border.
    std::cout << "   +";
    for (int c = 0; c < size; ++c) {
        std::cout << "---+";
    }
    std::cout << "\n";

    // Rows.
    for (int r = 0; r < size; ++r) {
        std::cout << " " << r << " |";
        for (int c = 0; c < size; ++c) {
            const char cell = board.getCell(r, c);
            // Display '.' as a space for a cleaner look.
            const char display = (cell == '.') ? ' ' : cell;
            std::cout << " " << display << " |";
        }
        std::cout << "\n";

        // Row separator.
        std::cout << "   +";
        for (int c = 0; c < size; ++c) {
            std::cout << "---+";
        }
        std::cout << "\n";
    }

    std::cout << "\n";
}

// ---------------------------------------------------------------------------
// Human move input
// ---------------------------------------------------------------------------

std::pair<int, int> ConsoleRenderer::getHumanMove(const Board& board)
{
    int row = -1;
    int col = -1;

    while (true) {
        std::cout << "Enter your move (row col): ";

        if (!(std::cin >> row >> col)) {
            // Non-numeric input — clear the error and discard the line.
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter two numbers.\n";
            continue;
        }

        // Range check.
        if (row < 0 || row >= board.getSize() ||
            col < 0 || col >= board.getSize()) {
            std::cout << "Coordinates out of range (0.."
                      << (board.getSize() - 1) << "). Try again.\n";
            continue;
        }

        // Availability check.
        if (board.getCell(row, col) != '.') {
            std::cout << "That cell is already occupied. Try again.\n";
            continue;
        }

        break;
    }

    return {row, col};
}

// ---------------------------------------------------------------------------
// Result display
// ---------------------------------------------------------------------------

void ConsoleRenderer::showResult(const std::string& message)
{
    std::cout << "\n========================================\n"
              << "  " << message << "\n"
              << "========================================\n\n";
}

// ---------------------------------------------------------------------------
// Window state
// ---------------------------------------------------------------------------

bool ConsoleRenderer::shouldClose() const
{
    return false;  // Console mode never closes early.
}
