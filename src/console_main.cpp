/**
 * @file console_main.cpp
 * @brief Console-only Tic-Tac-Toe executable.
 */

#include "Game.hpp"
#include "AIPlayer.hpp"
#include "HumanPlayer.hpp"

#include <iostream>
#include <limits>
#include <memory>
#include <string>

static int readInt(const std::string& prompt, int minVal, int maxVal) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value && value >= minVal && value <= maxVal) {
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Invalid input. Please enter a number between "
                  << minVal << " and " << maxVal << ".\n";
    }
}

static void printBoard(const ttt::Board& board) {
    const int n = board.size();

    // Column header
    std::cout << "   ";
    for (int c = 0; c < n; ++c) {
        std::cout << " " << c << "  ";
    }
    std::cout << "\n";

    for (int r = 0; r < n; ++r) {
        std::cout << " " << r << " ";
        for (int c = 0; c < n; ++c) {
            ttt::Cell cell = board.at(r, c);
            char ch = '.';
            if (cell == ttt::Cell::X) ch = 'X';
            else if (cell == ttt::Cell::O) ch = 'O';

            std::cout << " " << ch << " ";
            if (c < n - 1) std::cout << "|";
        }
        std::cout << "\n";
        if (r < n - 1) {
            std::cout << "   ";
            for (int c = 0; c < n; ++c) {
                std::cout << "---";
                if (c < n - 1) std::cout << "+";
            }
            std::cout << "\n";
        }
    }
    std::cout << "\n";
}

int main() {
    std::cout << "=== Tic-Tac-Toe (Console) ===\n\n";

    int boardSize = readInt("Board size (3-15): ", 3, 15);
    int winLength = readInt("Win length (3-" + std::to_string(boardSize) + "): ",
                            3, boardSize);

    std::cout << "Play against AI? (1 = yes, 0 = no): ";
    int vsAI = readInt("", 0, 1);

    int difficulty = 4; // medium default
    if (vsAI) {
        difficulty = readInt("AI difficulty (1=easy, 4=medium, 9=hard): ", 1, 9);
    }

    ttt::GameConfig config{boardSize, winLength};
    ttt::Game game(config);

    std::unique_ptr<ttt::Player> playerX, playerO;
    playerX = std::make_unique<ttt::HumanPlayer>();
    playerX->setMark(ttt::Cell::X);

    if (vsAI) {
        auto ai = std::make_unique<ttt::AIPlayer>(difficulty);
        ai->setMark(ttt::Cell::O);
        playerO = std::move(ai);
    } else {
        playerO = std::make_unique<ttt::HumanPlayer>();
        playerO->setMark(ttt::Cell::O);
    }

    std::cout << "\nBoard: " << boardSize << "x" << boardSize
              << "  |  Win: " << winLength << " in a row\n\n";

    while (!game.isOver()) {
        printBoard(game.board());

        ttt::Cell current = game.currentPlayer();
        bool isX = (current == ttt::Cell::X);
        ttt::Player& player = isX ? *playerX : *playerO;

        if (player.isHuman()) {
            std::cout << (isX ? "X" : "O") << "'s turn.\n";
            while (true) {
                int row = readInt("  Row: ", 0, boardSize - 1);
                int col = readInt("  Col: ", 0, boardSize - 1);
                if (game.play(row, col)) break;
                std::cout << "  Invalid move. Try again.\n";
            }
        } else {
            std::cout << (isX ? "X" : "O") << " (AI) is thinking...\n";
            ttt::Move move = player.chooseMove(game.board(), game.rules());
            game.play(move.row, move.col);
            std::cout << "  AI plays: (" << move.row << ", " << move.col << ")\n\n";
        }
    }

    printBoard(game.board());

    switch (game.status()) {
        case ttt::GameStatus::XWins: std::cout << "X wins!\n"; break;
        case ttt::GameStatus::OWins: std::cout << "O wins!\n"; break;
        case ttt::GameStatus::Draw:  std::cout << "It's a draw!\n"; break;
        default: break;
    }

    return 0;
}
