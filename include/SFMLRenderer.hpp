#pragma once

#include "Game.hpp"
#include "Types.hpp"

#include <SFML/Graphics.hpp>
#include <array>
#include <string>

class SFMLRenderer {
public:
    SFMLRenderer(const ttt::Game& game, sf::Font& font);

    void render();
    ttt::Move getHumanMove();
    void showResult(const std::string& message);
    bool shouldClose() const;

    void addScore(ttt::GameStatus result);
    void resetScores();
    bool wasNewGameRequested();
    void clearNewGameRequest() { newGameRequested_ = false; }
    bool isInsideNewGameButton(sf::Vector2i pixel) const;
    bool wasMenuRequested();
    bool isInsideMenuButton(sf::Vector2i pixel) const;
    sf::RenderWindow& getWindow();
    void setGame(const ttt::Game& game);

private:
    sf::RenderWindow   window_;
    sf::Font&          font_;
    const ttt::Game*   game_;

    static constexpr unsigned WINDOW_W = 1050;
    static constexpr unsigned WINDOW_H = 750;

    float boardOriginX_  = 0.f;
    float boardOriginY_  = 0.f;
    float boardPixelSize_ = 0.f;
    float cellSize_       = 0.f;

    float panelX_ = 0.f;
    float panelW_ = 0.f;

    int  hoverRow_         = -1;
    int  hoverCol_         = -1;
    bool windowClosed_     = false;
    bool newGameRequested_ = false;
    bool menuRequested_    = false;

    std::array<int, 3> scores_ = {0, 0, 0};
    std::string resultMessage_;

    static const sf::Color kBackground;
    static const sf::Color kGridLine;
    static const sf::Color kXColor;
    static const sf::Color kOColor;
    static const sf::Color kHover;
    static const sf::Color kWinGlow;
    static const sf::Color kText;
    static const sf::Color kPanelBg;

    void recalculateLayout();
    void drawBoard();
    void drawGridLines();
    void drawMarks();
    void drawX(float cx, float cy, float size, const sf::Color& color, float thickness = 6.f);
    void drawO(float cx, float cy, float size, const sf::Color& color, float thickness = 4.f);
    void drawHoverPreview();
    void drawWinHighlight();
    void drawInfoPanel();
    void drawButton(float x, float y, float w, float h,
                    const std::string& label, const sf::Color& bg);
    void drawResultOverlay();

    std::pair<int, int> pixelToCell(sf::Vector2i pixel) const;

    sf::FloatRect newGameBtnRect_;
    sf::FloatRect menuBtnRect_;
};
