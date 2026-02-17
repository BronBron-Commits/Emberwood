// CharacterModule.h
// Defines a new character type for Emberwood
#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

struct CharacterColor {
    Uint8 r, g, b;
};

struct Character {
    float x, y;
    float vx, vy;
    int health;
    int maxHealth;
    int energy;
    int maxEnergy;
    CharacterColor mainColor;
    CharacterColor accentColor;
    std::string name;
    int facing = 0; // 0=right, 1=left, 2=up, 3=down
};

// Creates a new character with default values
Character createNewCharacter(const std::string& name, float x, float y, CharacterColor main, CharacterColor accent);

// Renders the character (placeholder, to be implemented in .cpp)
void renderCharacter(SDL_Renderer* renderer, const Character& character, TTF_Font* font);

// Renders a knight with a scythe
void renderKnightWithScythe(SDL_Renderer* renderer, const Character& character, TTF_Font* font);
