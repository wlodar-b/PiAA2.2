#pragma once

#include "Player.hpp"
#include <vector>

namespace ttt {

class AIPlayer : public Player {
public:
    explicit AIPlayer(int maxDepth = 6);

    bool isHuman() const override { return false; }
    Move chooseMove(const Board& board, const GameRules& rules) override;

private:
    int maxDepth_;

    static constexpr int kWinScore = 1'000'000;
    static constexpr int kInf      = 2'147'483'647; // numeric_limits<int>::max()

    int minimax(Board& board, const GameRules& rules,
                int depth, int alpha, int beta,
                Cell toMove, int depthLimit) const;

    int evaluate(const Board& board, const GameRules& rules) const;

    int windowValue(int count) const;

    std::vector<Move> candidateMoves(const Board& board) const;

    int effectiveDepth(const Board& board, int branching) const;

    Move openingMove(const Board& board) const;

    bool hasNeighbor(const Board& board, int r, int c) const;
};

} // namespace ttt
