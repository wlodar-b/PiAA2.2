cmake_minimum_required(VERSION 3.16)
project(TicTacToe LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ── Core library ──────────────────────────────────────────────────────────
add_library(ttt_core STATIC
    src/Board.cpp
    src/GameRules.cpp
    src/Game.cpp
    src/AIPlayer.cpp
)
target_include_directories(ttt_core PUBLIC include)
target_compile_features(ttt_core PUBLIC cxx_std_17)

# ── Console executable ───────────────────────────────────────────────────
add_executable(tictactoe_console src/console_main.cpp)
target_link_libraries(tictactoe_console PRIVATE ttt_core)

# ── GUI executable (SFML) ───────────────────────────────────────────────
find_package(SFML 3 COMPONENTS Graphics REQUIRED)

add_executable(tictactoe_gui
    src/main.cpp
    src/SFMLRenderer.cpp
)
target_include_directories(tictactoe_gui PRIVATE include)
target_link_libraries(tictactoe_gui PRIVATE ttt_core SFML::Graphics)
