g++ -std=c++17 main.cpp gfx\*.c gfx\*.h -I.\SDL2\x86_64-w64-mingw32\include -L.\SDL2\x86_64-w64-mingw32\lib -Wall -lmingw32 -lSDL2main -lSDL2 -I.\gfx -o maincc
.\maincc.exe
