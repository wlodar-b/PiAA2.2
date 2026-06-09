#pragma once

#include "Board.hpp"
#include "GameRules.hpp"
#include "Types.hpp"

namespace ttt {

class Player {
public:
    virtual ~Player() = default;

    Cell mark() const { return mark_; }
    void setMark(Cell mark) { mark_ = mark; }

    virtual bool isHuman() const = 0;
    virtual Move chooseMove(const Board& board, const GameRules& rules) = 0;

protected:
    Cell mark_ = Cell::Empty;
};

} // namespace ttt
