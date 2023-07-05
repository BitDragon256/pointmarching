#!/bin/bash
g++ main.cpp gfx/*.c gfx/*.h -lSDL2 -w -I./gfx -o maincc
./maincc
