/**
 * @file main.cpp
 * @brief Entry point for the Tic-Tac-Toe application.
 *
 * Supports two modes:
 *   1. **Console mode** (--console) — text-based, runs in the terminal.
 *   2. **GUI mode** (default)      — SFML 3.0 graphical interface with a
 *      setup screen for configuring board size, win condition, difficulty,
 *      and game mode before starting the match.
 *
 * Command-line options:
 *   --console              Use the console renderer instead of the GUI.
 *   --size N               Board dimension (3–15, default 3).
 *   --win K                Marks in a row to win (3–size, default = size if ≤ 5, else 5).
 *   --difficulty easy|medium|hard   AI difficulty (default medium).
 *   --mode pvp|pve|eve     Game mode (default pve).
 */

#include "Board.h"
#include "Game.h"
#include "Player.h"
#include "HumanPlayer.h"
#include "AIPlayer.h"
#include "Renderer.h"
#include "ConsoleRenderer.h"
#include "SFMLRenderer.h"

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// Forward declarations
// ═══════════════════════════════════════════════════════════════════════════

/// Run the game in console (text) mode.
static void runConsoleMode(int size, int winCond, Difficulty diff,
                           const std::string& mode);

/// Run the game in GUI (SFML) mode — includes setup screen + game loop.
static void runGUIMode(int size, int winCond, Difficulty diff,
                       const std::string& mode);

/// Parse a difficulty string ("easy", "medium", "hard") into the enum.
static Difficulty parseDifficulty(const std::string& s);

/// Try to load a system font; returns a valid sf::Font or exits with error.
static sf::Font loadSystemFont();

// ═══════════════════════════════════════════════════════════════════════════
// Setup-screen helpers (GUI)
// ═══════════════════════════════════════════════════════════════════════════

/// Draw a small clickable button; returns true if the mouse is inside it.
static void drawSetupButton(sf::RenderWindow& win, sf::Font& font,
                            float x, float y, float w, float h,
                            const std::string& label,
                            bool selected, const sf::Color& selColor);

/// Check if a point is inside a rectangle.
static bool insideRect(sf::Vector2i pt, float x, float y, float w, float h);

// ═══════════════════════════════════════════════════════════════════════════
// main
// ═══════════════════════════════════════════════════════════════════════════

int main(int argc, char* argv[])
{
    // ── Default settings ────────────────────────────────────────────
    bool        consoleMode = false;
    int         size        = 3;
    int         winCond     = 0;   // 0 means "auto-choose later"
    std::string diffStr     = "medium";
    std::string mode        = "pve";

    // ── Parse command-line arguments ────────────────────────────────
    const std::vector<std::string> args(argv + 1, argv + argc);

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--console") {
            consoleMode = true;
        } else if (args[i] == "--size" && i + 1 < args.size()) {
            size = std::clamp(std::stoi(args[++i]), 3, 15);
        } else if (args[i] == "--win" && i + 1 < args.size()) {
            winCond = std::stoi(args[++i]);
        } else if (args[i] == "--difficulty" && i + 1 < args.size()) {
            diffStr = args[++i];
        } else if (args[i] == "--mode" && i + 1 < args.size()) {
            mode = args[++i];
        }
    }

    // Auto-choose win condition if not explicitly set.
    if (winCond <= 0) {
        winCond = (size <= 5) ? size : 5;
    }
    winCond = std::clamp(winCond, 3, size);

    const Difficulty diff = parseDifficulty(diffStr);

    // ── Launch the appropriate mode ─────────────────────────────────
    if (consoleMode) {
        runConsoleMode(size, winCond, diff, mode);
    } else {
        runGUIMode(size, winCond, diff, mode);
    }

    return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// Console mode
// ═══════════════════════════════════════════════════════════════════════════

