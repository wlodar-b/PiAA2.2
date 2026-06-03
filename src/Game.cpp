#include "Game.h"
#include "Board.h"
#include "Player.h"
#include "Renderer.h"

#include <iostream>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

Game::Game(Board& board, Player& p1, Player& p2, Renderer& renderer)
    : board_(board)
    , player1_(p1)
    , player2_(p2)
    , renderer_(renderer)
    , currentPlayer_(&p1)
    , state_(GameState::PLAYING)
{
}

// ---------------------------------------------------------------------------
// Main game loop
// ---------------------------------------------------------------------------

void Game::run()
{
    while (state_ == GameState::PLAYING && !renderer_.shouldClose()) {
        // 1. Render the current board.
        renderer_.render(board_);

        // 2. Obtain the current player's move.
        std::pair<int, int> move;

        if (currentPlayer_->isHuman()) {
            // For human players, delegate input to the renderer so that
            // console and GUI modes can each handle input their own way.
            move = renderer_.getHumanMove(board_);
        } else {
            // AI players compute their move internally.
            std::cout << "Player " << currentPlayer_->getSymbol()
                      << " (AI) is thinking...\n";
            move = currentPlayer_->getMove(board_);
            std::cout << "Player " << currentPlayer_->getSymbol()
                      << " plays: " << move.first << " " << move.second
                      << "\n";
        }

        // 3. Place the move on the board.
        if (!board_.makeMove(move.first, move.second,
                             currentPlayer_->getSymbol())) {
            // This should never happen with correct input handling,
            // but guard against it.
            std::cerr << "Error: invalid move attempted ("
                      << move.first << ", " << move.second << ").\n";
            continue;
        }

        // 4. Check for win or draw.
        updateState();

        if (state_ != GameState::PLAYING) {
            // Show the final board and the result.
            renderer_.render(board_);
            renderer_.showResult(getResultMessage());
            break;
        }

        // 5. Switch to the other player.
        switchPlayer();
    }
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------

char Game::getCurrentPlayerSymbol() const
{
    return currentPlayer_->getSymbol();
}

GameState Game::getState() const
{
    return state_;
}

// ---------------------------------------------------------------------------
// Reset
// ---------------------------------------------------------------------------

void Game::reset()
{
    // Rebuild the board grid by recreating with the same dimensions.
    const int size = board_.getSize();
    for (int r = 0; r < size; ++r) {
        for (int c = 0; c < size; ++c) {
            board_.undoMove(r, c);
        }
    }

    currentPlayer_ = &player1_;
    state_ = GameState::PLAYING;
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void Game::switchPlayer()
{
    currentPlayer_ = (currentPlayer_ == &player1_)
                   ? &player2_
                   : &player1_;
}

void Game::updateState()
{
    const char sym = currentPlayer_->getSymbol();

    if (board_.checkWin(sym)) {
        state_ = (sym == 'X') ? GameState::X_WON : GameState::O_WON;
    } else if (board_.isFull()) {
        state_ = GameState::DRAW;
    }
    // Otherwise, state_ remains PLAYING.
}

std::string Game::getResultMessage() const
{
    switch (state_) {
        case GameState::X_WON:  return "Player X wins!";
        case GameState::O_WON:  return "Player O wins!";
        case GameState::DRAW:   return "It's a draw!";
        default:                return "Game is still in progress.";
    }
}
