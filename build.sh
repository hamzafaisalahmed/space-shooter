#!/bin/bash
# ============================================
# Direct build script (no CMake needed)
# Run from the SpaceShooter directory in MSYS2 MINGW64
# ============================================

echo "=== Building Space Shooter ==="

# Copy font if not present
if [ ! -f "assets/game_font.ttf" ]; then
    echo "Copying Arial font..."
    cp /c/Windows/Fonts/arial.ttf assets/game_font.ttf 2>/dev/null
fi

# Compile (requires SFML installed via pacman)
g++ -std=c++17 -Wall -O2 \
    src/main.cpp \
    -o SpaceShooter.exe \
    -lsfml-graphics -lsfml-window -lsfml-system

if [ $? -eq 0 ]; then
    echo "=== Build successful! ==="
    echo "Run with: ./SpaceShooter.exe"
else
    echo "=== Build FAILED ==="
    echo "Make sure SFML is installed: pacman -S mingw-w64-x86_64-sfml"
fi