static void runConsoleMode(int size, int winCond, Difficulty diff,
                           const std::string& mode)
{
    Board board(size, winCond);
    ConsoleRenderer renderer;

    // Create players based on mode.
    std::unique_ptr<Player> p1, p2;

    if (mode == "pvp") {
        p1 = std::make_unique<HumanPlayer>('X');
        p2 = std::make_unique<HumanPlayer>('O');
    } else if (mode == "eve") {
        p1 = std::make_unique<AIPlayer>('X', 'O', diff);
        p2 = std::make_unique<AIPlayer>('O', 'X', diff);
    } else {  // pve (default)
        p1 = std::make_unique<HumanPlayer>('X');
        p2 = std::make_unique<AIPlayer>('O', 'X', diff);
    }

    Game game(board, *p1, *p2, renderer);

    std::cout << "=== Tic-Tac-Toe ===\n"
              << "Board: " << size << "x" << size
              << "  |  Win: " << winCond << " in a row\n\n";

    game.run();
}

// ═══════════════════════════════════════════════════════════════════════════
// GUI mode — setup screen
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Displays an interactive setup screen where the user can configure
 *        board size, win condition, difficulty, and game mode.
 *
 * @param[in,out] window  The SFML render window.
 * @param[in]     font    Font for text rendering.
 * @param[out]    size    Chosen board size.
 * @param[out]    winCond Chosen win condition.
 * @param[out]    diff    Chosen difficulty.
 * @param[out]    mode    Chosen game mode string.
 * @return true if the user clicked "Start Game", false if window was closed.
 */
