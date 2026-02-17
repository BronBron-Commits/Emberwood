#pragma once
#include <SDL.h>
#include <SDL_ttf.h>

class Wizard;
class WizardDemo {
public:
    WizardDemo(SDL_Renderer* renderer, int windowW, int windowH, TTF_Font* font);
    void render();
    void handleEvent(const SDL_Event& event);
private:
    SDL_Renderer* renderer;
    int windowW, windowH;
    TTF_Font* font;
    float x, y;
    int width, height;
    Wizard* wizard;
};
