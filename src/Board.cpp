#include "Board.hpp"

namespace ttt {

Board::Board(int size)
    : size_(size)
    , cells_(static_cast<std::size_t>(size * size), Cell::Empty)
{}

Cell Board::at(int r, int c) const {
    return cells_[static_cast<std::size_t>(r * size_ + c)];
}

void Board::set(int r, int c, Cell value) {
    auto& cell = cells_[static_cast<std::size_t>(r * size_ + c)];
    if (cell == Cell::Empty && value != Cell::Empty) {
        ++filled_;
    } else if (cell != Cell::Empty && value == Cell::Empty) {
        --filled_;
    }
    cell = value;
}

bool Board::inBounds(int r, int c) const {
    return r >= 0 && r < size_ && c >= 0 && c < size_;
}

bool Board::isEmpty(int r, int c) const {
    return at(r, c) == Cell::Empty;
}

void Board::clear() {
    std::fill(cells_.begin(), cells_.end(), Cell::Empty);
    filled_ = 0;
}

} // namespace ttt
