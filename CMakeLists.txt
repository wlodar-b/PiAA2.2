cmake_minimum_required(VERSION 3.16)
project(TicTacToe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── SFML 3.0 ─────────────────────────────────────────────────────────────
find_package(SFML 3 COMPONENTS Graphics REQUIRED)

# ── Executable ───────────────────────────────────────────────────────────
add_executable(TicTacToe
    src/main.cpp
    src/Board.cpp
    src/Game.cpp
    src/HumanPlayer.cpp
    src/AIPlayer.cpp
    src/ConsoleRenderer.cpp
    src/SFMLRenderer.cpp
)

target_include_directories(TicTacToe PRIVATE include)
target_link_libraries(TicTacToe PRIVATE SFML::Graphics)
