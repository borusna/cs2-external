@echo off
g++ main.cpp -Ofast -o a.exe -lgdi32 -luser32 -lwinmm -ldwmapi -s -static -static-libgcc -DDRIVER
rem g++ main.cpp -Ofast -o a.dll -lgdi32 -luser32 -lwinmm -s -static -static-libgcc -shared -DDLL
