#include "AIPlayer.hpp"

#include <algorithm>
#include <random>

namespace ttt {

// ═════════════════════════════════════════════════════════════════════════════
// Construction
// ═════════════════════════════════════════════════════════════════════════════

AIPlayer::AIPlayer(int maxDepth)
    : maxDepth_(maxDepth)
{}

// ═════════════════════════════════════════════════════════════════════════════
// chooseMove — root search
// ═════════════════════════════════════════════════════════════════════════════

Move AIPlayer::chooseMove(const Board& board, const GameRules& rules) {
    // Empty board → opening move
    if (board.filledCount() == 0) {
        return openingMove(board);
    }

    auto candidates = candidateMoves(board);
    if (candidates.empty()) return {};

    const int depthLim = effectiveDepth(board, static_cast<int>(candidates.size()));

    int bestScore = -kInf;
    std::vector<Move> bestMoves;

    // We need a mutable copy for the search.
    Board scratch = board;

    for (const auto& move : candidates) {
        scratch.set(move.row, move.col, mark_);

        int score;

        // Check immediate win
        if (rules.hasWonAt(scratch, move.row, move.col)) {
            score = kWinScore;
        } else if (scratch.isFull()) {
            score = 0;
        } else {
            // IMPORTANT: use full window (-kInf, kInf) for EACH root move
            // so that all equally-good moves are identified for randomization.
            score = minimax(scratch, rules, 1, -kInf, kInf,
                            opponent(mark_), depthLim);
        }

        scratch.set(move.row, move.col, Cell::Empty);

        if (score > bestScore) {
            bestScore = score;
            bestMoves.clear();
            bestMoves.push_back(move);
        } else if (score == bestScore) {
            bestMoves.push_back(move);
        }
    }

    // Pick randomly among equally-good moves.
    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(0, static_cast<int>(bestMoves.size()) - 1);
    return bestMoves[static_cast<std::size_t>(dist(rng))];
}

// ═════════════════════════════════════════════════════════════════════════════
// minimax with alpha-beta pruning
// ═════════════════════════════════════════════════════════════════════════════

int AIPlayer::minimax(Board& board, const GameRules& rules,
                      int depth, int alpha, int beta,
                      Cell toMove, int depthLimit) const {
    if (board.isFull()) return 0;
    if (depth >= depthLimit) return evaluate(board, rules);

    const bool maximizing = (toMove == mark_);
    auto candidates = candidateMoves(board);

    if (maximizing) {
        int best = -kInf;
        for (const auto& move : candidates) {
            board.set(move.row, move.col, toMove);

            int score;
            if (rules.hasWonAt(board, move.row, move.col)) {
                score = kWinScore - depth;
            } else {
                score = minimax(board, rules, depth + 1, alpha, beta,
                                opponent(toMove), depthLimit);
            }

            board.set(move.row, move.col, Cell::Empty);

            best = std::max(best, score);
            alpha = std::max(alpha, best);
            if (alpha >= beta) break;
        }
        return best;
    } else {
        int best = kInf;
        for (const auto& move : candidates) {
            board.set(move.row, move.col, toMove);

            int score;
            if (rules.hasWonAt(board, move.row, move.col)) {
                score = -(kWinScore - depth);
            } else {
                score = minimax(board, rules, depth + 1, alpha, beta,
                                opponent(toMove), depthLimit);
            }

            board.set(move.row, move.col, Cell::Empty);

            best = std::min(best, score);
            beta = std::min(beta, best);
            if (alpha >= beta) break;
        }
        return best;
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// evaluate — heuristic board evaluation
// ═════════════════════════════════════════════════════════════════════════════

int AIPlayer::evaluate(const Board& board, const GameRules& rules) const {
    const int n = board.size();
    const int wl = rules.winLength();
    const Cell mine   = mark_;
    const Cell theirs = opponent(mark_);

    int score = 0;

    // Scan all lines in all 4 directions.
    // For directions (0,1), (1,0), (1,1), (1,-1), we iterate over
    // starting positions and slide a window of size winLength.

    // Direction (0,1): horizontal — each row
    for (int r = 0; r < n; ++r) {
        for (int c = 0; c <= n - wl; ++c) {
            int myCount = 0, theirCount = 0;
            for (int k = 0; k < wl; ++k) {
                Cell cell = board.at(r, c + k);
                if (cell == mine) ++myCount;
                else if (cell == theirs) ++theirCount;
            }
            if (myCount > 0 && theirCount == 0)
                score += windowValue(myCount);
            else if (theirCount > 0 && myCount == 0)
                score -= windowValue(theirCount);
        }
    }

    // Direction (1,0): vertical — each column
    for (int c = 0; c < n; ++c) {
        for (int r = 0; r <= n - wl; ++r) {
            int myCount = 0, theirCount = 0;
            for (int k = 0; k < wl; ++k) {
                Cell cell = board.at(r + k, c);
                if (cell == mine) ++myCount;
                else if (cell == theirs) ++theirCount;
            }
            if (myCount > 0 && theirCount == 0)
                score += windowValue(myCount);
            else if (theirCount > 0 && myCount == 0)
                score -= windowValue(theirCount);
        }
    }

    // Direction (1,1): diagonal ↘
    for (int r = 0; r <= n - wl; ++r) {
        for (int c = 0; c <= n - wl; ++c) {
            int myCount = 0, theirCount = 0;
            for (int k = 0; k < wl; ++k) {
                Cell cell = board.at(r + k, c + k);
                if (cell == mine) ++myCount;
                else if (cell == theirs) ++theirCount;
            }
            if (myCount > 0 && theirCount == 0)
                score += windowValue(myCount);
            else if (theirCount > 0 && myCount == 0)
                score -= windowValue(theirCount);
        }
    }

    // Direction (1,-1): anti-diagonal ↙
    for (int r = 0; r <= n - wl; ++r) {
        for (int c = wl - 1; c < n; ++c) {
            int myCount = 0, theirCount = 0;
            for (int k = 0; k < wl; ++k) {
                Cell cell = board.at(r + k, c - k);
                if (cell == mine) ++myCount;
                else if (cell == theirs) ++theirCount;
            }
            if (myCount > 0 && theirCount == 0)
                score += windowValue(myCount);
            else if (theirCount > 0 && myCount == 0)
                score -= windowValue(theirCount);
        }
    }

    return score;
}

int AIPlayer::windowValue(int count) const {
    if (count <= 0) return 0;
    int value = 1;
    for (int i = 1; i < count; ++i) {
        value *= 10;
        if (value >= 100000) return 100000;
    }
    return value;
}

// ═════════════════════════════════════════════════════════════════════════════
// candidateMoves
// ═════════════════════════════════════════════════════════════════════════════

std::vector<Move> AIPlayer::candidateMoves(const Board& board) const {
    const int n = board.size();
    const int empties = n * n - board.filledCount();

    // Empty board → center only
    if (empties == n * n) {
        return {{n / 2, n / 2}};
    }

    // Endgame (≤9 empties) → return ALL empty cells for exhaustive search
    if (empties <= 9) {
        std::vector<Move> moves;
        moves.reserve(static_cast<std::size_t>(empties));
        for (int r = 0; r < n; ++r) {
            for (int c = 0; c < n; ++c) {
                if (board.isEmpty(r, c)) {
                    moves.push_back({r, c});
                }
            }
        }
        return moves;
    }

    // Otherwise → only cells adjacent to existing marks
    std::vector<Move> moves;
    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            if (board.isEmpty(r, c) && hasNeighbor(board, r, c)) {
                moves.push_back({r, c});
            }
        }
    }
    return moves;
}

bool AIPlayer::hasNeighbor(const Board& board, int r, int c) const {
    static constexpr int kNeighborDirs[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        { 0, -1},          { 0, 1},
        { 1, -1}, { 1, 0}, { 1, 1}
    };
    for (const auto& d : kNeighborDirs) {
        int nr = r + d[0];
        int nc = c + d[1];
        if (board.inBounds(nr, nc) && !board.isEmpty(nr, nc)) {
            return true;
        }
    }
    return false;
}

// ═════════════════════════════════════════════════════════════════════════════
// effectiveDepth
// ═════════════════════════════════════════════════════════════════════════════

int AIPlayer::effectiveDepth(const Board& board, int branching) const {
    const int n = board.size();
    const int empties = n * n - board.filledCount();

    // Endgame: search as deep as possible
    if (empties <= 9) {
        return std::min(empties, maxDepth_);
    }

    int cap;
    if (branching <= 6)       cap = 6;
    else if (branching <= 12) cap = 4;
    else if (branching <= 24) cap = 3;
    else                      cap = 2;

    return std::min(maxDepth_, cap);
}

// ═════════════════════════════════════════════════════════════════════════════
// openingMove
// ═════════════════════════════════════════════════════════════════════════════

Move AIPlayer::openingMove(const Board& board) const {
    const int n = board.size();
    const int center = n / 2;

    static thread_local std::mt19937 rng{std::random_device{}()};

    // Hard difficulty → always center
    if (maxDepth_ >= 7) {
        return {center, center};
    }

    // Easy difficulty → random any cell
    if (maxDepth_ <= 1) {
        std::uniform_int_distribution<int> dist(0, n * n - 1);
        int idx = dist(rng);
        return {idx / n, idx % n};
    }

    // Medium difficulty → random from center + 8 neighbors
    std::vector<Move> options;
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            int r = center + dr;
            int c = center + dc;
            if (board.inBounds(r, c)) {
                options.push_back({r, c});
            }
        }
    }

    std::uniform_int_distribution<int> dist(0, static_cast<int>(options.size()) - 1);
    return options[static_cast<std::size_t>(dist(rng))];
}

} // namespace ttt
