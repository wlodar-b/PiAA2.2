#pragma once

#include "Types.hpp"
#include <vector>

namespace ttt {

class Board {
public:
    explicit Board(int size = 3);

    int  size() const { return size_; }

    Cell at(int r, int c) const;
    void set(int r, int c, Cell value);

    bool inBounds(int r, int c) const;
    bool isEmpty(int r, int c) const;
    bool isFull() const { return filled_ == size_ * size_; }
    int  filledCount() const { return filled_; }

    void clear();

private:
    int              size_;
    int              filled_ = 0;
    std::vector<Cell> cells_;
};

} // namespace ttt
