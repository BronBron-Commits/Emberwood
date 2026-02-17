
#include "Cape.h"
#include <cmath>
#include <algorithm>

Cape::Cape() {}

void Cape::render(SDL_Renderer* renderer, int bodyX, int bodyY, int bodyW, int bodyH, int height, float time) {
    // --- Draw cape-like animated robe (behind body) ---
    int capeW_top = (int)(bodyW * 1.7f); // Top of cape is much wider
    int capeW_bottom = (int)(bodyW * 3.8f); // Bottom is dramatically wider
    int capeTopY = bodyY + (int)(bodyH * 0.10f);
    // Move the cape double to the left behind the player
    int capeTopX = bodyX + bodyW / 2 - capeW_top / 2 - (int)(bodyW * 2.0f);
    int capeH = (int)(height * 0.85f); // Ends before the feet
    int capeBottomY = capeTopY + capeH;
    int capeSegments = 22;
    SDL_Point capePoly[24];
    capePoly[0].x = capeTopX;
    capePoly[0].y = capeTopY;
    capePoly[capeSegments + 1].x = capeTopX + capeW_top;
    capePoly[capeSegments + 1].y = capeTopY;
    for (int i = 0; i <= capeSegments; ++i) {
        float tseg = (float)i / capeSegments;
        float wave = std::sin(time * 2.0f + tseg * 4.0f) * 8.0f * (0.5f + 0.5f * tseg);
        int x = capeTopX + (int)((capeW_top * (1.0f - tseg)) + (capeW_bottom * tseg));
        float flare = std::sin(tseg * 3.14159f) * (capeW_bottom * 0.32f); // More dramatic flare
        int y = capeBottomY + (int)wave + (int)flare;
        capePoly[1 + i].x = x;
        capePoly[1 + i].y = y;
    }
    // Draw filled cape polygon (simple scanline fill)
    SDL_SetRenderDrawColor(renderer, 80, 0, 120, 255);
    for (int i = 0; i < capeSegments; ++i) {
        int x1 = capePoly[1 + i].x, y1 = capePoly[1 + i].y;
        int x2 = capePoly[1 + i + 1].x, y2 = capePoly[1 + i + 1].y;
        float tseg1 = (float)i / capeSegments;
        float tseg2 = (float)(i + 1) / capeSegments;
        int tx1 = capeTopX + (int)((capeW_top * (1.0f - tseg1)) + (capeW_bottom * tseg1));
        int tx2 = capeTopX + (int)((capeW_top * (1.0f - tseg2)) + (capeW_bottom * tseg2));
        for (int y = capeTopY; y <= std::max(y1, y2); ++y) {
            float alpha = (y1 == y2) ? 0.5f : (float)(y - capeTopY) / (float)(std::max(y1, y2) - capeTopY + 1);
            int xL = (int)(tx1 + (x1 - tx1) * alpha);
            int xR = (int)(tx2 + (x2 - tx2) * alpha);
            if (xL > xR) std::swap(xL, xR);
            for (int x = xL; x <= xR; ++x) {
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
}
