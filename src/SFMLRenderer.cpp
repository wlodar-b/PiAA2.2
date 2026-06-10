#include "SFMLRenderer.hpp"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <string>

// Black & white palette
const sf::Color SFMLRenderer::kBackground{0x0a, 0x0a, 0x0a};
const sf::Color SFMLRenderer::kGridLine  {0x2a, 0x2a, 0x2a};
const sf::Color SFMLRenderer::kXColor    {0xff, 0xff, 0xff};
const sf::Color SFMLRenderer::kOColor    {0x88, 0x88, 0x88};
const sf::Color SFMLRenderer::kHover     {255, 255, 255, 8};
const sf::Color SFMLRenderer::kWinGlow   {0xff, 0xff, 0xff};
const sf::Color SFMLRenderer::kText      {0xff, 0xff, 0xff};
const sf::Color SFMLRenderer::kPanelBg   {0x14, 0x14, 0x14};

SFMLRenderer::SFMLRenderer(const ttt::Game& game, sf::Font& font)
    : window_(sf::VideoMode(sf::Vector2u{WINDOW_W, WINDOW_H}),
              "Tic-Tac-Toe",
              sf::Style::Default,
              sf::State::Windowed)
    , font_(font)
    , game_(&game)
{
    window_.setFramerateLimit(60);
    recalculateLayout();
}

void SFMLRenderer::setGame(const ttt::Game& game) {
    game_ = &game;
    hoverRow_ = -1;
    hoverCol_ = -1;
    recalculateLayout();
}

bool SFMLRenderer::shouldClose() const { return windowClosed_; }

void SFMLRenderer::showResult(const std::string& message) { resultMessage_ = message; }

void SFMLRenderer::addScore(ttt::GameStatus result) {
    if (result == ttt::GameStatus::XWins)      ++scores_[0];
    else if (result == ttt::GameStatus::OWins)  ++scores_[1];
    else if (result == ttt::GameStatus::Draw)   ++scores_[2];
}

void SFMLRenderer::resetScores() { scores_ = {0, 0, 0}; }

bool SFMLRenderer::wasNewGameRequested() {
    bool r = newGameRequested_;
    newGameRequested_ = false;
    return r;
}

bool SFMLRenderer::wasMenuRequested() {
    bool r = menuRequested_;
    menuRequested_ = false;
    return r;
}

sf::RenderWindow& SFMLRenderer::getWindow() { return window_; }

void SFMLRenderer::recalculateLayout() {
    constexpr float kPad = 30.f;
    const int boardSize = game_->board().size();

    const float availW = WINDOW_W * 0.58f - 2.f * kPad;
    const float availH = WINDOW_H - 2.f * kPad;
    boardPixelSize_ = std::min(availW, availH);
    cellSize_ = boardPixelSize_ / static_cast<float>(boardSize);

    boardOriginX_ = kPad;
    boardOriginY_ = (WINDOW_H - boardPixelSize_) / 2.f;

    panelX_ = boardOriginX_ + boardPixelSize_ + kPad;
    panelW_ = WINDOW_W - panelX_ - kPad;
}

void SFMLRenderer::render() {
    window_.clear(kBackground);
    drawBoard();
    drawInfoPanel();
    if (!resultMessage_.empty()) drawResultOverlay();
    window_.display();
}

ttt::Move SFMLRenderer::getHumanMove() {
    while (window_.isOpen()) {
        while (const auto event = window_.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window_.close();
                windowClosed_ = true;
                return {};
            }
            if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (isInsideNewGameButton(mb->position)) {
                    newGameRequested_ = true;
                    return {};
                }
                if (isInsideMenuButton(mb->position)) {
                    menuRequested_ = true;
                    return {};
                }
                auto [row, col] = pixelToCell(mb->position);
                if (row >= 0 && col >= 0 && game_->board().isEmpty(row, col)) {
                    return {row, col};
                }
            }
            if (const auto* mm = event->getIf<sf::Event::MouseMoved>()) {
                auto [r, c] = pixelToCell(mm->position);
                hoverRow_ = r;
                hoverCol_ = c;
            }
        }
        window_.clear(kBackground);
        drawBoard();
        drawInfoPanel();
        window_.display();
    }
    windowClosed_ = true;
    return {};
}

