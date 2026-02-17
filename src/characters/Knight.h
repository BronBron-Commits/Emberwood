#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include "../engine/ControllerProfile.h"


class Knight {
public:
    Knight(SDL_Renderer* renderer, TTF_Font* font, int windowW, int windowH);
    ~Knight();

    void handleEvent(const SDL_Event& event);
    void handleInput(const Uint8* keystate);
    void update();
    void render(SDL_Renderer* renderer, int camX, int camY, int windowW, int windowH);

    // Accessors for HUD, camera, etc.
    int getHealth() const;
    int getMaxHealth() const;
    int getStamina() const;
    int getMaxStamina() const;
    float getX() const;
    float getY() const;
    int getWidth() const;
    int getHeight() const;
    int getFacing() const;
    void setPosition(float x, float y);
    void setVelocity(float vx, float vy);
    void setFacing(int f);

private:
    // State (mirrors main.cpp for now)
    float x, y, vx, vy;
    int health, maxHealth, stamina, maxStamina;
    float staminaF;
    int facing;
    int walkFrame, walkCounter;
    int width, height;
    // Animation
    int headBobVals[2];
    int helmetTiltVals[2];
    // Projectiles
    struct EnergyBlade {
        float x, y, vx, vy;
        int life;
        float length = 80.0f;
        float width = 18.0f;
        float angle = 0.0f;
        bool active = true;
    };
    std::vector<EnergyBlade> energyBlades;
    // Sword spin state
    struct KnightSpinAttack {
        bool spinning = false;
        Uint32 spinStartTime = 0;
        float spinAngle = 0.0f;
    } knightSpin;
    ControllerProfile controllerProfile;
};
