#pragma once
#include "Character.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>
#include <functional>

struct Fireball {
    float x, y;
    float vx, vy;
    int life;
    float size = 1.0f;
    bool exploding = false;
    int explosionFrame = 0;
};

struct DustParticle {
    float x, y;
    float vx, vy;
    int life;
};

struct OutfitColor {
    Uint8 r, g, b;
};

class WizardCharacter : public Character {
public:
    WizardCharacter(SDL_Renderer* renderer, TTF_Font* font, int windowW, int windowH);
    ~WizardCharacter();

    void handleEvent(const SDL_Event& event) override;
    void update() override;
    void render(SDL_Renderer* renderer, int camX, int camY, int windowW, int windowH) override;
    void renderAura(SDL_Renderer* renderer, std::function<SDL_Rect(float, float, float, float)> cameraTransform) override;
    void renderOnMinimap(SDL_Renderer* renderer, int miniMapX, int miniMapY, float scaleX, float scaleY) override;

    // Expose for HUD
    int getHealth() const override;
    int getMaxHealth() const override;
    int getEnergy() const;
    int getMaxEnergy() const;
    // Camera helpers
    float getX() const override;
    float getY() const override;
    int getWidth() const;
    int getHeight() const;

    void setPosition(float x, float y) override { charX = x; charY = y; }
    void setVelocity(float vx, float vy) override { charVX = vx; charVY = vy; }
    float getVX() const override { return charVX; }
    float getVY() const override { return charVY; }
    void setFacing(int f) override { facing = f; }
    int getFacing() const override { return facing; }

private:
    // State
    float charX, charY;
    float charVX, charVY;
    int facing;
    int walkFrame, walkCounter;
    int playerHealth, playerMaxHealth;
    int playerEnergy, playerMaxEnergy;
    float playerEnergyF;
    std::vector<Fireball> fireballs;
    std::vector<DustParticle> dustParticles;
    int dustEmitCooldown;
    int currentOutfit;
    std::vector<OutfitColor> outfitColors;
    // Animation
    int headBobVals[2];
    int hatTiltVals[2];
    SDL_Texture* spriteSideTexture[2];
    SDL_Texture* spriteBackTexture[2];
    SDL_Texture* spriteFrontTexture[2];
    void regenerateSprites(SDL_Renderer* renderer);
    // ...other private helpers as needed...
};
