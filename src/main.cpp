/**
 * @file main.cpp
 * @brief GUI entry point for the Tic-Tac-Toe application (SFML 3.0).
 *
 * Shows a setup screen for board size, win length, difficulty, game mode,
 * and player sign choice. Then runs the game loop with play-again support.
 */

#include "Game.hpp"
#include "AIPlayer.hpp"
#include "HumanPlayer.hpp"
#include "SFMLRenderer.hpp"

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

static sf::Font loadSystemFont();

static void drawSetupButton(sf::RenderWindow& win, sf::Font& font,
                             float x, float y, float w, float h,
                             const std::string& label,
                             bool selected, const sf::Color& selColor);

static bool insideRect(sf::Vector2i pt, float x, float y, float w, float h);

// ═══════════════════════════════════════════════════════════════════════════
// Difficulty → maxDepth mapping
// ═══════════════════════════════════════════════════════════════════════════

enum class Difficulty { Easy, Medium, Hard };

static int difficultyToDepth(Difficulty d) {
    switch (d) {
        case Difficulty::Easy:   return 1;
        case Difficulty::Medium: return 4;
        case Difficulty::Hard:   return 9;
    }
    return 4;
}

// ═══════════════════════════════════════════════════════════════════════════
// Setup screen
// ═══════════════════════════════════════════════════════════════════════════

struct SetupResult {
    int         boardSize  = 3;
    int         winLength  = 3;
    Difficulty  difficulty = Difficulty::Medium;
    std::string mode       = "pve";     // "pvp", "pve", "eve"
    ttt::Cell   playerSign = ttt::Cell::X;  // human plays as X or O
};