void SFMLRenderer::drawBoard() {
    sf::RectangleShape bg(sf::Vector2f{boardPixelSize_, boardPixelSize_});
    bg.setPosition(sf::Vector2f{boardOriginX_, boardOriginY_});
    bg.setFillColor(sf::Color{0x11, 0x11, 0x11});
    window_.draw(bg);
    drawGridLines();
    drawMarks();
    drawHoverPreview();
    drawWinHighlight();
}

void SFMLRenderer::drawGridLines() {
    constexpr float kLineThickness = 2.f;
    const int boardSize = game_->board().size();
    for (int i = 1; i < boardSize; ++i) {
        float x = boardOriginX_ + i * cellSize_;
        sf::RectangleShape line(sf::Vector2f{kLineThickness, boardPixelSize_});
        line.setPosition(sf::Vector2f{x - kLineThickness / 2.f, boardOriginY_});
        line.setFillColor(kGridLine);
        window_.draw(line);
    }
    for (int i = 1; i < boardSize; ++i) {
        float y = boardOriginY_ + i * cellSize_;
        sf::RectangleShape line(sf::Vector2f{boardPixelSize_, kLineThickness});
        line.setPosition(sf::Vector2f{boardOriginX_, y - kLineThickness / 2.f});
        line.setFillColor(kGridLine);
        window_.draw(line);
    }
}

void SFMLRenderer::drawMarks() {
    const auto& board = game_->board();
    const int boardSize = board.size();
    for (int r = 0; r < boardSize; ++r) {
        for (int c = 0; c < boardSize; ++c) {
            ttt::Cell cell = board.at(r, c);
            if (cell == ttt::Cell::Empty) continue;
            const float cx = boardOriginX_ + (c + 0.5f) * cellSize_;
            const float cy = boardOriginY_ + (r + 0.5f) * cellSize_;
            const float sz = cellSize_ * 0.6f;
            if (cell == ttt::Cell::X) drawX(cx, cy, sz, kXColor);
            else                      drawO(cx, cy, sz, kOColor);
        }
    }
}