static bool showSetupScreen(sf::RenderWindow& window, sf::Font& font,
                             int& size, int& winCond,
                             Difficulty& diff, std::string& mode)
{
    // Colour constants.
    const sf::Color kBg      {0x1a, 0x1a, 0x2e};
    const sf::Color kPanel   {0x16, 0x21, 0x3e};
    const sf::Color kAccent  {0xe9, 0x45, 0x60};
    const sf::Color kBlue    {0x4d, 0xa6, 0xff};
    const sf::Color kText    {0xea, 0xea, 0xea};
    const sf::Color kDimText {160, 160, 160};
    const sf::Color kBtnBg   {0x2d, 0x46, 0x6b};
    const sf::Color kSelBg   {0xe9, 0x45, 0x60};

    while (window.isOpen()) {
        // ── Event handling ──────────────────────────────────────────
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                return false;
            }

            if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                const auto mx = mb->position;

                // Board size +/- buttons.
                if (insideRect(mx, 390, 195, 40, 36)) {          // "-"
                    size = std::max(3, size - 1);
                    winCond = std::clamp(winCond, 3, size);
                }
                if (insideRect(mx, 500, 195, 40, 36)) {          // "+"
                    size = std::min(15, size + 1);
                }

                // Win condition +/- buttons.
                if (insideRect(mx, 390, 255, 40, 36)) {          // "-"
                    winCond = std::max(3, winCond - 1);
                }
                if (insideRect(mx, 500, 255, 40, 36)) {          // "+"
                    winCond = std::min(size, winCond + 1);
                }

                // Difficulty buttons.
                if (insideRect(mx, 320, 335, 90, 36))  diff = Difficulty::EASY;
                if (insideRect(mx, 420, 335, 90, 36))  diff = Difficulty::MEDIUM;
                if (insideRect(mx, 520, 335, 90, 36))  diff = Difficulty::HARD;

                // Game mode buttons.
                if (insideRect(mx, 300, 415, 100, 36)) mode = "pvp";
                if (insideRect(mx, 410, 415, 100, 36)) mode = "pve";
                if (insideRect(mx, 520, 415, 100, 36)) mode = "eve";

                // Start Game button.
                if (insideRect(mx, 330, 500, 240, 50)) {
                    return true;
                }
            }
        }

        // ── Drawing ─────────────────────────────────────────────────
        window.clear(kBg);

        // Centred panel background.
        {
            sf::RectangleShape panel(sf::Vector2f{500.f, 480.f});
            panel.setPosition(sf::Vector2f{200.f, 100.f});
            panel.setFillColor(kPanel);
            window.draw(panel);
        }

        // Title.
        {
            sf::Text title(font, "Tic-Tac-Toe Setup", 32);
            title.setFillColor(kText);
            const float tw = title.getLocalBounds().size.x;
            title.setPosition(sf::Vector2f{(900.f - tw) / 2.f, 120.f});
            window.draw(title);
        }

        // ── Board size ──────────────────────────────────────────────
        {
            sf::Text label(font, "Board Size", 18);
            label.setFillColor(kDimText);
            label.setPosition(sf::Vector2f{230.f, 200.f});
            window.draw(label);

            drawSetupButton(window, font, 390, 195, 40, 36, "-", false, kBtnBg);

            sf::Text val(font, std::to_string(size) + "x" + std::to_string(size), 18);
            val.setFillColor(kText);
            val.setPosition(sf::Vector2f{442.f, 200.f});
            window.draw(val);

            drawSetupButton(window, font, 500, 195, 40, 36, "+", false, kBtnBg);
        }

        // ── Win condition ───────────────────────────────────────────
        {
            sf::Text label(font, "Win Condition", 18);
            label.setFillColor(kDimText);
            label.setPosition(sf::Vector2f{230.f, 260.f});
            window.draw(label);

            drawSetupButton(window, font, 390, 255, 40, 36, "-", false, kBtnBg);

            sf::Text val(font, std::to_string(winCond) + " in a row", 18);
            val.setFillColor(kText);
            val.setPosition(sf::Vector2f{440.f, 260.f});
            window.draw(val);

            drawSetupButton(window, font, 500, 255, 40, 36, "+", false, kBtnBg);
        }

        // ── Difficulty ──────────────────────────────────────────────
        {
            sf::Text label(font, "Difficulty", 18);
            label.setFillColor(kDimText);
            label.setPosition(sf::Vector2f{230.f, 340.f});
            window.draw(label);

            drawSetupButton(window, font, 320, 335, 90, 36, "Easy",
                            diff == Difficulty::EASY, kSelBg);
            drawSetupButton(window, font, 420, 335, 90, 36, "Medium",
                            diff == Difficulty::MEDIUM, kSelBg);
            drawSetupButton(window, font, 520, 335, 90, 36, "Hard",
                            diff == Difficulty::HARD, kSelBg);
        }

        // ── Game mode ───────────────────────────────────────────────
        {
            sf::Text label(font, "Game Mode", 18);
            label.setFillColor(kDimText);
            label.setPosition(sf::Vector2f{230.f, 420.f});
            window.draw(label);

            drawSetupButton(window, font, 300, 415, 100, 36, "PvP",
                            mode == "pvp", kBlue);
            drawSetupButton(window, font, 410, 415, 100, 36, "PvE",
                            mode == "pve", kBlue);
            drawSetupButton(window, font, 520, 415, 100, 36, "AI vs AI",
                            mode == "eve", kBlue);
        }

        // ── Start Game button ───────────────────────────────────────
        {
            sf::RectangleShape btn(sf::Vector2f{240.f, 50.f});
            btn.setPosition(sf::Vector2f{330.f, 500.f});
            btn.setFillColor(kAccent);
            window.draw(btn);

            sf::Text btnText(font, "Start Game", 22);
            btnText.setFillColor(sf::Color::White);
            const float tw = btnText.getLocalBounds().size.x;
            btnText.setPosition(sf::Vector2f{330.f + (240.f - tw) / 2.f, 508.f});
            window.draw(btnText);
        }

        window.display();
    }

    return false;
}

// ═══════════════════════════════════════════════════════════════════════════
// GUI mode — main game loop (with play-again support)
// ═══════════════════════════════════════════════════════════════════════════

