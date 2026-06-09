#pragma once

#include "Player.hpp"

namespace ttt {

class HumanPlayer : public Player {
public:
    bool isHuman() const override { return true; }
    Move chooseMove(const Board&, const GameRules&) override { return {}; }
};

} // namespace ttt
