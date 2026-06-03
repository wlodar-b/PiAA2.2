#include "AIPlayer.h"
#include "Board.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <random>
#include <vector>
#include <cmath>

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

static constexpr int SCORE_WIN  =  10000;
static constexpr int SCORE_LOSS = -10000;

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

AIPlayer::AIPlayer(char symbol, char opponentSymbol, Difficulty difficulty)
    : Player(symbol)
    , opponentSymbol_(opponentSymbol)
    , difficulty_(difficulty)
{
}

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

std::pair<int, int> AIPlayer::getMove(const Board& board)
{
    const auto moves = board.getAvailableMoves();
    if (moves.empty()) {
        return {-1, -1};  // Should never happen in a well-formed game.
    }

    // -----------------------------------------------------------------------
    // EASY: pick a random legal move.
    // -----------------------------------------------------------------------
    if (difficulty_ == Difficulty::EASY) {
        static std::mt19937 rng(
            static_cast<unsigned>(
                std::chrono::steady_clock::now().time_since_epoch().count()));
        std::uniform_int_distribution<int> dist(0,
            static_cast<int>(moves.size()) - 1);
        return moves[dist(rng)];
    }

    // -----------------------------------------------------------------------
    // MEDIUM / HARD: Minimax with alpha-beta pruning.
    // -----------------------------------------------------------------------
    const int maxDepth = getMaxDepth(board.getSize());

    // We need a mutable copy of the board for search.
    Board searchBoard = board;

    int bestScore = std::numeric_limits<int>::min();
    std::vector<std::pair<int, int>> bestMoves;  // Track ties for randomness.

    for (const auto& [r, c] : moves) {
        searchBoard.makeMove(r, c, symbol_);
        const int score = minimax(searchBoard, maxDepth - 1, false,
                                  std::numeric_limits<int>::min(),
                                  std::numeric_limits<int>::max());
        searchBoard.undoMove(r, c);

        if (score > bestScore) {
            bestScore = score;
            bestMoves.clear();
            bestMoves.emplace_back(r, c);
        } else if (score == bestScore) {
            bestMoves.emplace_back(r, c);
        }
    }

    // If multiple moves share the best score, pick one at random.
    if (bestMoves.size() == 1) {
        return bestMoves[0];
    }
    static std::mt19937 rng(
        static_cast<unsigned>(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<int> dist(0,
        static_cast<int>(bestMoves.size()) - 1);
    return bestMoves[dist(rng)];
}

bool AIPlayer::isHuman() const
{
    return false;
}

// ---------------------------------------------------------------------------
// Minimax with alpha-beta pruning
// ---------------------------------------------------------------------------

int AIPlayer::minimax(Board& board, int depth, bool isMaximizing,
                      int alpha, int beta)
{
    // Terminal checks: win, loss, draw, or depth limit.
    if (board.checkWin(symbol_)) {
        // Prefer faster wins (add depth bonus so shallower wins score higher).
        return SCORE_WIN + depth;
    }
    if (board.checkWin(opponentSymbol_)) {
        return SCORE_LOSS - depth;
    }
    if (board.isFull() || depth == 0) {
        return evaluate(board);
    }

    const auto moves = board.getAvailableMoves();

    if (isMaximizing) {
        int maxEval = std::numeric_limits<int>::min();
        for (const auto& [r, c] : moves) {
            board.makeMove(r, c, symbol_);
            const int eval = minimax(board, depth - 1, false, alpha, beta);
            board.undoMove(r, c);

            maxEval = std::max(maxEval, eval);
            alpha   = std::max(alpha, eval);
            if (beta <= alpha) break;  // Beta cutoff.
        }
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        for (const auto& [r, c] : moves) {
            board.makeMove(r, c, opponentSymbol_);
            const int eval = minimax(board, depth - 1, true, alpha, beta);
            board.undoMove(r, c);

            minEval = std::min(minEval, eval);
            beta    = std::min(beta, eval);
            if (beta <= alpha) break;  // Alpha cutoff.
        }
        return minEval;
    }
}

// ---------------------------------------------------------------------------
// Heuristic evaluation
// ---------------------------------------------------------------------------

int AIPlayer::evaluate(const Board& board) const
{
    // Quick check for terminal states.
    if (board.checkWin(symbol_))         return SCORE_WIN;
    if (board.checkWin(opponentSymbol_)) return SCORE_LOSS;

    const int size = board.getSize();
    const int winLen = board.getWinCondition();
    int score = 0;

    // -----------------------------------------------------------------------
    // 1. Center preference — positions near the center are strategically
    //    stronger.  The weight decreases with distance from the centre.
    // -----------------------------------------------------------------------
    const float center = static_cast<float>(size - 1) / 2.0f;
    for (int r = 0; r < size; ++r) {
        for (int c = 0; c < size; ++c) {
            const char cell = board.getCell(r, c);
            if (cell == '.') continue;

            // Manhattan distance from center, normalized to [0, 1].
            const float dist = (std::abs(r - center) + std::abs(c - center))
                             / static_cast<float>(size);
            const int bonus = static_cast<int>((1.0f - dist) * 3);

            if (cell == symbol_) {
                score += bonus;
            } else {
                score -= bonus;
            }
        }
    }

    // -----------------------------------------------------------------------
    // 2. Evaluate sliding windows of length winCondition along every line.
    //    Directions: horizontal, vertical, diagonal, anti-diagonal.
    // -----------------------------------------------------------------------
    auto evaluateWindows = [&](int startR, int startC, int dR, int dC,
                               int lineLength)
    {
        // Build the full line.
        std::vector<char> line;
        line.reserve(lineLength);
        for (int i = 0; i < lineLength; ++i) {
            line.push_back(board.getCell(startR + i * dR,
                                         startC + i * dC));
        }

        // Slide a window of size winLen across the line.
        for (int i = 0; i <= lineLength - winLen; ++i) {
            std::vector<char> window(line.begin() + i,
                                     line.begin() + i + winLen);
            score += scoreWindow(window);
        }
    };

    // Horizontal lines.
    for (int r = 0; r < size; ++r) {
        evaluateWindows(r, 0, 0, 1, size);
    }

    // Vertical lines.
    for (int c = 0; c < size; ++c) {
        evaluateWindows(0, c, 1, 0, size);
    }

    // Diagonals (top-left to bottom-right).
    // Main diagonal and those above it.
    for (int c = 0; c < size; ++c) {
        const int len = std::min(size, size - c);
        if (len >= winLen) {
            evaluateWindows(0, c, 1, 1, len);
        }
    }
    // Diagonals below the main diagonal.
    for (int r = 1; r < size; ++r) {
        const int len = std::min(size, size - r);
        if (len >= winLen) {
            evaluateWindows(r, 0, 1, 1, len);
        }
    }

    // Anti-diagonals (top-right to bottom-left).
    for (int c = 0; c < size; ++c) {
        const int len = std::min(c + 1, size);
        if (len >= winLen) {
            evaluateWindows(0, c, 1, -1, len);
        }
    }
    for (int r = 1; r < size; ++r) {
        const int len = std::min(size, size - r);
        if (len >= winLen) {
            evaluateWindows(r, size - 1, 1, -1, len);
        }
    }

    return score;
}

// ---------------------------------------------------------------------------
// Window scoring
// ---------------------------------------------------------------------------

int AIPlayer::scoreWindow(const std::vector<char>& window) const
{
    const int winLen = static_cast<int>(window.size());

    int aiCount  = 0;
    int oppCount = 0;
    int emptyCount = 0;

    for (const char c : window) {
        if (c == symbol_)         ++aiCount;
        else if (c == opponentSymbol_) ++oppCount;
        else                           ++emptyCount;
    }

    // If both players occupy the same window, it's contested — no score.
    if (aiCount > 0 && oppCount > 0) {
        return 0;
    }

    // -----------------------------------------------------------------------
    // Scoring weights — longer sequences receive exponentially more weight.
    // "Open-ended" windows (those with remaining empty cells) score higher
    // than blocked ones because they have more potential.
    // -----------------------------------------------------------------------
    int windowScore = 0;

    if (aiCount > 0) {
        // AI sequences.
        if (aiCount == winLen) {
            windowScore = SCORE_WIN;  // Complete winning line.
        } else if (aiCount == winLen - 1 && emptyCount == 1) {
            windowScore = 500;  // One move away from winning.
        } else if (aiCount == winLen - 2 && emptyCount == 2) {
            windowScore = 50;   // Two moves away — good potential.
        } else if (aiCount >= 1) {
            windowScore = aiCount * 5;  // General presence bonus.
        }
    } else if (oppCount > 0) {
        // Opponent sequences (mirror with negative weight).
        if (oppCount == winLen) {
            windowScore = SCORE_LOSS;
        } else if (oppCount == winLen - 1 && emptyCount == 1) {
            windowScore = -450;  // Slightly less weight than AI's threat
                                 // so the AI prefers winning over blocking.
        } else if (oppCount == winLen - 2 && emptyCount == 2) {
            windowScore = -40;
        } else if (oppCount >= 1) {
            windowScore = -(oppCount * 5);
        }
    }

    return windowScore;
}

// ---------------------------------------------------------------------------
// Depth configuration
// ---------------------------------------------------------------------------

int AIPlayer::getMaxDepth(int boardSize) const
{
    switch (difficulty_) {
        case Difficulty::EASY:
            return 0;  // Not used (random moves), but defined for safety.

        case Difficulty::MEDIUM:
            return 4;

        case Difficulty::HARD:
            // For small boards, search the entire tree for perfect play.
            if (boardSize <= 4) {
                return boardSize * boardSize;  // Effectively unlimited.
            }
            return 8;

        default:
            return 4;
    }
}
