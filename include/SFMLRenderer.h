#pragma once

#include "Renderer.h"
#include "Board.h"

#include <SFML/Graphics.hpp>
#include <string>
#include <utility>
#include <array>

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
class SFMLRenderer : public Renderer {
public:
    /**
     * @brief Constructs the SFML renderer, opening the game window.
     * @param boardSize  Number of rows/columns on the board.
     * @param font       Reference to an already-loaded sf::Font.
     */
    SFMLRenderer(int boardSize, sf::Font& font);

    // ── Renderer interface ──────────────────────────────────────────────

    /// Draws the board, info panel, and any overlays to the window.
    void render(const Board& board) override;

    /**
     * @brief Waits for the human player to click a valid empty cell.
     *
     * While waiting the method continues to process window events (close,
     * resize) and renders a hover preview of the current player's symbol.
     *
     * @param board  The current board state (used for validation / previews).
     * @return A (row, col) pair for the chosen cell.
     */
    std::pair<int, int> getHumanMove(const Board& board) override;

    /// Displays a translucent overlay with the result message.
    void showResult(const std::string& message) override;

    /// Returns true if the user has closed the window.
    bool shouldClose() const override;

    // ── GUI-mode helpers ────────────────────────────────────────────────

    /// Set the current player symbol (used for hover previews).
    void setCurrentPlayer(char symbol);

    /// Get the current player symbol.
    char getCurrentPlayer() const;

    /// Increment the score counter for the given symbol ('X', 'O', or 'D' for draw).
    void addScore(char result);

    /// Reset scores to zero.
    void resetScores();

    /// Returns true if the "New Game" button was clicked (and resets the flag).
    bool wasNewGameRequested();

    /// Get the underlying SFML window (non-const, for event processing in main).
    sf::RenderWindow& getWindow();

private:
    // ── Window & layout ─────────────────────────────────────────────────
    sf::RenderWindow window_;
    sf::Font&        font_;
    int              boardSize_;

    static constexpr unsigned WINDOW_W = 900;
    static constexpr unsigned WINDOW_H = 700;

    // Board area (left portion of the window).
    float boardOriginX_ = 0.f;
    float boardOriginY_ = 0.f;
    float boardPixelSize_ = 0.f;   ///< Total pixel size of the square board area.
    float cellSize_       = 0.f;   ///< Pixel size of one cell.

    // Panel area (right portion).
    float panelX_ = 0.f;
    float panelW_ = 0.f;

    // ── State ───────────────────────────────────────────────────────────
    char currentPlayer_ = 'X';
    int  hoverRow_      = -1;
    int  hoverCol_      = -1;
    bool windowClosed_  = false;
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
    void drawBoard(const Board& board);
    void drawGridLines();
    void drawMarks(const Board& board);
    void drawX(float cx, float cy, float size, const sf::Color& color, float thickness = 6.f);
    void drawO(float cx, float cy, float size, const sf::Color& color, float thickness = 4.f);
    void drawHoverPreview();
    void drawWinHighlight(const Board& board);
    void drawInfoPanel(const Board& board);
    void drawButton(float x, float y, float w, float h,
                    const std::string& label, const sf::Color& bg);
    void drawResultOverlay();

    // ── Hit-testing ─────────────────────────────────────────────────────
    /// Convert a pixel position to a board cell. Returns {-1,-1} if outside.
    std::pair<int, int> pixelToCell(sf::Vector2i pixel) const;

    /// Check if a pixel position is inside the "New Game" button.
    bool isInsideNewGameButton(sf::Vector2i pixel) const;

    /// Infer the current player from board state (count X vs O marks).
    void inferCurrentPlayer(const Board& board);

    // "New Game" button rectangle (set during drawInfoPanel).
    sf::FloatRect newGameBtnRect_;
};