void SFMLRenderer::drawX(float cx, float cy, float size,
                          const sf::Color& color, float thickness) {
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

void SFMLRenderer::drawO(float cx, float cy, float size,
                          const sf::Color& color, float thickness) {
    const float radius = size / 2.f;
    sf::CircleShape circle(radius - thickness / 2.f);
    circle.setOrigin(sf::Vector2f{radius - thickness / 2.f, radius - thickness / 2.f});
    circle.setPosition(sf::Vector2f{cx, cy});
    circle.setFillColor(sf::Color::Transparent);
    circle.setOutlineColor(color);
    circle.setOutlineThickness(thickness);
    circle.setPointCount(64);
    window_.draw(circle);
}

void SFMLRenderer::drawHoverPreview() {
    if (hoverRow_ < 0 || hoverCol_ < 0) return;
    if (game_->isOver()) return;
    if (!game_->board().isEmpty(hoverRow_, hoverCol_)) return;

    const float x = boardOriginX_ + hoverCol_ * cellSize_;
    const float y = boardOriginY_ + hoverRow_ * cellSize_;
    sf::RectangleShape highlight(sf::Vector2f{cellSize_, cellSize_});
    highlight.setPosition(sf::Vector2f{x, y});
    highlight.setFillColor(kHover);
    window_.draw(highlight);

    const float cx = x + cellSize_ / 2.f;
    const float cy = y + cellSize_ / 2.f;
    const float sz = cellSize_ * 0.6f;
    ttt::Cell cur = game_->currentPlayer();
    sf::Color faint = (cur == ttt::Cell::X) ? kXColor : kOColor;
    faint.a = 40;
    if (cur == ttt::Cell::X) drawX(cx, cy, sz, faint, 4.f);
    else                     drawO(cx, cy, sz, faint, 3.f);
}

void SFMLRenderer::drawWinHighlight() {
    const auto& winLine = game_->winningLine();
    if (winLine.empty()) return;
    for (const auto& move : winLine) {
        const float x = boardOriginX_ + move.col * cellSize_;
        const float y = boardOriginY_ + move.row * cellSize_;
        sf::RectangleShape glow(sf::Vector2f{cellSize_, cellSize_});
        glow.setPosition(sf::Vector2f{x, y});
        glow.setFillColor(sf::Color{255, 255, 255, 25});
        window_.draw(glow);
    }
}

void SFMLRenderer::drawInfoPanel() {
    constexpr float kPad = 20.f;
    const float panelH = static_cast<float>(WINDOW_H) - 60.f;
    const float panelY = 30.f;

    sf::RectangleShape panelBg(sf::Vector2f{panelW_, panelH});
    panelBg.setPosition(sf::Vector2f{panelX_, panelY});
    panelBg.setFillColor(kPanelBg);
    window_.draw(panelBg);

    float curY = panelY + kPad + 5.f;

    // Title
    {
        sf::Text title(font_, "Tic-Tac-Toe", 28);
        title.setFillColor(kText);
        const float tw = title.getLocalBounds().size.x;
        title.setPosition(sf::Vector2f{panelX_ + (panelW_ - tw) / 2.f, curY});
        window_.draw(title);
        curY += 50.f;
    }

    // Divider
    {
        sf::RectangleShape div(sf::Vector2f{panelW_ - 2.f * kPad, 1.f});
        div.setPosition(sf::Vector2f{panelX_ + kPad, curY});
        div.setFillColor(sf::Color{255, 255, 255, 20});
        window_.draw(div);
        curY += 25.f;
    }

    // Current turn label
    {
        sf::Text turnLabel(font_, "Current Turn", 16);
        turnLabel.setFillColor(sf::Color{0x88, 0x88, 0x88});
        const float tw = turnLabel.getLocalBounds().size.x;
        turnLabel.setPosition(sf::Vector2f{panelX_ + (panelW_ - tw) / 2.f, curY});
        window_.draw(turnLabel);
        curY += 30.f;
    }

    // Turn symbol
    {
        ttt::Cell cur = game_->currentPlayer();
        std::string sym = (cur == ttt::Cell::X) ? "X" : "O";
        sf::Text turnSym(font_, sym, 44);
        turnSym.setFillColor(cur == ttt::Cell::X ? kXColor : kOColor);
        const float sw = turnSym.getLocalBounds().size.x;
        turnSym.setPosition(sf::Vector2f{panelX_ + (panelW_ - sw) / 2.f, curY});
        window_.draw(turnSym);
        curY += 65.f;
    }

    // Game status
    {
        std::string status;
        switch (game_->status()) {
            case ttt::GameStatus::XWins:      status = "X wins!"; break;
            case ttt::GameStatus::OWins:      status = "O wins!"; break;
            case ttt::GameStatus::Draw:       status = "Draw!"; break;
            case ttt::GameStatus::InProgress: status = "Game in progress"; break;
        }
        sf::Text statusText(font_, status, 18);
        statusText.setFillColor(sf::Color{0xaa, 0xaa, 0xaa});
        const float sw = statusText.getLocalBounds().size.x;
        statusText.setPosition(sf::Vector2f{panelX_ + (panelW_ - sw) / 2.f, curY});
        window_.draw(statusText);
        curY += 40.f;
    }

    // Divider
    {
        sf::RectangleShape div(sf::Vector2f{panelW_ - 2.f * kPad, 1.f});
        div.setPosition(sf::Vector2f{panelX_ + kPad, curY});
        div.setFillColor(sf::Color{255, 255, 255, 20});
        window_.draw(div);
        curY += 25.f;
    }

    // Scoreboard
    {
        sf::Text scoreLabel(font_, "Scoreboard", 16);
        scoreLabel.setFillColor(sf::Color{0x88, 0x88, 0x88});
        const float tw = scoreLabel.getLocalBounds().size.x;
        scoreLabel.setPosition(sf::Vector2f{panelX_ + (panelW_ - tw) / 2.f, curY});
        window_.draw(scoreLabel);
        curY += 32.f;

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
            curY += 28.f;
        };

        drawScoreLine("X Wins", scores_[0], kXColor);
        drawScoreLine("O Wins", scores_[1], kOColor);
        drawScoreLine("Draws",  scores_[2], sf::Color{0x66, 0x66, 0x66});
        curY += 20.f;
    }

    // New Game button
    {
        const float btnW = panelW_ - 2.f * kPad;
        const float btnH = 48.f;
        const float btnX = panelX_ + kPad;
        const float btnY = curY;
        newGameBtnRect_ = sf::FloatRect(sf::Vector2f{btnX, btnY},
                                        sf::Vector2f{btnW, btnH});
        drawButton(btnX, btnY, btnW, btnH, "New Game",
                   sf::Color{0x25, 0x25, 0x25});
        curY += btnH + 12.f;
    }

    // Menu button
    {
        const float btnW = panelW_ - 2.f * kPad;
        const float btnH = 42.f;
        const float btnX = panelX_ + kPad;
        const float btnY = curY;
        menuBtnRect_ = sf::FloatRect(sf::Vector2f{btnX, btnY},
                                     sf::Vector2f{btnW, btnH});
        drawButton(btnX, btnY, btnW, btnH, "Menu",
                   sf::Color{0x1a, 0x1a, 0x1a});
    }
}

