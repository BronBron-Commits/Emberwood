// Emberwood entry point
#include <SDL2/SDL.h>
#include <iostream>

int main(int argc, char* argv[]) {
    // ...existing code...
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Get display size for fullscreen
    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) != 0) {
        std::cerr << "SDL_GetCurrentDisplayMode Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    int windowW = displayMode.w;
    int windowH = displayMode.h;
    SDL_Window* window = SDL_CreateWindow("Emberwood", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowW, windowH, SDL_WINDOW_FULLSCREEN);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        // --- Marble floor texture generation (after renderer is created) ---
        SDL_Texture* marbleTexture = nullptr;
        const int tileSize = 64;
        SDL_Surface* marbleSurface = SDL_CreateRGBSurface(0, tileSize, tileSize, 32,
            0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
        if (marbleSurface) {
            // Simple procedural marble pattern: white base with gray veins
            for (int y = 0; y < tileSize; ++y) {
                for (int x = 0; x < tileSize; ++x) {
                    // Base color
                    Uint8 base = 220 + (rand() % 20);
                    Uint32 color = SDL_MapRGB(marbleSurface->format, base, base, base);
                    // Add veins: sine wave + noise
                    float v = 0.5f * sinf((float)x * 0.15f + (float)y * 0.08f) + 0.5f;
                    if (v > 0.8f + 0.15f * ((rand() % 100) / 100.0f)) {
                        color = SDL_MapRGB(marbleSurface->format, 180, 180, 180);
                    }
                    ((Uint32*)marbleSurface->pixels)[y * tileSize + x] = color;
                }
            }
            marbleTexture = SDL_CreateTextureFromSurface(renderer, marbleSurface);
            SDL_FreeSurface(marbleSurface);
        }
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Generate a wizard sprite: purple cloak, purple Santa hat, and face
    // Bigger, more detailed avatar with limbs
    const int avatarW = 120;
    const int avatarH = 160;
    SDL_Surface* spriteSurface = SDL_CreateRGBSurface(0, avatarW, avatarH, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (!spriteSurface) {
        std::cerr << "Failed to create sprite surface: " << SDL_GetError() << std::endl;
    } else {
        // Fill background with transparent (black)
        SDL_FillRect(spriteSurface, nullptr, SDL_MapRGBA(spriteSurface->format, 0, 0, 0, 0));

        // Draw body (cloak)
        SDL_Rect body = { 35, 60, 50, 80 };
        SDL_FillRect(spriteSurface, &body, SDL_MapRGB(spriteSurface->format, 128, 0, 128));

        // Draw head (peach color)
        SDL_Rect head = { 40, 30, 40, 30 };
        SDL_FillRect(spriteSurface, &head, SDL_MapRGB(spriteSurface->format, 255, 220, 177));

        // Draw eyes (black)
        SDL_Rect leftEye = { 52, 40, 4, 4 };
        SDL_Rect rightEye = { 64, 40, 4, 4 };
        SDL_FillRect(spriteSurface, &leftEye, SDL_MapRGB(spriteSurface->format, 0, 0, 0));
        SDL_FillRect(spriteSurface, &rightEye, SDL_MapRGB(spriteSurface->format, 0, 0, 0));

        // Draw mouth (smile, dark red)
        SDL_Rect mouth = { 56, 52, 8, 3 };
        SDL_FillRect(spriteSurface, &mouth, SDL_MapRGB(spriteSurface->format, 120, 40, 40));

        // Draw white beard (rectangle + triangle)
        SDL_Rect beardRect = { 48, 55, 24, 12 };
        SDL_FillRect(spriteSurface, &beardRect, SDL_MapRGB(spriteSurface->format, 255, 255, 255));
        // Triangle part of beard (pointed)
        for (int y = 0; y < 12; ++y) {
            int xStart = 48 + y;
            int xEnd = 71 - y;
            for (int x = xStart; x <= xEnd; ++x) {
                if (x >= 0 && x < avatarW) {
                    Uint32* pixels = (Uint32*)spriteSurface->pixels;
                    pixels[(67 + y) * avatarW + x] = SDL_MapRGB(spriteSurface->format, 255, 255, 255);
                }
            }
        }

        // Draw left arm
        SDL_Rect leftArm = { 15, 70, 20, 60 };
        SDL_FillRect(spriteSurface, &leftArm, SDL_MapRGB(spriteSurface->format, 128, 0, 128));

        // Draw right arm
        SDL_Rect rightArm = { 85, 70, 20, 60 };
        SDL_FillRect(spriteSurface, &rightArm, SDL_MapRGB(spriteSurface->format, 128, 0, 128));

        // Draw left leg
        SDL_Rect leftLeg = { 45, 140, 10, 20 };
        SDL_FillRect(spriteSurface, &leftLeg, SDL_MapRGB(spriteSurface->format, 60, 0, 60));

        // Draw right leg
        SDL_Rect rightLeg = { 65, 140, 10, 20 };
        SDL_FillRect(spriteSurface, &rightLeg, SDL_MapRGB(spriteSurface->format, 60, 0, 60));

        // Draw purple Santa hat base (rectangle above face)
        SDL_Rect hatBase = { 50, 15, 20, 15 };
        SDL_FillRect(spriteSurface, &hatBase, SDL_MapRGB(spriteSurface->format, 128, 0, 128));

        // Draw purple Santa hat tip (triangle)
        for (int y = 5; y < 15; ++y) {
            int xStart = 60 - (y - 5);
            int xEnd = 60 + (y - 5);
            for (int x = xStart; x <= xEnd; ++x) {
                if (x >= 0 && x < avatarW) {
                    Uint32* pixels = (Uint32*)spriteSurface->pixels;
                    pixels[y * avatarW + x] = SDL_MapRGB(spriteSurface->format, 128, 0, 128);
                }
            }
        }

        // Draw white pom-pom at tip
        SDL_Rect pom = { 58, 0, 5, 5 };
        SDL_FillRect(spriteSurface, &pom, SDL_MapRGB(spriteSurface->format, 255, 255, 255));
    }
    SDL_Texture* spriteTexture = nullptr;
    if (spriteSurface) {
        spriteTexture = SDL_CreateTextureFromSurface(renderer, spriteSurface);
        SDL_FreeSurface(spriteSurface);
        if (!spriteTexture) {
            std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
        }
    }

    // Character position
    int charX = 380;
    int charY = 280;
    const int speed = 5;
    float zoom = 1.0f;
    const float zoomStep = 0.1f;
    const float minZoom = 0.5f;
    const float maxZoom = 3.0f;

    // Camera setup
    int camX = 0;
    int camY = 0;
    // windowW and windowH are now set from displayMode above
    const int margin = 100; // Margin from edge before camera moves

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_w:
                        charY -= speed;
                        break;
                    case SDLK_s:
                        charY += speed;
                        break;
                    case SDLK_a:
                        charX -= speed;
                        break;
                    case SDLK_d:
                        charX += speed;
                        break;
                }
            } else if (event.type == SDL_MOUSEWHEEL) {
                if (event.wheel.y > 0) {
                    zoom += zoomStep;
                    if (zoom > maxZoom) zoom = maxZoom;
                } else if (event.wheel.y < 0) {
                    zoom -= zoomStep;
                    if (zoom < minZoom) zoom = minZoom;
                }
            }
        }

        // Camera follows player if near edge
        int spriteW = static_cast<int>(avatarW * zoom);
        int spriteH = static_cast<int>(avatarH * zoom);
        // Left
        if (charX - camX < margin) {
            camX = charX - margin;
        }
        // Right
        if ((charX + spriteW) - camX > windowW - margin) {
            camX = (charX + spriteW) - (windowW - margin);
        }
        // Top
        if (charY - camY < margin) {
            camY = charY - margin;
        }
        // Bottom
        if ((charY + spriteH) - camY > windowH - margin) {
            camY = (charY + spriteH) - (windowH - margin);
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
        SDL_RenderClear(renderer);

        // --- Render marble floor, tiled ---
        if (marbleTexture) {
            int viewW = static_cast<int>(windowW / zoom) + tileSize;
            int viewH = static_cast<int>(windowH / zoom) + tileSize;
            int startX = (camX / tileSize) * tileSize;
            int startY = (camY / tileSize) * tileSize;
            for (int y = startY; y < camY + viewH; y += tileSize) {
                for (int x = startX; x < camX + viewW; x += tileSize) {
                    SDL_Rect dest = {
                        static_cast<int>((x - camX) * zoom),
                        static_cast<int>((y - camY) * zoom),
                        static_cast<int>(tileSize * zoom),
                        static_cast<int>(tileSize * zoom)
                    };
                    SDL_RenderCopy(renderer, marbleTexture, nullptr, &dest);
                }
            }
        }

        // Render sprite if loaded, scaled by zoom, relative to camera
        if (spriteTexture) {
            SDL_Rect destRect = { charX - camX, charY - camY, spriteW, spriteH };
            SDL_RenderCopy(renderer, spriteTexture, nullptr, &destRect);
        }

        SDL_RenderPresent(renderer);
    }

    if (spriteTexture) {
        SDL_DestroyTexture(spriteTexture);
    }
    if (marbleTexture) {
        SDL_DestroyTexture(marbleTexture);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
