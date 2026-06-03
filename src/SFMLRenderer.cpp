/**
 * @file SFMLRenderer.cpp
 * @brief Implementation of the SFML 3.0 graphical renderer for Tic-Tac-Toe.
 *
 * All drawing uses SFML 3.0.2 API conventions (Vector2u video mode, optional
 * events, getIf<> pattern, font constructor, etc.).
 */

#include "SFMLRenderer.h"
#include "Board.h"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <string>

// ═══════════════════════════════════════════════════════════════════════════
// Colour palette (dark theme)
// ═══════════════════════════════════════════════════════════════════════════

const sf::Color SFMLRenderer::kBackground{0x1a, 0x1a, 0x2e};
const sf::Color SFMLRenderer::kGridLine  {0x16, 0x21, 0x3e};
const sf::Color SFMLRenderer::kXColor    {0xe9, 0x45, 0x60};
const sf::Color SFMLRenderer::kOColor    {0x4d, 0xa6, 0xff};
const sf::Color SFMLRenderer::kHover     {255, 255, 255, 13};
const sf::Color SFMLRenderer::kWinGlow   {0xff, 0xd7, 0x00};
const sf::Color SFMLRenderer::kText      {0xea, 0xea, 0xea};
const sf::Color SFMLRenderer::kPanelBg   {0x16, 0x21, 0x3e};

// ═══════════════════════════════════════════════════════════════════════════
// Construction
// ═══════════════════════════════════════════════════════════════════════════

SFMLRenderer::SFMLRenderer(int boardSize, sf::Font& font)
    : window_(sf::VideoMode(sf::Vector2u{WINDOW_W, WINDOW_H}),
              "Tic-Tac-Toe",
              sf::Style::Default,
              sf::State::Windowed)
    , font_(font)
    , boardSize_(boardSize)
{
    window_.setFramerateLimit(60);
    recalculateLayout();
}

// ═══════════════════════════════════════════════════════════════════════════
// Layout calculation
// ═══════════════════════════════════════════════════════════════════════════

void SFMLRenderer::recalculateLayout() {
    // The board is a square that fits in the left ~65 % of the window,
    // with some padding.  The info panel takes the remaining right side.
    constexpr float kPad = 30.f;

    const float availW = WINDOW_W * 0.62f - 2.f * kPad;
    const float availH = WINDOW_H        - 2.f * kPad;
    boardPixelSize_    = std::min(availW, availH);
    cellSize_          = boardPixelSize_ / static_cast<float>(boardSize_);

    // Centre the board vertically in the left region.
    boardOriginX_ = kPad;
    boardOriginY_ = (WINDOW_H - boardPixelSize_) / 2.f;

    // Panel occupies the remaining space.
    panelX_ = boardOriginX_ + boardPixelSize_ + kPad;
    panelW_ = WINDOW_W - panelX_ - kPad;
}

// ═══════════════════════════════════════════════════════════════════════════
// Renderer interface: render
// ═══════════════════════════════════════════════════════════════════════════

void SFMLRenderer::render(const Board& board) {
    // Auto-detect whose turn it is from the board state.
    inferCurrentPlayer(board);

    window_.clear(kBackground);
    drawBoard(board);
    drawInfoPanel(board);

    if (!resultMessage_.empty()) {
        drawResultOverlay();
    }

    window_.display();
}

// ═══════════════════════════════════════════════════════════════════════════
// Renderer interface: getHumanMove
// ═══════════════════════════════════════════════════════════════════════════

std::pair<int, int> SFMLRenderer::getHumanMove(const Board& board) {
    inferCurrentPlayer(board);

    while (window_.isOpen()) {
        // ── Process events ──────────────────────────────────────────
        while (const auto event = window_.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window_.close();
                windowClosed_ = true;
                return {-1, -1};
            }

            // Mouse click — check for cell selection.
            if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                // Check "New Game" button first.
                if (isInsideNewGameButton(mb->position)) {
                    newGameRequested_ = true;
                    return {-1, -1};
                }

                auto [row, col] = pixelToCell(mb->position);
                if (row >= 0 && col >= 0 && board.getCell(row, col) == '.') {
                    return {row, col};
                }
            }

            // Mouse move — update hover cell.
            if (const auto* mm = event->getIf<sf::Event::MouseMoved>()) {
                auto [r, c] = pixelToCell(mm->position);
                hoverRow_ = r;
                hoverCol_ = c;
            }
        }

        // ── Redraw with hover preview ───────────────────────────────
        window_.clear(kBackground);
        drawBoard(board);
        drawInfoPanel(board);
        window_.display();
    }

    windowClosed_ = true;
    return {-1, -1};
}

