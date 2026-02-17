#pragma once
#include <SDL.h>
#include <SDL_ttf.h>

class Game {
public:
    Game(int windowW, int windowH);
    ~Game();
    bool init();
    void run();
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    int windowW, windowH;
    bool quit;
};