void SFMLRenderer::drawButton(float x, float y, float w, float h,
                               const std::string& label, const sf::Color& bg) {
    sf::RectangleShape btn(sf::Vector2f{w, h});
    btn.setPosition(sf::Vector2f{x, y});
    btn.setFillColor(bg);
    btn.setOutlineColor(sf::Color{0x33, 0x33, 0x33});
    btn.setOutlineThickness(1.f);
    window_.draw(btn);

    sf::Text text(font_, label, 18);
    text.setFillColor(kText);
    const float tw = text.getLocalBounds().size.x;
    const float th = text.getLocalBounds().size.y;
    text.setPosition(sf::Vector2f{x + (w - tw) / 2.f,
                                  y + (h - th) / 2.f - 4.f});
    window_.draw(text);
}

void SFMLRenderer::drawResultOverlay() {
    sf::RectangleShape overlay(sf::Vector2f{
        static_cast<float>(WINDOW_W), static_cast<float>(WINDOW_H)});
    overlay.setFillColor(sf::Color{0, 0, 0, 180});
    window_.draw(overlay);

    sf::Text msg(font_, resultMessage_, 48);
    msg.setFillColor(sf::Color::White);
    const float tw = msg.getLocalBounds().size.x;
    const float th = msg.getLocalBounds().size.y;
    msg.setPosition(sf::Vector2f{(WINDOW_W - tw) / 2.f,
                                 (WINDOW_H - th) / 2.f - 40.f});
    window_.draw(msg);

    sf::Text sub(font_, "Click anywhere to continue", 16);
    sub.setFillColor(sf::Color{0x88, 0x88, 0x88});
    const float sw = sub.getLocalBounds().size.x;
    sub.setPosition(sf::Vector2f{(WINDOW_W - sw) / 2.f,
                                 (WINDOW_H - th) / 2.f + 30.f});
    window_.draw(sub);
}

std::pair<int, int> SFMLRenderer::pixelToCell(sf::Vector2i pixel) const {
    const float px = static_cast<float>(pixel.x);
    const float py = static_cast<float>(pixel.y);
    const int boardSize = game_->board().size();
    if (px < boardOriginX_ || py < boardOriginY_ ||
        px >= boardOriginX_ + boardPixelSize_ ||
        py >= boardOriginY_ + boardPixelSize_) {
        return {-1, -1};
    }
    const int col = static_cast<int>((px - boardOriginX_) / cellSize_);
    const int row = static_cast<int>((py - boardOriginY_) / cellSize_);
    if (row < 0 || row >= boardSize || col < 0 || col >= boardSize) {
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

bool SFMLRenderer::isInsideMenuButton(sf::Vector2i pixel) const {
    const float px = static_cast<float>(pixel.x);
    const float py = static_cast<float>(pixel.y);
    return px >= menuBtnRect_.position.x &&
           py >= menuBtnRect_.position.y &&
           px <= menuBtnRect_.position.x + menuBtnRect_.size.x &&
           py <= menuBtnRect_.position.y + menuBtnRect_.size.y;
}