static bool showSetupScreen(sf::RenderWindow& window, sf::Font& font,
                             SetupResult& result)
{
    const sf::Color kBg      {0x1a, 0x1a, 0x2e};
    const sf::Color kPanel   {0x16, 0x21, 0x3e};
    const sf::Color kAccent  {0xe9, 0x45, 0x60};
    const sf::Color kBlue    {0x4d, 0xa6, 0xff};
    const sf::Color kText    {0xea, 0xea, 0xea};
    const sf::Color kDimText {160, 160, 160};
    const sf::Color kBtnBg   {0x2d, 0x46, 0x6b};
    const sf::Color kSelBg   {0xe9, 0x45, 0x60};

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                return false;
            }

            if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                const auto mx = mb->position;

                // Board size +/- buttons
                if (insideRect(mx, 390, 195, 40, 36)) {
                    result.boardSize = std::max(3, result.boardSize - 1);
                    result.winLength = std::clamp(result.winLength, 3, result.boardSize);
                }
                if (insideRect(mx, 500, 195, 40, 36)) {
                    result.boardSize = std::min(15, result.boardSize + 1);
                }

                // Win length +/- buttons
                if (insideRect(mx, 390, 255, 40, 36)) {
                    result.winLength = std::max(3, result.winLength - 1);
                }
                if (insideRect(mx, 500, 255, 40, 36)) {
                    result.winLength = std::min(result.boardSize, result.winLength + 1);
                }

                // Difficulty buttons
                if (insideRect(mx, 320, 335, 90, 36))  result.difficulty = Difficulty::Easy;
                if (insideRect(mx, 420, 335, 90, 36))  result.difficulty = Difficulty::Medium;
                if (insideRect(mx, 520, 335, 90, 36))  result.difficulty = Difficulty::Hard;

                // Game mode buttons
                if (insideRect(mx, 300, 415, 100, 36)) result.mode = "pvp";
                if (insideRect(mx, 410, 415, 100, 36)) result.mode = "pve";
                if (insideRect(mx, 520, 415, 100, 36)) result.mode = "eve";

                // Player sign buttons (only meaningful for PvE)
                if (insideRect(mx, 370, 490, 80, 36))  result.playerSign = ttt::Cell::X;
                if (insideRect(mx, 460, 490, 80, 36))  result.playerSign = ttt::Cell::O;

                // Start Game button
                if (insideRect(mx, 330, 550, 240, 50)) {
                    return true;
                }
            }
        }

        // ── Drawing ─────────────────────────────────────────────────
        window.clear(kBg);

        // Panel background
        {
            sf::RectangleShape panel(sf::Vector2f{500.f, 530.f});
            panel.setPosition(sf::Vector2f{200.f, 80.f});
            panel.setFillColor(kPanel);
            window.draw(panel);
        }

        // Title
        {
            sf::Text title(font, "Tic-Tac-Toe Setup", 32);
            title.setFillColor(kText);
            const float tw = title.getLocalBounds().size.x;
            title.setPosition(sf::Vector2f{(900.f - tw) / 2.f, 100.f});
            window.draw(title);
        }

        // ── Board size ──────────────────────────────────────────────
        {
            sf::Text label(font, "Board Size", 18);
            label.setFillColor(kDimText);
            label.setPosition(sf::Vector2f{230.f, 200.f});
            window.draw(label);

            drawSetupButton(window, font, 390, 195, 40, 36, "-", false, kBtnBg);

            sf::Text val(font, std::to_string(result.boardSize) + "x" +
                               std::to_string(result.boardSize), 18);
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

            sf::Text val(font, std::to_string(result.winLength) + " in a row", 18);
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
                            result.difficulty == Difficulty::Easy, kSelBg);
            drawSetupButton(window, font, 420, 335, 90, 36, "Medium",
                            result.difficulty == Difficulty::Medium, kSelBg);
            drawSetupButton(window, font, 520, 335, 90, 36, "Hard",
                            result.difficulty == Difficulty::Hard, kSelBg);
        }

        // ── Game mode ───────────────────────────────────────────────
        {
            sf::Text label(font, "Game Mode", 18);
            label.setFillColor(kDimText);
            label.setPosition(sf::Vector2f{230.f, 420.f});
            window.draw(label);

            drawSetupButton(window, font, 300, 415, 100, 36, "PvP",
                            result.mode == "pvp", kBlue);
            drawSetupButton(window, font, 410, 415, 100, 36, "PvE",
                            result.mode == "pve", kBlue);
            drawSetupButton(window, font, 520, 415, 100, 36, "AI vs AI",
                            result.mode == "eve", kBlue);
        }

        // ── Player sign (PvE only) ─────────────────────────────────
        {
            sf::Text label(font, "Play As", 18);
            label.setFillColor(result.mode == "pve" ? kDimText : sf::Color{80, 80, 80});
            label.setPosition(sf::Vector2f{230.f, 495.f});
            window.draw(label);

            const sf::Color xSelColor = result.mode == "pve" ? kAccent : sf::Color{60, 60, 80};
            const sf::Color oSelColor = result.mode == "pve" ? kBlue   : sf::Color{60, 60, 80};
            drawSetupButton(window, font, 370, 490, 80, 36, "X",
                            result.playerSign == ttt::Cell::X, xSelColor);
            drawSetupButton(window, font, 460, 490, 80, 36, "O",
                            result.playerSign == ttt::Cell::O, oSelColor);
        }

        // ── Start Game button ───────────────────────────────────────
        {
            sf::RectangleShape btn(sf::Vector2f{240.f, 50.f});
            btn.setPosition(sf::Vector2f{330.f, 550.f});
            btn.setFillColor(kAccent);
            window.draw(btn);

            sf::Text btnText(font, "Start Game", 22);
            btnText.setFillColor(sf::Color::White);
            const float tw = btnText.getLocalBounds().size.x;
            btnText.setPosition(sf::Vector2f{330.f + (240.f - tw) / 2.f, 558.f});
            window.draw(btnText);
        }

        window.display();
    }

    return false;
}

// ═══════════════════════════════════════════════════════════════════════════
// main
// ═══════════════════════════════════════════════════════════════════════════