// ═══════════════════════════════════════════════════════════════════════════
// Renderer interface: showResult
// ═══════════════════════════════════════════════════════════════════════════

void SFMLRenderer::showResult(const std::string& message) {
    resultMessage_ = message;
}

// ═══════════════════════════════════════════════════════════════════════════
// Renderer interface: shouldClose
// ═══════════════════════════════════════════════════════════════════════════

bool SFMLRenderer::shouldClose() const {
    return windowClosed_;
}

// ═══════════════════════════════════════════════════════════════════════════
// Public helpers
// ═══════════════════════════════════════════════════════════════════════════

void SFMLRenderer::setCurrentPlayer(char symbol) { currentPlayer_ = symbol; }
char SFMLRenderer::getCurrentPlayer() const       { return currentPlayer_;  }

void SFMLRenderer::addScore(char result) {
    if (result == 'X')      ++scores_[0];
    else if (result == 'O') ++scores_[1];
    else                    ++scores_[2];   // Draw
}

void SFMLRenderer::resetScores() { scores_ = {0, 0, 0}; }

bool SFMLRenderer::wasNewGameRequested() {
    bool r = newGameRequested_;
    newGameRequested_ = false;
    return r;
}

sf::RenderWindow& SFMLRenderer::getWindow() { return window_; }

void SFMLRenderer::inferCurrentPlayer(const Board& board) {
    int xCount = 0, oCount = 0;
    for (int r = 0; r < boardSize_; ++r) {
        for (int c = 0; c < boardSize_; ++c) {
            if (board.getCell(r, c) == 'X') ++xCount;
            else if (board.getCell(r, c) == 'O') ++oCount;
        }
    }
    // X always goes first, so if counts are equal it's X's turn.
    currentPlayer_ = (xCount <= oCount) ? 'X' : 'O';
}

// ═══════════════════════════════════════════════════════════════════════════
// Board drawing
// ═══════════════════════════════════════════════════════════════════════════

void SFMLRenderer::drawBoard(const Board& board) {
    // Background rectangle for the board area.
    sf::RectangleShape bg(sf::Vector2f{boardPixelSize_, boardPixelSize_});
    bg.setPosition(sf::Vector2f{boardOriginX_, boardOriginY_});
    bg.setFillColor(sf::Color{0x0f, 0x0f, 0x23});
    window_.draw(bg);

    drawGridLines();
    drawMarks(board);
    drawHoverPreview();
    drawWinHighlight(board);
}

// ── Grid lines ──────────────────────────────────────────────────────────

void SFMLRenderer::drawGridLines() {
    constexpr float kLineThickness = 3.f;

    // Vertical lines.
    for (int i = 1; i < boardSize_; ++i) {
        float x = boardOriginX_ + i * cellSize_;
        sf::RectangleShape line(sf::Vector2f{kLineThickness, boardPixelSize_});
        line.setPosition(sf::Vector2f{x - kLineThickness / 2.f, boardOriginY_});
        line.setFillColor(kGridLine);
        window_.draw(line);
    }

    // Horizontal lines.
    for (int i = 1; i < boardSize_; ++i) {
        float y = boardOriginY_ + i * cellSize_;
        sf::RectangleShape line(sf::Vector2f{boardPixelSize_, kLineThickness});
        line.setPosition(sf::Vector2f{boardOriginX_, y - kLineThickness / 2.f});
        line.setFillColor(kGridLine);
        window_.draw(line);
    }
}

// ── Marks (X and O) ────────────────────────────────────────────────────

