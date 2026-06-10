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

static sf::Font loadSystemFont();
static void drawSetupButton(sf::RenderWindow& win, sf::Font& font,
                             float x, float y, float w, float h,
                             const std::string& label,
                             bool selected);
static bool insideRect(sf::Vector2i pt, float x, float y, float w, float h);

struct SetupResult {
    int         boardSize  = 3;
    int         winLength  = 3;
    std::string mode       = "pve";
};

static bool showSetupScreen(sf::RenderWindow& window, sf::Font& font,
                             SetupResult& result)
{
    const sf::Color kBg      {0x0a, 0x0a, 0x0a};
    const sf::Color kPanel   {0x14, 0x14, 0x14};
    const sf::Color kText    {0xff, 0xff, 0xff};
    const sf::Color kDimText {0x88, 0x88, 0x88};

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                return false;
            }
            if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                const auto mx = mb->position;

                // Board size +/- (centered buttons)
                if (insideRect(mx, 420, 200, 45, 40)) {
                    result.boardSize = std::max(3, result.boardSize - 1);
                    result.winLength = std::clamp(result.winLength, 3, result.boardSize);
                }
                if (insideRect(mx, 570, 200, 45, 40)) {
                    result.boardSize = std::min(15, result.boardSize + 1);
                }

                // Win length +/-
                if (insideRect(mx, 420, 270, 45, 40)) {
                    result.winLength = std::max(3, result.winLength - 1);
                }
                if (insideRect(mx, 570, 270, 45, 40)) {
                    result.winLength = std::min(result.boardSize, result.winLength + 1);
                }

                // Game mode buttons
                if (insideRect(mx, 400, 345, 130, 42)) result.mode = "pvp";
                if (insideRect(mx, 550, 345, 130, 42)) result.mode = "pve";

                // Start Game
                if (insideRect(mx, 380, 440, 290, 55)) {
                    return true;
                }
            }
        }

        window.clear(kBg);

        // Panel background
        {
            sf::RectangleShape panel(sf::Vector2f{570.f, 420.f});
            panel.setPosition(sf::Vector2f{240.f, 100.f});
            panel.setFillColor(kPanel);
            window.draw(panel);
        }

        // Title
        {
            sf::Text title(font, "Tic-Tac-Toe", 36);
            title.setFillColor(kText);
            const float tw = title.getLocalBounds().size.x;
            title.setPosition(sf::Vector2f{(1050.f - tw) / 2.f, 120.f});
            window.draw(title);
        }

        // Board size
        {
            sf::Text label(font, "Board Size", 18);
            label.setFillColor(kDimText);
            label.setPosition(sf::Vector2f{270.f, 210.f});
            window.draw(label);

            drawSetupButton(window, font, 420, 200, 45, 40, "-", false);

            sf::Text val(font, std::to_string(result.boardSize) + "x" +
                               std::to_string(result.boardSize), 20);
            val.setFillColor(kText);
            const float vw = val.getLocalBounds().size.x;
            val.setPosition(sf::Vector2f{520.f - vw / 2.f, 208.f});
            window.draw(val);

            drawSetupButton(window, font, 570, 200, 45, 40, "+", false);
        }

        // Win condition
        {
            sf::Text label(font, "Win Condition", 18);
            label.setFillColor(kDimText);
            label.setPosition(sf::Vector2f{270.f, 280.f});
            window.draw(label);

            drawSetupButton(window, font, 420, 270, 45, 40, "-", false);

            sf::Text val(font, std::to_string(result.winLength) + " in a row", 20);
            val.setFillColor(kText);
            const float vw = val.getLocalBounds().size.x;
            val.setPosition(sf::Vector2f{520.f - vw / 2.f, 278.f});
            window.draw(val);

            drawSetupButton(window, font, 570, 270, 45, 40, "+", false);
        }

        // Game mode
        {
            sf::Text label(font, "Game Mode", 18);
            label.setFillColor(kDimText);
            label.setPosition(sf::Vector2f{270.f, 350.f});
            window.draw(label);

            drawSetupButton(window, font, 400, 345, 130, 42, "PvP",
                            result.mode == "pvp");
            drawSetupButton(window, font, 550, 345, 130, 42, "PvE",
                            result.mode == "pve");
        }

        // Start Game button (white bg, black text)
        {
            sf::RectangleShape btn(sf::Vector2f{290.f, 55.f});
            btn.setPosition(sf::Vector2f{380.f, 440.f});
            btn.setFillColor(sf::Color::White);
            window.draw(btn);

            sf::Text btnText(font, "Start Game", 22);
            btnText.setFillColor(sf::Color::Black);
            const float tw = btnText.getLocalBounds().size.x;
            btnText.setPosition(sf::Vector2f{380.f + (290.f - tw) / 2.f, 450.f});
            window.draw(btnText);
        }

        window.display();
    }
    return false;
}

