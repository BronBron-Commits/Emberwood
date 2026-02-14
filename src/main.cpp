// Emberwood entry point
#include <SDL2/SDL.h>
#include <iostream>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Emberwood", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Generate a wizard sprite: purple cloak, purple Santa hat, and face
    SDL_Surface* spriteSurface = SDL_CreateRGBSurface(0, 40, 40, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (!spriteSurface) {
        std::cerr << "Failed to create sprite surface: " << SDL_GetError() << std::endl;
    } else {
        // Fill background with transparent (black)
        SDL_FillRect(spriteSurface, nullptr, SDL_MapRGBA(spriteSurface->format, 0, 0, 0, 0));

        // Draw purple cloak (bottom half)
        SDL_Rect cloak = { 8, 20, 24, 16 };
        SDL_FillRect(spriteSurface, &cloak, SDL_MapRGB(spriteSurface->format, 128, 0, 128));

        // Draw face (peach color)
        SDL_Rect face = { 14, 12, 12, 8 };
        SDL_FillRect(spriteSurface, &face, SDL_MapRGB(spriteSurface->format, 255, 220, 177));

        // Draw purple Santa hat base (rectangle above face)
        SDL_Rect hatBase = { 14, 8, 12, 6 };
        SDL_FillRect(spriteSurface, &hatBase, SDL_MapRGB(spriteSurface->format, 128, 0, 128));

        // Draw purple Santa hat tip (triangle)
        for (int y = 2; y < 8; ++y) {
            int xStart = 20 - (y - 2);
            int xEnd = 20 + (y - 2);
            for (int x = xStart; x <= xEnd; ++x) {
                Uint32* pixels = (Uint32*)spriteSurface->pixels;
                pixels[y * spriteSurface->w + x] = SDL_MapRGB(spriteSurface->format, 128, 0, 128);
            }
        }

        // Draw white pom-pom at tip
        SDL_Rect pom = { 19, 0, 3, 3 };
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

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
        SDL_RenderClear(renderer);

        // Render sprite if loaded
        if (spriteTexture) {
            SDL_Rect destRect = { 380, 280, 40, 40 };
            SDL_RenderCopy(renderer, spriteTexture, nullptr, &destRect);
        }

        SDL_RenderPresent(renderer);
    }

    if (spriteTexture) {
        SDL_DestroyTexture(spriteTexture);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