void SFMLRenderer::drawMarks(const Board& board) {
    for (int r = 0; r < boardSize_; ++r) {
        for (int c = 0; c < boardSize_; ++c) {
            const char cell = board.getCell(r, c);
            if (cell == '.') continue;

            const float cx = boardOriginX_ + (c + 0.5f) * cellSize_;
            const float cy = boardOriginY_ + (r + 0.5f) * cellSize_;
            const float sz = cellSize_ * 0.65f;  // symbol occupies ~65 % of cell

            if (cell == 'X') {
                drawX(cx, cy, sz, kXColor);
            } else {
                drawO(cx, cy, sz, kOColor);
            }
        }
    }
}

// ── X drawing (two crossed thick lines) ─────────────────────────────────

void SFMLRenderer::drawX(float cx, float cy, float size,
                          const sf::Color& color, float thickness) {
    // Compute the length of each diagonal bar.
    const float diagLen = std::sqrt(2.f) * size;

    for (int sign : {-1, 1}) {
        sf::RectangleShape bar(sf::Vector2f{diagLen, thickness});
        bar.setOrigin(sf::Vector2f{diagLen / 2.f, thickness / 2.f});
        bar.setPosition(sf::Vector2f{cx, cy});
        bar.setRotation(sf::degrees(sign * 45.f));
        bar.setFillColor(color);
        window_.draw(bar);
    }
}

// ── O drawing (circle outline) ──────────────────────────────────────────

void SFMLRenderer::drawO(float cx, float cy, float size,
                          const sf::Color& color, float thickness) {
    const float radius = size / 2.f;

    sf::CircleShape circle(radius - thickness / 2.f);
    circle.setOrigin(sf::Vector2f{radius - thickness / 2.f, radius - thickness / 2.f});
    circle.setPosition(sf::Vector2f{cx, cy});
    circle.setFillColor(sf::Color::Transparent);
    circle.setOutlineColor(color);
    circle.setOutlineThickness(thickness);
    circle.setPointCount(64);   // smooth circle
    window_.draw(circle);
}

// ── Hover preview ───────────────────────────────────────────────────────

void SFMLRenderer::drawHoverPreview() {
    if (hoverRow_ < 0 || hoverCol_ < 0) return;

    // Draw a faint background highlight on the hovered cell.
    const float x = boardOriginX_ + hoverCol_ * cellSize_;
    const float y = boardOriginY_ + hoverRow_ * cellSize_;

    sf::RectangleShape highlight(sf::Vector2f{cellSize_, cellSize_});
    highlight.setPosition(sf::Vector2f{x, y});
    highlight.setFillColor(kHover);
    window_.draw(highlight);

    // Draw a faint preview symbol.
    const float cx = x + cellSize_ / 2.f;
    const float cy = y + cellSize_ / 2.f;
    const float sz = cellSize_ * 0.65f;

    sf::Color faint = (currentPlayer_ == 'X') ? kXColor : kOColor;
    faint.a = 60;   // very translucent

    if (currentPlayer_ == 'X') {
        drawX(cx, cy, sz, faint, 4.f);
    } else {
        drawO(cx, cy, sz, faint, 3.f);
    }
}

// ── Win highlight ───────────────────────────────────────────────────────

