#include "Game.hpp"
#include "AIPlayer.hpp"
#include <iostream>

using namespace ttt;

int main() {
    GameConfig config{6, 3};
    Game game(config);
    AIPlayer ai(9);
    ai.setMark(Cell::O);
    game.play(0, 0);
    game.play(0, 1);
    game.play(1, 0);
    game.play(1, 1);
    game.play(2, 0);
    game.play(2, 1);
    game.play(3, 0);
    game.play(3, 1);
    game.play(4, 0);
    game.play(4, 1);
    game.play(5, 0);
    game.play(5, 1);
    try {
        ai.chooseMove(game.board(), game.rules());
        std::cout << "OK\n";
    } catch(std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
    }
}
