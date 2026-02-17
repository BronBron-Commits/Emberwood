#include "Checkerboard.h"

void renderCheckerboard(SDL_Renderer* renderer, int windowW, int windowH, int tileSize) {
    for (int y = 0; y < windowH; y += tileSize) {
        for (int x = 0; x < windowW; x += tileSize) {
            bool isDark = ((x / tileSize) + (y / tileSize)) % 2 == 0;
            if (isDark)
                SDL_SetRenderDrawColor(renderer, 60, 60, 80, 255);
            else
                SDL_SetRenderDrawColor(renderer, 180, 180, 200, 255);
            SDL_Rect tileRect = { x, y, tileSize, tileSize };
            SDL_RenderFillRect(renderer, &tileRect);
        }
    }
}
