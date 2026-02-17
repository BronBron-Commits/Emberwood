// Ensure SDL2/SDL_ttf includes are first for type definitions
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <cmath>
#include "CharacterModule.h"

// Helper: draw a simple ellipse
static void drawEllipse(SDL_Renderer* renderer, int cx, int cy, int rx, int ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    for (int y = -ry; y <= ry; ++y) {
        for (int x = -rx; x <= rx; ++x) {
            if ((x * x) * (ry * ry) + (y * y) * (rx * rx) <= (rx * rx) * (ry * ry)) {
                SDL_SetRenderDrawColor(renderer, r, g, b, a);
                SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            }
        }
    }
}

void renderKnightWithScythe(SDL_Renderer* renderer, const Character& character, TTF_Font* font) {
    // Body (armor, gray)
    SDL_Rect body = { (int)character.x, (int)character.y, 48, 72 };
    SDL_SetRenderDrawColor(renderer, 120, 120, 130, 255);
    SDL_RenderFillRect(renderer, &body);
    // Helmet (ellipse, darker gray)
    drawEllipse(renderer, (int)character.x + 24, (int)character.y + 18, 18, 16, 80, 80, 90, 255);
    // Visor (horizontal slit)
    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_Rect visor = { (int)character.x + 10, (int)character.y + 18, 28, 4 };
    SDL_RenderFillRect(renderer, &visor);
    // Shoulder pads (ellipses)
    drawEllipse(renderer, (int)character.x + 8, (int)character.y + 32, 8, 6, 100, 100, 110, 255);
    drawEllipse(renderer, (int)character.x + 40, (int)character.y + 32, 8, 6, 100, 100, 110, 255);
    // Belt (brown)
    SDL_Rect belt = { (int)character.x, (int)character.y + 48, 48, 8 };
    SDL_SetRenderDrawColor(renderer, 110, 70, 30, 255);
    SDL_RenderFillRect(renderer, &belt);
    // Scythe handle (long, dark brown)
    SDL_SetRenderDrawColor(renderer, 70, 40, 20, 255);
    SDL_RenderDrawLine(renderer, (int)character.x + 44, (int)character.y + 20, (int)character.x + 10, (int)character.y + 80);
    SDL_RenderDrawLine(renderer, (int)character.x + 45, (int)character.y + 20, (int)character.x + 11, (int)character.y + 80);
    // Scythe blade (arc, silver)
    for (int i = 0; i < 18; ++i) {
        float angle = 3.14f * 0.7f + (i / 18.0f) * 3.14f * 0.7f;
        int bx = (int)(character.x + 44 + std::cos(angle) * 28);
        int by = (int)(character.y + 20 - std::sin(angle) * 18);
        SDL_SetRenderDrawColor(renderer, 200, 200, 220, 255);
        SDL_RenderDrawPoint(renderer, bx, by);
    }
    // Draw name above
    if (font) {
        SDL_Color textColor = {255,255,255,255};
        SDL_Surface* textSurface = TTF_RenderText_Blended(font, character.name.c_str(), textColor);
        if (textSurface) {
            SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_Rect textRect = { (int)character.x, (int)character.y - 24, textSurface->w, textSurface->h };
            SDL_RenderCopy(renderer, textTex, nullptr, &textRect);
            SDL_DestroyTexture(textTex);
            SDL_FreeSurface(textSurface);
        }
    }
}
// CharacterModule.cpp

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include "CharacterModule.h"

Character createNewCharacter(const std::string& name, float x, float y, CharacterColor main, CharacterColor accent) {
    Character c;
    c.x = x;
    c.y = y;
    c.vx = 0;
    c.vy = 0;
    c.health = 100;
    c.maxHealth = 100;
    c.energy = 80;
    c.maxEnergy = 100;
    c.mainColor = main;
    c.accentColor = accent;
    c.name = name;
    return c;
}

void renderCharacter(SDL_Renderer* renderer, const Character& character, TTF_Font* font) {
    // Simple placeholder: draw a colored rectangle and name
    SDL_Rect body = { (int)character.x, (int)character.y, 48, 72 };
    SDL_SetRenderDrawColor(renderer, character.mainColor.r, character.mainColor.g, character.mainColor.b, 255);
    SDL_RenderFillRect(renderer, &body);
    // Accent (e.g., belt)
    SDL_Rect belt = { (int)character.x, (int)character.y + 40, 48, 8 };
    SDL_SetRenderDrawColor(renderer, character.accentColor.r, character.accentColor.g, character.accentColor.b, 255);
    SDL_RenderFillRect(renderer, &belt);
    // Draw name above
    if (font) {
        SDL_Color textColor = {255,255,255,255};
        SDL_Surface* textSurface = TTF_RenderText_Blended(font, character.name.c_str(), textColor);
        if (textSurface) {
            SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_Rect textRect = { (int)character.x, (int)character.y - 24, textSurface->w, textSurface->h };
            SDL_RenderCopy(renderer, textTex, nullptr, &textRect);
            SDL_DestroyTexture(textTex);
            SDL_FreeSurface(textSurface);
        }
    }
}