static void runGUIMode(int size, int winCond, Difficulty diff,
                       const std::string& mode)
{
    sf::Font font = loadSystemFont();

    // Create the SFML renderer (this also opens the window).
    SFMLRenderer renderer(size, font);
    sf::RenderWindow& window = renderer.getWindow();

    // ── Setup screen ────────────────────────────────────────────────
    std::string gameMode = mode;
    if (!showSetupScreen(window, font, size, winCond, diff, gameMode)) {
        return;   // Window was closed during setup.
    }

    // The setup screen may have changed the board size, so we need to
    // re-create the renderer with the correct board size.
    // (Close the old window and make a new renderer.)
    window.close();

    SFMLRenderer gameRenderer(size, font);

    // ── Game loop (with play-again) ─────────────────────────────────
    bool running = true;
    while (running && !gameRenderer.shouldClose()) {
        Board board(size, winCond);

        // Create players based on mode.
        std::unique_ptr<Player> p1, p2;

        if (gameMode == "pvp") {
            p1 = std::make_unique<HumanPlayer>('X');
            p2 = std::make_unique<HumanPlayer>('O');
        } else if (gameMode == "eve") {
            p1 = std::make_unique<AIPlayer>('X', 'O', diff);
            p2 = std::make_unique<AIPlayer>('O', 'X', diff);
        } else {  // pve (default)
            p1 = std::make_unique<HumanPlayer>('X');
            p2 = std::make_unique<AIPlayer>('O', 'X', diff);
        }

        Game game(board, *p1, *p2, gameRenderer);

        // Update the renderer's current player indicator.
        gameRenderer.setCurrentPlayer('X');

        // Run the game to completion.
        game.run();

        if (gameRenderer.shouldClose()) break;

        // Update scores.
        switch (game.getState()) {
            case GameState::X_WON: gameRenderer.addScore('X'); break;
            case GameState::O_WON: gameRenderer.addScore('O'); break;
            case GameState::DRAW:  gameRenderer.addScore('D'); break;
            default: break;
        }

        // Render the final state with the result overlay.
        gameRenderer.render(board);

        // Wait for the user to click "New Game" or close the window.
        while (!gameRenderer.shouldClose()) {
            while (const auto event = gameRenderer.getWindow().pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    gameRenderer.getWindow().close();
                    running = false;
                    break;
                }

                if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                    // Check if "New Game" was clicked.
                    // We re-render to keep the button responsive.
                    (void)mb;  // processed below via wasNewGameRequested
                }
            }

            if (!running) break;

            // Re-render to keep the display alive.
            gameRenderer.render(board);

            if (gameRenderer.wasNewGameRequested()) {
                // Clear the result overlay and start a new game.
                gameRenderer.showResult("");  // clear overlay
                break;
            }
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Utility functions
// ═══════════════════════════════════════════════════════════════════════════

static Difficulty parseDifficulty(const std::string& s) {
    if (s == "easy")   return Difficulty::EASY;
    if (s == "hard")   return Difficulty::HARD;
    return Difficulty::MEDIUM;   // default
}

static sf::Font loadSystemFont() {
    // Try common Windows system font paths.
    const std::vector<std::string> fontPaths = {
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/tahoma.ttf",
        "C:/Windows/Fonts/verdana.ttf",
    };

    for (const auto& path : fontPaths) {
        try {
            return sf::Font(path);
        } catch (...) {
            // Try the next path.
        }
    }

    std::cerr << "Error: could not load any system font.\n"
              << "Tried: segoeui.ttf, arial.ttf, tahoma.ttf, verdana.ttf\n";
    std::exit(1);
}

static void drawSetupButton(sf::RenderWindow& win, sf::Font& font,
                             float x, float y, float w, float h,
                             const std::string& label,
                             bool selected, const sf::Color& selColor)
{
    const sf::Color bg = selected ? selColor : sf::Color{0x2d, 0x46, 0x6b};

    sf::RectangleShape btn(sf::Vector2f{w, h});
    btn.setPosition(sf::Vector2f{x, y});
    btn.setFillColor(bg);
    win.draw(btn);

    sf::Text text(font, label, 16);
    text.setFillColor(sf::Color{0xea, 0xea, 0xea});
    const float tw = text.getLocalBounds().size.x;
    const float th = text.getLocalBounds().size.y;
    text.setPosition(sf::Vector2f{x + (w - tw) / 2.f,
                                  y + (h - th) / 2.f - 3.f});
    win.draw(text);
}

static bool insideRect(sf::Vector2i pt, float x, float y, float w, float h) {
    const auto px = static_cast<float>(pt.x);
    const auto py = static_cast<float>(pt.y);
    return px >= x && px <= x + w && py >= y && py <= y + h;
}
