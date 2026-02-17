#include "KnightCharacter.h"
#include <cmath>
#include <algorithm>

KnightCharacter::KnightCharacter(SDL_Renderer* renderer, TTF_Font* font, int windowW, int windowH) {
    charX = windowW / 2.0f;
    charY = windowH / 2.0f;
    charVX = 0.0f; charVY = 0.0f;
    facing = 0;
    walkFrame = 0; walkCounter = 0;
    playerHealth = 120; playerMaxHealth = 120;
    playerStamina = 100; playerMaxStamina = 100;
    playerStaminaF = (float)playerStamina;
    slashCooldown = 0;
    headBobVals[0] = -1; headBobVals[1] = 1;
    helmetTiltVals[0] = -1; helmetTiltVals[1] = 1;
    spriteSideTexture[0] = spriteSideTexture[1] = nullptr;
    spriteBackTexture[0] = spriteBackTexture[1] = nullptr;
    spriteFrontTexture[0] = spriteFrontTexture[1] = nullptr;
    regenerateSprites(renderer);
}

KnightCharacter::~KnightCharacter() {
    for (int i = 0; i < 2; ++i) {
        if (spriteSideTexture[i]) SDL_DestroyTexture(spriteSideTexture[i]);
        if (spriteBackTexture[i]) SDL_DestroyTexture(spriteBackTexture[i]);
        if (spriteFrontTexture[i]) SDL_DestroyTexture(spriteFrontTexture[i]);
    }
}

void KnightCharacter::handleEvent(const SDL_Event& event) {
    // TODO: Implement knight input (WASD, slash, etc.)
}

void KnightCharacter::update() {
    // TODO: Implement movement, animation, slash, stamina, etc.
}

void KnightCharacter::render(SDL_Renderer* renderer, int camX, int camY, int windowW, int windowH) {
    // TEMP: Draw a simple placeholder knight (gray rectangle with helmet)
    SDL_Rect destRect = { (int)(charX - camX), (int)(charY - camY), getWidth(), getHeight() };
    SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255); // knight body
    SDL_RenderFillRect(renderer, &destRect);
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255); // helmet
    SDL_Rect helmetRect = { destRect.x + 20, destRect.y + 10, getWidth() - 40, 40 };
    SDL_RenderFillRect(renderer, &helmetRect);
    SDL_SetRenderDrawColor(renderer, 200, 200, 0, 255); // visor
    SDL_Rect visorRect = { destRect.x + 40, destRect.y + 25, getWidth() - 80, 10 };
    SDL_RenderFillRect(renderer, &visorRect);
}

void KnightCharacter::renderAura(SDL_Renderer* renderer, std::function<SDL_Rect(float, float, float, float)> cameraTransform) {
    int auraParticleCount = 32;
    float auraRadius = (getWidth() + getHeight()) / 4.0f + 10.0f;
    float auraTime = SDL_GetTicks() / 1000.0f;
    float waistY = getY() + getHeight() * 0.7f;
    float centerX = getX() + getWidth() / 2;
    for (int i = 0; i < auraParticleCount; ++i) {
        float angle = (2.0f * M_PI * i) / auraParticleCount + auraTime * 0.8f;
        float px = centerX + std::cos(angle) * auraRadius;
        float py = waistY + std::sin(angle) * auraRadius;
        SDL_Rect auraRect = cameraTransform(px - 3, py - 3, 6, 6);
        Uint8 r = 200;
        Uint8 g = 200;
        Uint8 b = 80 + 80 * std::sin(angle + auraTime);
        SDL_SetRenderDrawColor(renderer, r, g, b, 180);
        SDL_RenderFillRect(renderer, &auraRect);
    }
}

void KnightCharacter::renderOnMinimap(SDL_Renderer* renderer, int miniMapX, int miniMapY, float scaleX, float scaleY) {
    int playerMiniX = miniMapX + (int)((getX() + getWidth() / 2) * scaleX);
    int playerMiniY = miniMapY + (int)((getY() + getHeight() / 2) * scaleY);
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_Rect playerDot = { playerMiniX - 3, playerMiniY - 3, 7, 7 };
    SDL_RenderFillRect(renderer, &playerDot);
}

int KnightCharacter::getHealth() const { return playerHealth; }
int KnightCharacter::getMaxHealth() const { return playerMaxHealth; }
int KnightCharacter::getStamina() const { return playerStamina; }
int KnightCharacter::getMaxStamina() const { return playerMaxStamina; }
float KnightCharacter::getX() const { return charX; }
float KnightCharacter::getY() const { return charY; }
int KnightCharacter::getWidth() const { return 120; }
int KnightCharacter::getHeight() const { return 160; }

void KnightCharacter::regenerateSprites(SDL_Renderer* renderer) {
    // TODO: Implement knight sprite generation
}
