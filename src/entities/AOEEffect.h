#pragma once
#include <SDL2/SDL.h>
#include <cmath>

class AOEEffect {
public:
    float x, y;
    int duration;
    int timer;
    int radius;
    bool active;

    AOEEffect(float x_, float y_, int duration_, int radius_)
        : x(x_), y(y_), duration(duration_ * 8), timer(duration_ * 8), radius(radius_ * 4), active(true) {} // Double size

    void update() {
        if (timer > 0) --timer;
        else active = false;
    }

    void render(SDL_Renderer* renderer, int camX, int camY, int windowW, int windowH) {
        if (!active) return;
        int cx = (int)x - camX;
        int cy = (int)y - camY;
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        // Explosion stages: first a much brighter, larger flash, then a vibrant color ring, then a shockwave
        int stage1 = duration * 0.30; // 30% of duration (longer flash)
        int stage2 = duration * 0.55; // next 25%
        int stage3 = duration * 0.80; // next 25% (shockwave starts)
        int elapsed = duration - timer;

        if (elapsed < stage1) {
            // Stage 1: very bright, large white flash with a magenta core
            int flashOuter = radius + 64;
            int flashInner = radius - 48;
            for (int r = flashInner; r <= flashOuter; ++r) {
                // White outer flash
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                for (int deg = 0; deg < 360; ++deg) {
                    float rad = deg * M_PI / 180.0f;
                    int px = cx + (int)(r * std::cos(rad));
                    int py = cy + (int)(r * std::sin(rad));
                    if (px >= 0 && py >= 0 && px < windowW && py < windowH)
                        SDL_RenderDrawPoint(renderer, px, py);
                }
            }
            // Magenta core
            int coreOuter = radius + 16;
            int coreInner = radius - 32;
            for (int r = coreInner; r <= coreOuter; ++r) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 200, 220);
                for (int deg = 0; deg < 360; ++deg) {
                    float rad = deg * M_PI / 180.0f;
                    int px = cx + (int)(r * std::cos(rad));
                    int py = cy + (int)(r * std::sin(rad));
                    if (px >= 0 && py >= 0 && px < windowW && py < windowH)
                        SDL_RenderDrawPoint(renderer, px, py);
                }
            }
        } else if (elapsed < stage2) {
            // Stage 2: vibrant purple ring, even larger
            int ringOuter = radius + 40;
            int ringInner = radius - 8;
            for (int r = ringInner; r <= ringOuter; ++r) {
                SDL_SetRenderDrawColor(renderer, 200, 0, 255, 255);
                for (int deg = 0; deg < 360; ++deg) {
                    float rad = deg * M_PI / 180.0f;
                    int px = cx + (int)(r * std::cos(rad));
                    int py = cy + (int)(r * std::sin(rad));
                    if (px >= 0 && py >= 0 && px < windowW && py < windowH)
                        SDL_RenderDrawPoint(renderer, px, py);
                }
            }
        }

        // Main persistent AOE: fading and flashing purple ring
        float baseAlpha = 255.0f * (float)timer / (float)duration;
        // Flashing effect: modulate alpha and color with a much faster sine wave
        float flashPhase = (float)timer * 1.5f; // much faster speed
        float flash = 0.5f + 0.5f * std::sin(flashPhase);
        Uint8 flashAlpha = (Uint8)(baseAlpha * (0.7f + 0.6f * flash));
        Uint8 flashR = (Uint8)(180 + 60 * flash); // purple to pink
        Uint8 flashG = 0;
        Uint8 flashB = (Uint8)(255 - 40 * flash); // purple to blueish
        int outer = radius;
        int inner = radius - 32;
        for (int r = inner; r <= outer; ++r) {
            SDL_SetRenderDrawColor(renderer, flashR, flashG, flashB, flashAlpha);
            for (int deg = 0; deg < 360; ++deg) {
                float rad = deg * M_PI / 180.0f;
                int px = cx + (int)(r * std::cos(rad));
                int py = cy + (int)(r * std::sin(rad));
                if (px >= 0 && py >= 0 && px < windowW && py < windowH)
                    SDL_RenderDrawPoint(renderer, px, py);
            }
        }

        // Stage 3: shockwave (expanding, fading ring, much faster)
        if (elapsed >= stage2 && elapsed < stage3) {
            float t = (float)(elapsed - stage2) / (float)(stage3 - stage2); // 0 to 1
            // Make shockwave move outward much faster
            int shockwaveRadius = radius + 32 + (int)(t * 420); // much faster expansion
            int shockwaveThickness = 10 + (int)(10 * (1.0f - t)); // thins out
            int shockwaveAlpha = (int)(180 * (1.0f - t)); // fades out
            int swInner = shockwaveRadius - shockwaveThickness;
            int swOuter = shockwaveRadius + shockwaveThickness;
            for (int r = swInner; r <= swOuter; ++r) {
                SDL_SetRenderDrawColor(renderer, 200, 180, 255, shockwaveAlpha);
                for (int deg = 0; deg < 360; ++deg) {
                    float rad = deg * M_PI / 180.0f;
                    int px = cx + (int)(r * std::cos(rad));
                    int py = cy + (int)(r * std::sin(rad));
                    if (px >= 0 && py >= 0 && px < windowW && py < windowH)
                        SDL_RenderDrawPoint(renderer, px, py);
                }
            }
        }
    }
};