int main()
{
    sf::Font font = loadSystemFont();

    // Create a temporary Game just for the setup-screen renderer.
    // We'll recreate it after setup.
    ttt::Game setupGame;

    // We need a window for the setup screen. Create a renderer (which opens
    // the window), then use its window for setup.
    SFMLRenderer renderer(setupGame, font);
    sf::RenderWindow& window = renderer.getWindow();

    SetupResult setup;
    if (!showSetupScreen(window, font, setup)) {
        return 0;  // Window closed during setup.
    }

    // ── Main game loop (with play-again) ────────────────────────────
    bool running = true;
    while (running && !renderer.shouldClose()) {
        ttt::GameConfig config{setup.boardSize, setup.winLength};
        ttt::Game game(config);

        renderer.setGame(game);
        renderer.showResult("");  // clear any previous overlay

        const int depth = difficultyToDepth(setup.difficulty);

        // Create players
        std::unique_ptr<ttt::Player> playerX, playerO;

        if (setup.mode == "pvp") {
            playerX = std::make_unique<ttt::HumanPlayer>();
            playerX->setMark(ttt::Cell::X);
            playerO = std::make_unique<ttt::HumanPlayer>();
            playerO->setMark(ttt::Cell::O);
        } else if (setup.mode == "eve") {
            auto ai1 = std::make_unique<ttt::AIPlayer>(depth);
            ai1->setMark(ttt::Cell::X);
            auto ai2 = std::make_unique<ttt::AIPlayer>(depth);
            ai2->setMark(ttt::Cell::O);
            playerX = std::move(ai1);
            playerO = std::move(ai2);
        } else {
            // PvE: player chooses their sign
            if (setup.playerSign == ttt::Cell::X) {
                playerX = std::make_unique<ttt::HumanPlayer>();
                playerX->setMark(ttt::Cell::X);
                auto ai = std::make_unique<ttt::AIPlayer>(depth);
                ai->setMark(ttt::Cell::O);
                playerO = std::move(ai);
            } else {
                // Player is O, AI is X (AI goes first)
                auto ai = std::make_unique<ttt::AIPlayer>(depth);
                ai->setMark(ttt::Cell::X);
                playerX = std::move(ai);
                playerO = std::make_unique<ttt::HumanPlayer>();
                playerO->setMark(ttt::Cell::O);
            }
        }

        // ── Game loop ───────────────────────────────────────────────
        while (!game.isOver() && !renderer.shouldClose()) {
            renderer.render();

            ttt::Cell current = game.currentPlayer();
            bool isX = (current == ttt::Cell::X);
            ttt::Player& player = isX ? *playerX : *playerO;

            if (player.isHuman()) {
                ttt::Move move = renderer.getHumanMove();
                if (renderer.shouldClose() || renderer.wasNewGameRequested()) break;
                if (move.isValid()) {
                    game.play(move.row, move.col);
                }
            } else {
                ttt::Move move = player.chooseMove(game.board(), game.rules());
                if (move.isValid()) {
                    game.play(move.row, move.col);
                }
                // Brief delay for AI vs AI to be watchable
                if (setup.mode == "eve") {
                    sf::sleep(sf::milliseconds(300));
                }
            }
        }

        if (renderer.shouldClose()) break;

        // Check if New Game was requested mid-game
        if (renderer.wasNewGameRequested()) {
            continue;
        }

        // Game finished — show result
        renderer.addScore(game.status());

        std::string resultMsg;
        switch (game.status()) {
            case ttt::GameStatus::XWins: resultMsg = "X Wins!"; break;
            case ttt::GameStatus::OWins: resultMsg = "O Wins!"; break;
            case ttt::GameStatus::Draw:  resultMsg = "It's a Draw!"; break;
            default: break;
        }
        renderer.showResult(resultMsg);
        renderer.render();

        // Wait for New Game or window close
        while (!renderer.shouldClose()) {
            while (const auto event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                    running = false;
                    break;
                }
                // Process mouse clicks for New Game button
                if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                    (void)mb;
                }
            }

            if (!running) break;

            renderer.render();

            if (renderer.wasNewGameRequested()) {
                renderer.showResult("");
                break;
            }
        }
    }

    return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// Utility functions
// ═══════════════════════════════════════════════════════════════════════════

static sf::Font loadSystemFont() {
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
