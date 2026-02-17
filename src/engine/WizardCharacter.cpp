#include "WizardCharacter.h"
#include <cmath>
#include <algorithm>

WizardCharacter::WizardCharacter(SDL_Renderer* renderer, TTF_Font* font, int windowW, int windowH) {
    // Initialize state (copy from main.cpp logic)
    charX = windowW / 2.0f;
    charY = windowH / 2.0f;
    charVX = 0.0f; charVY = 0.0f;
    facing = 0;
    walkFrame = 0; walkCounter = 0;
    playerHealth = 100; playerMaxHealth = 100;
    playerEnergy = 80; playerMaxEnergy = 100;
    playerEnergyF = (float)playerEnergy;
    dustEmitCooldown = 0;
    currentOutfit = 0;
    outfitColors = {
        {128, 0, 128}, {0, 120, 200}, {200, 60, 40}, {40, 180, 60}, {200, 200, 60}
    };
    headBobVals[0] = -2; headBobVals[1] = 2;
    hatTiltVals[0] = -2; hatTiltVals[1] = 2;
    spriteSideTexture[0] = spriteSideTexture[1] = nullptr;
    spriteBackTexture[0] = spriteBackTexture[1] = nullptr;
    spriteFrontTexture[0] = spriteFrontTexture[1] = nullptr;
    regenerateSprites(renderer);
}

WizardCharacter::~WizardCharacter() {
    for (int i = 0; i < 2; ++i) {
        if (spriteSideTexture[i]) SDL_DestroyTexture(spriteSideTexture[i]);
        if (spriteBackTexture[i]) SDL_DestroyTexture(spriteBackTexture[i]);
        if (spriteFrontTexture[i]) SDL_DestroyTexture(spriteFrontTexture[i]);
    }
}

void WizardCharacter::handleEvent(const SDL_Event& event) {
    // TODO: Implement input handling (WASD, fireball, outfit, etc.)
}

void WizardCharacter::update() {
    // TODO: Implement movement, animation, fireball, dust, etc.
}

void WizardCharacter::render(SDL_Renderer* renderer, int camX, int camY, int windowW, int windowH) {
    // TODO: Implement rendering logic (sprites, fireballs, dust, etc.)
}

void WizardCharacter::renderAura(SDL_Renderer* renderer, std::function<SDL_Rect(float, float, float, float)> cameraTransform) {
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
        Uint8 r = 120 + 80 * std::sin(angle + auraTime);
        Uint8 g = 120 + 80 * std::sin(angle + auraTime + 2.0f);
        Uint8 b = 255;
        SDL_SetRenderDrawColor(renderer, r, g, b, 180);
        SDL_RenderFillRect(renderer, &auraRect);
    }
}

void WizardCharacter::renderOnMinimap(SDL_Renderer* renderer, int miniMapX, int miniMapY, float scaleX, float scaleY) {
    int playerMiniX = miniMapX + (int)((getX() + getWidth() / 2) * scaleX);
    int playerMiniY = miniMapY + (int)((getY() + getHeight() / 2) * scaleY);
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_Rect playerDot = { playerMiniX - 3, playerMiniY - 3, 7, 7 };
    SDL_RenderFillRect(renderer, &playerDot);
}

int WizardCharacter::getHealth() const { return playerHealth; }
int WizardCharacter::getMaxHealth() const { return playerMaxHealth; }
int WizardCharacter::getEnergy() const { return playerEnergy; }
int WizardCharacter::getMaxEnergy() const { return playerMaxEnergy; }
float WizardCharacter::getX() const { return charX; }
float WizardCharacter::getY() const { return charY; }
int WizardCharacter::getWidth() const { return 120; }
int WizardCharacter::getHeight() const { return 160; }

void WizardCharacter::regenerateSprites(SDL_Renderer* renderer) {
    // TODO: Copy sprite generation logic from main.cpp
}
