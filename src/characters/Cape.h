#pragma once
#include <SDL.h>

class Cape {
public:
    Cape();
    void render(SDL_Renderer* renderer, int bodyX, int bodyY, int bodyW, int bodyH, int height, float time);
};