void SFMLRenderer::drawWinHighlight(const Board& board) {
    // Determine if there is a winner.
    char winner = '.';
    if (board.checkWin('X'))      winner = 'X';
    else if (board.checkWin('O')) winner = 'O';
    if (winner == '.') return;

    const auto cells = board.getWinningLine(winner);
    for (const auto& [r, c] : cells) {
        const float x = boardOriginX_ + c * cellSize_;
        const float y = boardOriginY_ + r * cellSize_;

        sf::RectangleShape glow(sf::Vector2f{cellSize_, cellSize_});
        glow.setPosition(sf::Vector2f{x, y});
        glow.setFillColor(sf::Color{kWinGlow.r, kWinGlow.g, kWinGlow.b, 40});
        window_.draw(glow);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Info panel drawing
// ═══════════════════════════════════════════════════════════════════════════

void SFMLRenderer::drawInfoPanel(const Board& board) {
    constexpr float kPad     = 15.f;
    const float panelH       = static_cast<float>(WINDOW_H) - 60.f;
    const float panelY       = 30.f;

    // Panel background.
    sf::RectangleShape panelBg(sf::Vector2f{panelW_, panelH});
    panelBg.setPosition(sf::Vector2f{panelX_, panelY});
    panelBg.setFillColor(kPanelBg);
    window_.draw(panelBg);

    float curY = panelY + kPad;

    // ── Title ───────────────────────────────────────────────────────
    {
        sf::Text title(font_, "Tic-Tac-Toe", 26);
        title.setFillColor(kText);
        // Centre horizontally in panel.
        const float tw = title.getLocalBounds().size.x;
        title.setPosition(sf::Vector2f{panelX_ + (panelW_ - tw) / 2.f, curY});
        window_.draw(title);
        curY += 45.f;
    }

    // ── Divider ─────────────────────────────────────────────────────
    {
        sf::RectangleShape div(sf::Vector2f{panelW_ - 2.f * kPad, 1.f});
        div.setPosition(sf::Vector2f{panelX_ + kPad, curY});
        div.setFillColor(sf::Color{255, 255, 255, 30});
        window_.draw(div);
        curY += 20.f;
    }

    // ── Current turn ────────────────────────────────────────────────
    {
        sf::Text turnLabel(font_, "Current Turn", 16);
        turnLabel.setFillColor(sf::Color{180, 180, 180});
        turnLabel.setPosition(sf::Vector2f{panelX_ + kPad, curY});
        window_.draw(turnLabel);
        curY += 28.f;

        // Draw the symbol in its colour.
        std::string sym(1, currentPlayer_);
        sf::Text turnSym(font_, sym, 40);
        turnSym.setFillColor(currentPlayer_ == 'X' ? kXColor : kOColor);
        const float sw = turnSym.getLocalBounds().size.x;
        turnSym.setPosition(sf::Vector2f{panelX_ + (panelW_ - sw) / 2.f, curY});
        window_.draw(turnSym);
        curY += 60.f;
    }

    // ── Game status ─────────────────────────────────────────────────
    {
        std::string status;
        if (board.checkWin('X'))      status = "X wins!";
        else if (board.checkWin('O')) status = "O wins!";
        else if (board.isFull())      status = "It's a draw!";
        else                          status = "Game in progress";

        sf::Text statusText(font_, status, 18);
        statusText.setFillColor(kText);
        const float sw = statusText.getLocalBounds().size.x;
        statusText.setPosition(sf::Vector2f{panelX_ + (panelW_ - sw) / 2.f, curY});
        window_.draw(statusText);
        curY += 40.f;
    }

    // ── Divider ─────────────────────────────────────────────────────
    {
        sf::RectangleShape div(sf::Vector2f{panelW_ - 2.f * kPad, 1.f});
        div.setPosition(sf::Vector2f{panelX_ + kPad, curY});
        div.setFillColor(sf::Color{255, 255, 255, 30});
        window_.draw(div);
        curY += 20.f;
    }

    // ── Score tracker ───────────────────────────────────────────────
    {
        sf::Text scoreLabel(font_, "Scoreboard", 16);
        scoreLabel.setFillColor(sf::Color{180, 180, 180});
        scoreLabel.setPosition(sf::Vector2f{panelX_ + kPad, curY});
        window_.draw(scoreLabel);
        curY += 28.f;

        auto drawScoreLine = [&](const std::string& label, int value,
                                 const sf::Color& color) {
            sf::Text lbl(font_, label, 18);
            lbl.setFillColor(color);
            lbl.setPosition(sf::Vector2f{panelX_ + kPad + 10.f, curY});
            window_.draw(lbl);

            sf::Text val(font_, std::to_string(value), 18);
            val.setFillColor(kText);
            val.setPosition(sf::Vector2f{panelX_ + panelW_ - kPad - 30.f, curY});
            window_.draw(val);

            curY += 26.f;
        };

        drawScoreLine("X Wins", scores_[0], kXColor);
        drawScoreLine("O Wins", scores_[1], kOColor);
        drawScoreLine("Draws",  scores_[2], sf::Color{180, 180, 180});
        curY += 15.f;
    }

    // ── New Game button ─────────────────────────────────────────────
    {
        const float btnW = panelW_ - 2.f * kPad;
        const float btnH = 44.f;
        const float btnX = panelX_ + kPad;
        const float btnY = curY;

        newGameBtnRect_ = sf::FloatRect(sf::Vector2f{btnX, btnY},
                                        sf::Vector2f{btnW, btnH});

        drawButton(btnX, btnY, btnW, btnH, "New Game",
                   sf::Color{0x2d, 0x46, 0x6b});
    }
}

// ── Generic button ──────────────────────────────────────────────────────

void SFMLRenderer::drawButton(float x, float y, float w, float h,
                               const std::string& label, const sf::Color& bg) {
    sf::RectangleShape btn(sf::Vector2f{w, h});
    btn.setPosition(sf::Vector2f{x, y});
    btn.setFillColor(bg);
    window_.draw(btn);

    sf::Text text(font_, label, 18);
    text.setFillColor(kText);
    const float tw = text.getLocalBounds().size.x;
    const float th = text.getLocalBounds().size.y;
    text.setPosition(sf::Vector2f{x + (w - tw) / 2.f,
                                  y + (h - th) / 2.f - 4.f});
    window_.draw(text);
}

// ═══════════════════════════════════════════════════════════════════════════
// Result overlay
// ═══════════════════════════════════════════════════════════════════════════

void SFMLRenderer::drawResultOverlay() {
    // Semi-transparent dark overlay.
    sf::RectangleShape overlay(sf::Vector2f{
        static_cast<float>(WINDOW_W), static_cast<float>(WINDOW_H)});
    overlay.setFillColor(sf::Color{0, 0, 0, 160});
    window_.draw(overlay);

    // Result text — large, centred.
    sf::Text msg(font_, resultMessage_, 42);
    msg.setFillColor(kWinGlow);
    const float tw = msg.getLocalBounds().size.x;
    const float th = msg.getLocalBounds().size.y;
    msg.setPosition(sf::Vector2f{(WINDOW_W - tw) / 2.f,
                                 (WINDOW_H - th) / 2.f - 40.f});
    window_.draw(msg);

    // Sub-text.
    sf::Text sub(font_, "Click 'New Game' to play again", 18);
    sub.setFillColor(kText);
    const float sw = sub.getLocalBounds().size.x;
    sub.setPosition(sf::Vector2f{(WINDOW_W - sw) / 2.f,
                                 (WINDOW_H - th) / 2.f + 30.f});
    window_.draw(sub);
}

// ═══════════════════════════════════════════════════════════════════════════
// Hit-testing
// ═══════════════════════════════════════════════════════════════════════════

std::pair<int, int> SFMLRenderer::pixelToCell(sf::Vector2i pixel) const {
    const float px = static_cast<float>(pixel.x);
    const float py = static_cast<float>(pixel.y);

    // Check if inside the board area.
    if (px < boardOriginX_ || py < boardOriginY_ ||
        px >= boardOriginX_ + boardPixelSize_ ||
        py >= boardOriginY_ + boardPixelSize_) {
        return {-1, -1};
    }

    const int col = static_cast<int>((px - boardOriginX_) / cellSize_);
    const int row = static_cast<int>((py - boardOriginY_) / cellSize_);

    // Clamp to valid range (safety).
    if (row < 0 || row >= boardSize_ || col < 0 || col >= boardSize_) {
        return {-1, -1};
    }

    return {row, col};
}

bool SFMLRenderer::isInsideNewGameButton(sf::Vector2i pixel) const {
    const float px = static_cast<float>(pixel.x);
    const float py = static_cast<float>(pixel.y);
    return px >= newGameBtnRect_.position.x &&
           py >= newGameBtnRect_.position.y &&
           px <= newGameBtnRect_.position.x + newGameBtnRect_.size.x &&
           py <= newGameBtnRect_.position.y + newGameBtnRect_.size.y;
}
