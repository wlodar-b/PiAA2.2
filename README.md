# Raport z Projektu: Rozszerzona Gra Kółko i Krzyżyk z AI

**Przedmiot:** Projektowanie i Analiza Algorytmów 2

## 1. Opis Projektu

Projekt to zaawansowana wersja klasycznej gry "Kółko i Krzyżyk" zrealizowana w języku C++ z interfejsem graficznym (biblioteka SFML 3). W przeciwieństwie do tradycyjnej wersji 3x3, aplikacja pozwala na rozgrywkę na planszach o rozmiarach od 3x3 do 15x15, ze zmienną długością linii niezbędną do wygranej (od 3 do rozmiaru planszy).  
Głównym założeniem algorytmicznym projektu była implementacja niepokonanego przeciwnika komputerowego (AI) operującego w warunkach silnie rozgałęzionego drzewa decyzyjnego dla dużych plansz.

## 2. Architektura Oprogramowania

Zgodnie z dobrymi praktykami programowania obiektowego, interfejs graficzny został całkowicie oddzielony od logiki gry, a całość zaimplementowano w dedykowanej przestrzeni nazw `ttt`.

*   **`Board` (Plansza):** Utrzymuje stan siatki gry w jednowymiarowej strukturze wektora, implementując szybkie operacje wstawiania i modyfikowania. Przechowuje zmienną `filled_` śledzącą liczbę zajętych komórek (czas $O(1)$ dla sprawdzenia remisu).
*   **`GameRules` (Zasady Gry):** Klasa agregująca reguły weryfikujące czy w zadanym układzie nastąpiła wygrana. Używa zoptymalizowanej heurystyki `hasWonAt()`, analizującej wyłącznie linie promieniście przechodzące przez nowo postawiony znak, co drastycznie redukuje liczbę sprawdzanych wierszy.
*   **`Game` (Kontroler):** Pętla decyzyjna zarządzająca turami, aktualizacją stanu i łączeniem obiektu *Zasad* z *Planszą*.
*   **`Player` (Klasa Bazowa):** Abstrakcja pozwalająca polimorficznie wstrzykiwać do silnika gry graczy sterowanych z zewnątrz (`HumanPlayer`) oraz sterowanych algorytmicznie (`AIPlayer`).
*   **`SFMLRenderer`:** Silnik interfejsu graficznego. Realizuje renderowanie elementów, responsywność okna, efekt najeżdżania na przyciski (`hover`) oraz walidację kliknięć, tłumacząc logikę koordynat graficznych na abstrakcyjną siatkę pól silnika.

## 3. Zastosowane Algorytmy (Logika AI)

Sztuczna Inteligencja oparta jest na algorytmie **Minimax z optymalizacją odcięć Alpha-Beta (Alpha-Beta Pruning)**, wspartym skomplikowaną **Heurystyką Oceny Planszy**.

### 3.1. Optymalizacja Odcięć (Alpha-Beta Pruning)
Klasyczny algorytm Minimax przeszukuje pełne drzewo możliwości. Dla planszy 3x3 ma to 9! ($362,880$) kombinacji. Dla plansz powyżej 4x4 pełne przeszukanie staje się niemożliwe. Zaimplementowano model Alpha-Beta Pruning, który odrzuca te poddrzewa decyzyjne, których wynik na pewno będzie gorszy niż ten, który zagwarantowaliśmy sobie w innych zbadanych gałęziach.  

Algorytm zawsze faworyzuje szybsze wygrane (poprzez stosowanie kary odległościowej od głębokości: `score = kWinScore - depth`) oraz wolniejsze porażki, przez co staje się idealnie wyważony. W wariancie 15x15 ze względu na ogromny współczynnik rozgałęzienia, wprowadzono limit głębokości przeszukiwania dostosowujący się dynamicznie (funkcja `effectiveDepth`).

### 3.2. Funkcja Heurystyczna
Ponieważ na dużych planszach drzewo zostaje przerwane, AI nigdy nie widzi "końca gry" podczas wyliczania ruchów. Konieczne było stworzenie bardzo celnej funkcji ewaluującej układ znaków na siatce: `AIPlayer::evaluate()`.

*   **Zasada działania:** System "przesuwnego okna" (o wielkości *winLength*). Okno przesuwa się w pionie, poziomie i obu skosach na całej planszy.
*   Jeżeli w danym wycinku "okna" obecne są pionki OBU graczy - jest to fragment *martwy* (nikt już tam nie utworzy zwycięskiej linii). Wycinek otrzymuje wynik $0$.
*   Jeżeli w oknie obecne są tylko pionki nasze, zwiększamy wynik dodatnio w sposób **wykładniczy** zależnie od zagęszczenia tych pionków (np. 1 pionek to $1$, 2 pionki to $10$, 3 pionki to $1000$).
*   Jeżeli w oknie obecne są tylko pionki przeciwnika - obniżamy wynik negatywnie w ten sam wykładniczy sposób.

Dzięki temu algorytm skupia się na wydłużaniu swoich sekwencji oraz brutalnym blokowaniu sekwencji przeciwnika (traktując je jako zagrożenie najwyższego stopnia).

### 3.3. Optymalizacja Generowania Pól (Candidate Moves)
Na starcie pustej dużej planszy, wyliczenie pełnego Minimaxa mijałoby się z celem. 
Wykorzystujemy tu własną optymalizację węzłów: funkcja zwracająca możliwe ruchy dla AI iteruje wybiórczo. Dla pustej planszy podaje ona automatycznie jedynie środek. Gdy gra wejdzie w fazę interakcji, algorytm nie bierze pod uwagę wszystkich pustych pól, a **wyłącznie puste pola mające przynajmniej jednego niepustego sąsiada w 8 kierunkach wokół nich**.

Zawęża to współczynnik rozgałęzienia (branching factor) drastycznie i przyspiesza wyliczenia rzędu od kliku do kilkuset razy w zależności od rozmiaru mapy.

## 4. Wykorzystane Narzędzia i Technologie
*   **Język:** C++17
*   **Kompilator:** MinGW (g++) używający pełnego restrykcyjnego zestawu ostrzeżeń (`-Wall`, `-Wextra`, `-Wpedantic`) w celu zapewnienia absolutnej stabilności pamięci i typów.
*   **GUI:** SFML 3.0.2 zrealizowane w natywnym akcelerowanym oknie systemowym.
*   **System budowy:** CMake używany jako warstwa uogólnienia do łatwej budowy na środowiskach MacOS, Linux oraz Windows.
