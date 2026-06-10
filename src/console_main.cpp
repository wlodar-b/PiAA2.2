#include "AIPlayer.hpp"
#include "Game.hpp"
#include "Types.hpp"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>

using namespace ttt;

namespace {

char glyph(Cell c) {
    switch (c) {
        case Cell::X: return 'X';
        case Cell::O: return 'O';
        default: return '.';
    }
}

void printBoard(const Board& board) {
    const int n = board.size();
    std::cout << "\n    ";
    for (int c = 0; c < n; ++c) std::cout << c % 10 << ' ';
    std::cout << "\n   ";
    for (int c = 0; c < n; ++c) std::cout << "--";
    std::cout << '\n';
    for (int r = 0; r < n; ++r) {
        std::cout << (r % 10) << " | ";
        for (int c = 0; c < n; ++c) std::cout << glyph(board.at(r, c)) << ' ';
        std::cout << '\n';
    }
    std::cout << '\n';
}

int readInt(const std::string& prompt, int min, int max) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            if (value >= min && value <= max) return value;
        } else if (std::cin.eof()) {
            std::cout << "\nEnd of input.\n";
            std::exit(0);
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Invalid (expected " << min << ".." << max << ").\n";
    }
}

void printResult(GameStatus status) {
    switch (status) {
        case GameStatus::XWins: std::cout << "X wins!\n"; break;
        case GameStatus::OWins: std::cout << "O wins!\n"; break;
        case GameStatus::Draw:  std::cout << "Draw.\n"; break;
        default: break;
    }
}

} // namespace

int main() {
    std::cout << "=== Tic-Tac-Toe (Console) ===\n\n";

    const int size = readInt("Board size (3-15): ", 3, 15);
    const int winLen = readInt("Win length (3-" + std::to_string(size) + "): ", 3, size);
    const bool vsAi = readInt("Play against AI? (1 = yes, 0 = no): ", 0, 1) == 1;

    Game game(GameConfig{size, winLen});
    AIPlayer ai(9);
    ai.setMark(Cell::O);

    std::cout << "\nBoard: " << size << "x" << size
              << "  |  Win: " << winLen << " in a row\n";

    while (!game.isOver()) {
        printBoard(game.board());
        const Cell turn = game.currentPlayer();

        if (vsAi && turn == Cell::O) {
            std::cout << "O (AI) is thinking...\n";
            Move m = ai.chooseMove(game.board(), game.rules());
            game.play(m.row, m.col);
            std::cout << "  AI plays: (" << m.row << ", " << m.col << ")\n";
        } else {
            std::cout << glyph(turn) << "'s turn.\n";
            const int row = readInt("  Row: ", 0, size - 1);
            const int col = readInt("  Col: ", 0, size - 1);
            if (!game.play(row, col)) {
                std::cout << "  Invalid move. Try again.\n";
            }
        }
    }

    printBoard(game.board());
    printResult(game.status());
    return 0;
}
