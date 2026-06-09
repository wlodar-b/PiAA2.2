#pragma once

#include "Game.hpp"
#include "Types.hpp"

#include <SFML/Graphics.hpp>
#include <array>
#include <string>

/**
 * @brief SFML 3.0-based graphical renderer for Tic-Tac-Toe.
 *
 * Renders a premium dark-themed game board on the left side of the window,
 * with an information panel (turn indicator, status, scores, New Game button)
 * on the right. Supports hover previews, winning-line highlights, and
 * animated result overlays.
 *
 * Colour palette (dark theme):
 *   Background   #1a1a2e    Grid lines  #16213e
 *   X symbol     #e94560    O symbol    #4da6ff
 *   Hover tint   rgba(255,255,255,13)   Win glow  #ffd700
 *   Text         #eaeaea    Panel bg    #16213e
 */
class SFMLRenderer {
public:
    /**
     * @brief Constructs the SFML renderer, opening the game window.
     * @param game  Reference to the Game object to render.
     * @param font  Reference to an already-loaded sf::Font.
     */
    SFMLRenderer(const ttt::Game& game, sf::Font& font);

    /// Draws the board, info panel, and any overlays to the window.
    void render();

    /**
     * @brief Waits for the human player to click a valid empty cell.
     * @return A Move for the chosen cell, or invalid Move if window closed.
     */
    ttt::Move getHumanMove();

    /// Displays a translucent overlay with the result message.
    void showResult(const std::string& message);

    /// Returns true if the user has closed the window.
    bool shouldClose() const;

    // ── GUI-mode helpers ────────────────────────────────────────────────

    /// Increment the score counter for the given result.
    void addScore(ttt::GameStatus result);

    /// Reset scores to zero.
    void resetScores();

    /// Returns true if the "New Game" button was clicked (and resets the flag).
    bool wasNewGameRequested();

    /// Get the underlying SFML window (non-const, for event processing in main).
    sf::RenderWindow& getWindow();

    /// Update the game reference (after resetting with new config).
    void setGame(const ttt::Game& game);

private:
    // ── Window & layout ─────────────────────────────────────────────────
    sf::RenderWindow   window_;
    sf::Font&          font_;
    const ttt::Game*   game_;

    static constexpr unsigned WINDOW_W = 900;
    static constexpr unsigned WINDOW_H = 700;

    // Board area (left portion of the window).
    float boardOriginX_  = 0.f;
    float boardOriginY_  = 0.f;
    float boardPixelSize_ = 0.f;
    float cellSize_       = 0.f;

    // Panel area (right portion).
    float panelX_ = 0.f;
    float panelW_ = 0.f;

    // ── State ───────────────────────────────────────────────────────────
    int  hoverRow_       = -1;
    int  hoverCol_       = -1;
    bool windowClosed_   = false;
    bool newGameRequested_ = false;

    // Scores: [X wins, O wins, Draws].
    std::array<int, 3> scores_ = {0, 0, 0};

    // Result overlay text (empty ⇒ no overlay shown).
    std::string resultMessage_;

    // ── Colour constants ────────────────────────────────────────────────
    static const sf::Color kBackground;
    static const sf::Color kGridLine;
    static const sf::Color kXColor;
    static const sf::Color kOColor;
    static const sf::Color kHover;
    static const sf::Color kWinGlow;
    static const sf::Color kText;
    static const sf::Color kPanelBg;

    // ── Layout helpers ──────────────────────────────────────────────────
    void recalculateLayout();

    // ── Drawing helpers ─────────────────────────────────────────────────
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

    // ── Hit-testing ─────────────────────────────────────────────────────
    std::pair<int, int> pixelToCell(sf::Vector2i pixel) const;
    bool isInsideNewGameButton(sf::Vector2i pixel) const;

    // "New Game" button rectangle (set during drawInfoPanel).
    sf::FloatRect newGameBtnRect_;
};
