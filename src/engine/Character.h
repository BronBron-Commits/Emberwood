#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <functional>

class Character {
public:
    virtual ~Character() {}
    virtual void handleEvent(const SDL_Event& event) = 0;
    virtual void update() = 0;
    virtual void render(SDL_Renderer* renderer, int camX, int camY, int windowW, int windowH) = 0;
    virtual void renderAura(SDL_Renderer* renderer, std::function<SDL_Rect(float, float, float, float)> cameraTransform) = 0;
    virtual void renderOnMinimap(SDL_Renderer* renderer, int miniMapX, int miniMapY, float scaleX, float scaleY) = 0;
    virtual void setPosition(float x, float y) = 0;
    virtual float getX() const = 0;
    virtual float getY() const = 0;
    virtual void setVelocity(float vx, float vy) = 0;
    virtual float getVX() const = 0;
    virtual float getVY() const = 0;
    virtual void setFacing(int f) = 0;
    virtual int getFacing() const = 0;
    virtual int getHealth() const = 0;
    virtual int getMaxHealth() const = 0;
};
