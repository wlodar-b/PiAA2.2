# Projekt – Kółko i krzyżyk z AI (MinMax)

## Opis projektu

Projekt to gra w kółko i krzyżyk z przeciwnikiem sterowanym przez sztuczną inteligencję. W przeciwieństwie do zwykłej gry 3x3, możemy tu ustalić **własny rozmiar planszy** (np. od 3x3 do 15x15) oraz **liczbę znaków w rzędzie** potrzebną do wygranej.

AI wykorzystuje algorytm **MinMax z odcinaniem alfa-beta**. Logika gry jest w pełni oddzielona od interfejsu graficznego. Interfejs graficzny został napisany przy użyciu biblioteki **SFML 3**. 

## Struktura projektu

Projekt składa się głównie ze standardowego kodu C++ z bardzo minimalistycznym plikiem CMake.

- `CMakeLists.txt` – konfiguracja całego projektu (zajmuje tylko kilkadziesiąt linijek, łączy SFML z kodem gry).
- `src/` – kod w czystym C++.
  - `Board.cpp` / `Board.hpp` – stan planszy.
  - `GameRules.cpp` / `GameRules.hpp` – wykrywanie wygranej.
  - `Game.cpp` / `Game.hpp` – łączy zasady z planszą i zarządza turami.
  - `AIPlayer.cpp` / `AIPlayer.hpp` – komputer (MinMax i heurystyka).
  - `SFMLRenderer.cpp` – plik rysujący grafikę.
  - `main.cpp` / `console_main.cpp` – uruchamianie aplikacji.

## Jak działa AI

### MinMax z odcinaniem alfa-beta
Klasyczny algorytm szuka najlepszego ruchu dla komputera, sprawdzając możliwe odpowiedzi gracza. Ponieważ dla plansz większych niż 3x3 wszystkich kombinacji jest za dużo, zastosowano **odcinanie alfa-beta**, które pomija bezsensowne ruchy, przyspieszając myślenie.

### Heurystyka i głębokość
Dla bardzo dużych plansz komputer nie da rady sprawdzić gry aż do samego końca. Z tego powodu zatrzymuje się na pewnej głębokości i ocenia planszę **heurystycznie** (np. dostaje punkty za 2 lub 3 swoje znaki w rzędzie, a traci punkty, gdy widzi znaki gracza).

### Optymalizacja ruchów
Aby komputer nie tracił czasu na sprawdzanie ruchów na drugim końcu pustej planszy, analizuje tylko te pola, które sąsiadują z już postawionymi znakami.

## Pobieranie i uruchamianie

Projekt kompilujemy za pomocą systemu **CMake**. Oczekiwany kompilator to `g++` obsługujący standard C++17 oraz zainstalowana biblioteka SFML 3 (najlepiej środowisko MSYS2 MinGW/UCRT64).

**1. Pobranie projektu z GitHuba:**
```bash
git clone <TUTAJ_WKLEJ_LINK_DO_SWOJEGO_REPOZYTORIUM>
cd <NAZWA_PABRANEGO_FOLDERU>
```

**2. Budowanie projektu:**
Będąc w folderze projektu, wygeneruj pliki kompilacji i zbuduj aplikację:
```bash
cmake -B build
cmake --build build -j8
```

**3. Uruchomienie gry graficznej:**
Po pomyślnym zbudowaniu wystarczy odpalić plik wykonywalny:
```bash
.\build\bin\tictactoe_gui.exe
```

## Wykorzystane technologie
- **C++17** (tylko standardowe biblioteki)
- **SFML 3** (do wyświetlania okna, przycisków i planszy)
- **CMake** (minimalny plik konfiguracyjny)