int main()
{
    sf::Font font = loadSystemFont();
    ttt::Game setupGame;
    SFMLRenderer renderer(setupGame, font);
    sf::RenderWindow& window = renderer.getWindow();

    bool running = true;
    while (running) {
        SetupResult setup;
        if (!showSetupScreen(window, font, setup)) {
            return 0;
        }

        renderer.resetScores();
        bool backToMenu = false;

        while (running && !renderer.shouldClose() && !backToMenu) {
            ttt::GameConfig config{setup.boardSize, setup.winLength};
            ttt::Game game(config);

            renderer.setGame(game);
            renderer.showResult("");

            // Create players — AI always depth 9
            std::unique_ptr<ttt::Player> playerX, playerO;

            if (setup.mode == "pvp") {
                playerX = std::make_unique<ttt::HumanPlayer>();
                playerX->setMark(ttt::Cell::X);
                playerO = std::make_unique<ttt::HumanPlayer>();
                playerO->setMark(ttt::Cell::O);
            } else {
                // PvE: human is X, AI is O
                playerX = std::make_unique<ttt::HumanPlayer>();
                playerX->setMark(ttt::Cell::X);
                auto ai = std::make_unique<ttt::AIPlayer>(9);
                ai->setMark(ttt::Cell::O);
                playerO = std::move(ai);
            }

            // Game loop
            while (!game.isOver() && !renderer.shouldClose()) {
                renderer.render();

                ttt::Cell current = game.currentPlayer();
                bool isX = (current == ttt::Cell::X);
                ttt::Player& player = isX ? *playerX : *playerO;

                if (player.isHuman()) {
                    ttt::Move move = renderer.getHumanMove();
                    if (renderer.shouldClose()) break;
                    if (renderer.wasMenuRequested()) { backToMenu = true; break; }
                    if (renderer.wasNewGameRequested()) break;
                    if (move.isValid()) {
                        game.play(move.row, move.col);
                    }
                } else {
                    while (const auto event = window.pollEvent()) {
                        if (event->is<sf::Event::Closed>()) {
                            window.close();
                            break;
                        }
                        if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                            if (renderer.isInsideMenuButton(mb->position)) {
                                backToMenu = true;
                            }
                        }
                    }
                    if (renderer.shouldClose() || backToMenu) break;

                    ttt::Move move = player.chooseMove(game.board(), game.rules());
                    if (move.isValid()) {
                        game.play(move.row, move.col);
                    }
                }
            }

            if (renderer.shouldClose()) { running = false; break; }
            if (backToMenu) break;
            if (renderer.wasNewGameRequested()) continue;

            // Game finished
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

            sf::Clock overlayClock;
            bool overlayVisible = true;
            bool newGameClicked = false;

            while (!renderer.shouldClose() && !newGameClicked && !backToMenu) {
                while (const auto event = window.pollEvent()) {
                    if (event->is<sf::Event::Closed>()) {
                        window.close();
                        running = false;
                        break;
                    }
                    if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                        if (overlayVisible) {
                            overlayVisible = false;
                            renderer.showResult("");
                        } else {
                            if (renderer.isInsideNewGameButton(mb->position)) {
                                newGameClicked = true;
                                break;
                            }
                            if (renderer.isInsideMenuButton(mb->position)) {
                                backToMenu = true;
                                break;
                            }
                        }
                    }
                }
                if (!running || newGameClicked || backToMenu) break;
                if (overlayVisible && overlayClock.getElapsedTime().asSeconds() >= 3.f) {
                    overlayVisible = false;
                    renderer.showResult("");
                }
                renderer.render();
            }
        }
        if (!running || !window.isOpen()) break;
    }
    return 0;
}

static sf::Font loadSystemFont() {
    const std::vector<std::string> fontPaths = {
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/tahoma.ttf",
        "C:/Windows/Fonts/verdana.ttf",
    };
    for (const auto& path : fontPaths) {
        try { return sf::Font(path); } catch (...) {}
    }
    std::cerr << "Error: could not load any system font.\n";
    std::exit(1);
}

static void drawSetupButton(sf::RenderWindow& win, sf::Font& font,
                             float x, float y, float w, float h,
                             const std::string& label, bool selected)
{
    sf::Color bg = selected ? sf::Color::White : sf::Color{0x25, 0x25, 0x25};
    sf::Color fg = selected ? sf::Color::Black : sf::Color::White;

    sf::RectangleShape btn(sf::Vector2f{w, h});
    btn.setPosition(sf::Vector2f{x, y});
    btn.setFillColor(bg);
    if (!selected) {
        btn.setOutlineColor(sf::Color{0x33, 0x33, 0x33});
        btn.setOutlineThickness(1.f);
    }
    win.draw(btn);

    sf::Text text(font, label, 16);
    text.setFillColor(fg);
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
