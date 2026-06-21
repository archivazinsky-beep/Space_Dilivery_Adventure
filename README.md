# Space_Dilivery_Adventure
Simple 2D game

Wymagania:
SFML 3 ,
clang++

Budowanie:
Zapisz plik jako Space_Dilivery_Adventure.cpp
clang++ -std=c++17 Space_Dilivery_Adventure.cpp \
  -I/opt/homebrew/include \
  -L/opt/homebrew/lib \
  -Wl,-rpath,/opt/homebrew/lib \
  -lsfml-graphics -lsfml-window -lsfml-system \
  -o Space_Delivery

Controls:
Kierunek lotu za pomocą myszy , 
Lewy przycisk dla przyspieszenia , 
'R' - Restart , 
'Esc' - Escape , 
