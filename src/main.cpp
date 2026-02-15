#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>
#include <vector>
// Fireball struct
struct Fireball {
	float x, y;
	float vx, vy;
	int life;
};

int main(int argc, char* argv[]) {
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
	if (!renderer) {
		std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	// --- Marble floor texture generation ---
	SDL_Texture* marbleTexture = nullptr;
	const int tileSize = 64;
	SDL_Surface* marbleSurface = SDL_CreateRGBSurface(0, tileSize, tileSize, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	if (marbleSurface) {
		for (int y = 0; y < tileSize; ++y) {
			for (int x = 0; x < tileSize; ++x) {
				Uint8 base = 220 + (rand() % 20);
				Uint32 color = SDL_MapRGB(marbleSurface->format, base, base, base);
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

	// --- Wizard sprite animation setup ---
	const int avatarW = 120;
	const int avatarH = 160;
	auto createSpriteSurface = [&](bool isSide, bool isFront, int frame, int headBob, int hatTilt) -> SDL_Surface* {
		SDL_Surface* surf = SDL_CreateRGBSurface(0, avatarW, avatarH, 32,
			0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		if (!surf) return nullptr;
		SDL_FillRect(surf, nullptr, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
		// Body (rounded)
		for (int y = 0; y < 80; ++y) {
			int xRadius = 25 - (int)(12.0 * (1.0 - ((float)y / 80.0)));
			int xStart = 60 - xRadius;
			int xEnd = 60 + xRadius;
			for (int x = xStart; x <= xEnd; ++x) {
				if (x >= 0 && x < avatarW) {
					Uint32* pixels = (Uint32*)surf->pixels;
					pixels[(60 + y) * avatarW + x] = SDL_MapRGB(surf->format, 128, 0, 128);
				}
			}
		}
		// Head (ellipse) with bob
		int headCenterX = 60;
		int headCenterY = 45 + headBob;
		int headRadiusX = 20;
		int headRadiusY = 15;
		for (int y = -headRadiusY; y <= headRadiusY; ++y) {
			for (int x = -headRadiusX; x <= headRadiusX; ++x) {
				if ((x * x) * (headRadiusY * headRadiusY) + (y * y) * (headRadiusX * headRadiusX) <= (headRadiusX * headRadiusX) * (headRadiusY * headRadiusY)) {
					int px = headCenterX + x;
					int py = headCenterY + y;
					if (px >= 0 && px < avatarW && py >= 0 && py < avatarH) {
						Uint32* pixels = (Uint32*)surf->pixels;
						pixels[py * avatarW + px] = SDL_MapRGB(surf->format, 255, 220, 177);
					}
				}
			}
		}
		// Limbs
		if (isSide) {
			int legOffset = (frame == 0 ? 10 : -10);
			// Leg (tapered)
			for (int y = 0; y < 20; ++y) {
				int legX = 65 + legOffset + y / 8;
				int legW = 8 - y / 8;
				for (int x = 0; x < legW; ++x) {
					int px = legX + x;
					int py = 140 + y;
					if (px >= 0 && px < avatarW && py >= 0 && py < avatarH) {
						Uint32* pixels = (Uint32*)surf->pixels;
						pixels[py * avatarW + px] = SDL_MapRGB(surf->format, 60, 0, 60);
					}
				}
			}
			// Foot (circle)
			int footX = 65 + legOffset + 3;
			int footY = 158;
			for (int y = -2; y <= 2; ++y) {
				for (int x = -2; x <= 2; ++x) {
					if (x * x + y * y <= 4) {
						int px = footX + x;
						int py = footY + y;
						if (px >= 0 && px < avatarW && py >= 0 && py < avatarH) {
							Uint32* pixels = (Uint32*)surf->pixels;
							pixels[py * avatarW + px] = SDL_MapRGB(surf->format, 60, 0, 60);
						}
					}
				}
			}
		} else {
			SDL_Rect leftArm = { 30, 70 + (frame == 0 ? 0 : 10), 15, 60 };
			SDL_Rect rightArm = { 75, 70 + (frame == 1 ? 0 : 10), 15, 60 };
			SDL_FillRect(surf, &leftArm, SDL_MapRGB(surf->format, 128, 0, 128));
			SDL_FillRect(surf, &rightArm, SDL_MapRGB(surf->format, 128, 0, 128));
			SDL_Rect leftLeg = { 45, 140 + (frame == 1 ? 0 : 10), 10, 20 };
			SDL_Rect rightLeg = { 65, 140 + (frame == 0 ? 0 : 10), 10, 20 };
			SDL_FillRect(surf, &leftLeg, SDL_MapRGB(surf->format, 60, 0, 60));
			SDL_FillRect(surf, &rightLeg, SDL_MapRGB(surf->format, 60, 0, 60));
		}
		// Hat with tilt
		int hatBaseX = 50 + hatTilt;
		int hatBaseY = 15 + headBob;
		SDL_Rect hatBase = { hatBaseX, hatBaseY, 20, 15 };
		SDL_FillRect(surf, &hatBase, SDL_MapRGB(surf->format, 128, 0, 128));
		for (int y = 5; y < 15; ++y) {
			int xStart = 60 + hatTilt - (y - 5);
			int xEnd = 60 + hatTilt + (y - 5);
			for (int x = xStart; x <= xEnd; ++x) {
				if (x >= 0 && x < avatarW) {
					Uint32* pixels = (Uint32*)surf->pixels;
					pixels[(y + hatBaseY - 15) * avatarW + x] = SDL_MapRGB(surf->format, 128, 0, 128);
				}
			}
		}
		SDL_Rect pom = { 58 + hatTilt, 0 + headBob, 5, 5 };
		SDL_FillRect(surf, &pom, SDL_MapRGB(surf->format, 255, 255, 255));
		// Face/beard for side and front only
		if (isSide) {
			SDL_Rect eye = { 64, 40, 4, 4 };
			SDL_FillRect(surf, &eye, SDL_MapRGB(surf->format, 0, 0, 0));
			SDL_Rect mouth = { 66, 52, 6, 3 };
			SDL_FillRect(surf, &mouth, SDL_MapRGB(surf->format, 120, 40, 40));
			SDL_Rect beardRect = { 60, 55, 12, 12 };
			SDL_FillRect(surf, &beardRect, SDL_MapRGB(surf->format, 255, 255, 255));
			for (int y = 0; y < 12; ++y) {
				int xStart = 60 + y;
				int xEnd = 71;
				for (int x = xStart; x <= xEnd; ++x) {
					if (x >= 0 && x < avatarW) {
						Uint32* pixels = (Uint32*)surf->pixels;
						pixels[(67 + y) * avatarW + x] = SDL_MapRGB(surf->format, 255, 255, 255);
					}
				}
			}
		} else if (isFront) {
			SDL_Rect leftEye = { 52, 40, 4, 4 };
			SDL_Rect rightEye = { 64, 40, 4, 4 };
			SDL_FillRect(surf, &leftEye, SDL_MapRGB(surf->format, 0, 0, 0));
			SDL_FillRect(surf, &rightEye, SDL_MapRGB(surf->format, 0, 0, 0));
			SDL_Rect mouth = { 56, 52, 8, 3 };
			SDL_FillRect(surf, &mouth, SDL_MapRGB(surf->format, 120, 40, 40));
			SDL_Rect beardRect = { 48, 55, 24, 12 };
			SDL_FillRect(surf, &beardRect, SDL_MapRGB(surf->format, 255, 255, 255));
			for (int y = 0; y < 12; ++y) {
				int xStart = 48 + y;
				int xEnd = 71 - y;
				for (int x = xStart; x <= xEnd; ++x) {
					if (x >= 0 && x < avatarW) {
						Uint32* pixels = (Uint32*)surf->pixels;
						pixels[(67 + y) * avatarW + x] = SDL_MapRGB(surf->format, 255, 255, 255);
					}
				}
			}
		}
		return surf;
	};

	// Create two frames for each view: side, back, front, with head bob and hat tilt
	// Bob and tilt: frame 0 = up, frame 1 = down
	int headBobVals[2] = { -4, 4 };
	int hatTiltVals[2] = { -5, 5 };
	SDL_Surface* spriteSideSurface[2] = {
		createSpriteSurface(true, false, 0, headBobVals[0], hatTiltVals[0]),
		createSpriteSurface(true, false, 1, headBobVals[1], hatTiltVals[1])
	};
	SDL_Surface* spriteBackSurface[2] = {
		createSpriteSurface(false, false, 0, headBobVals[0], hatTiltVals[0]),
		createSpriteSurface(false, false, 1, headBobVals[1], hatTiltVals[1])
	};
	SDL_Surface* spriteFrontSurface[2] = {
		createSpriteSurface(false, true, 0, headBobVals[0], hatTiltVals[0]),
		createSpriteSurface(false, true, 1, headBobVals[1], hatTiltVals[1])
	};

	SDL_Texture* spriteSideTexture[2] = { nullptr, nullptr };
	SDL_Texture* spriteBackTexture[2] = { nullptr, nullptr };
	SDL_Texture* spriteFrontTexture[2] = { nullptr, nullptr };
	for (int i = 0; i < 2; ++i) {
		if (spriteSideSurface[i]) {
			spriteSideTexture[i] = SDL_CreateTextureFromSurface(renderer, spriteSideSurface[i]);
			SDL_FreeSurface(spriteSideSurface[i]);
		}
		if (spriteBackSurface[i]) {
			spriteBackTexture[i] = SDL_CreateTextureFromSurface(renderer, spriteBackSurface[i]);
			SDL_FreeSurface(spriteBackSurface[i]);
		}
		if (spriteFrontSurface[i]) {
			spriteFrontTexture[i] = SDL_CreateTextureFromSurface(renderer, spriteFrontSurface[i]);
			SDL_FreeSurface(spriteFrontSurface[i]);
		}
	}

	// Character state
	int charX = windowW / 2;
	int charY = windowH / 2;
	const int speed = 5;
	float zoom = 1.0f;
	const float zoomStep = 0.1f;
	const float minZoom = 0.5f;
	const float maxZoom = 3.0f;
	int camX = 0;
	int camY = 0;
	const int margin = 100;
	int facing = 0; // 0=right, 1=left, 2=up, 3=down
	int walkFrame = 0;
	int walkCounter = 0;
	const int walkFrameDelay = 10;
	std::vector<Fireball> fireballs;

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
						facing = 2;
						break;
					case SDLK_s:
						charY += speed;
						facing = 3;
						break;
					case SDLK_a:
						charX -= speed;
						facing = 1;
						break;
					case SDLK_d:
						charX += speed;
						facing = 0;
						break;
				}
				if (event.key.keysym.sym == SDLK_SPACE) {
					// Shoot fireball
					float fx = charX + avatarW / 2;
					float fy = charY + avatarH / 2;
					float speed = 16.0f;
					float vx = 0, vy = 0;
					if (facing == 0) { vx = speed; }
					else if (facing == 1) { vx = -speed; }
					else if (facing == 2) { vy = -speed; }
					else if (facing == 3) { vy = speed; }
					fireballs.push_back({fx, fy, vx, vy, 60});
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
		// Animation: always advance frame (persistent animation)
		walkCounter++;
		if (walkCounter >= walkFrameDelay) {
			walkCounter = 0;
			walkFrame = 1 - walkFrame;
		}
		// Update fireballs
		for (auto& f : fireballs) {
			f.x += f.vx;
			f.y += f.vy;
			f.life--;
		}
		fireballs.erase(std::remove_if(fireballs.begin(), fireballs.end(), [](const Fireball& f) { return f.life <= 0; }), fireballs.end());

		// Camera follows player if near edge
		int spriteW = static_cast<int>(avatarW * zoom);
		int spriteH = static_cast<int>(avatarH * zoom);
		if (charX - camX < margin) camX = charX - margin;
		if ((charX + spriteW) - camX > windowW - margin) camX = (charX + spriteW) - (windowW - margin);
		if (charY - camY < margin) camY = charY - margin;
		if ((charY + spriteH) - camY > windowH - margin) camY = (charY + spriteH) - (windowH - margin);

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
		// Render fireballs
		for (const auto& f : fireballs) {
			SDL_SetRenderDrawColor(renderer, 180, 0, 255, 255); // purple
			for (int r = 10; r > 0; --r) {
				for (int dy = -r; dy <= r; ++dy) {
					for (int dx = -r; dx <= r; ++dx) {
						if (dx*dx + dy*dy <= r*r) {
							int px = static_cast<int>((f.x - camX) * zoom) + dx;
							int py = static_cast<int>((f.y - camY) * zoom) + dy;
							if (px >= 0 && px < windowW && py >= 0 && py < windowH)
								SDL_RenderDrawPoint(renderer, px, py);
						}
					}
				}
			}
		}
		// Render correct sprite for direction
		SDL_Rect destRect = { charX - camX, charY - camY, spriteW, spriteH };
		if (facing == 2 && spriteBackTexture[walkFrame]) {
			SDL_RenderCopy(renderer, spriteBackTexture[walkFrame], nullptr, &destRect);
		} else if (facing == 3 && spriteFrontTexture[walkFrame]) {
			SDL_RenderCopy(renderer, spriteFrontTexture[walkFrame], nullptr, &destRect);
		} else if ((facing == 0 || facing == 1) && spriteSideTexture[walkFrame]) {
			SDL_RendererFlip flip = (facing == 1) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
			SDL_RenderCopyEx(renderer, spriteSideTexture[walkFrame], nullptr, &destRect, 0, nullptr, flip);
		}
		SDL_RenderPresent(renderer);
	}
	for (int i = 0; i < 2; ++i) {
		if (spriteSideTexture[i]) SDL_DestroyTexture(spriteSideTexture[i]);
		if (spriteBackTexture[i]) SDL_DestroyTexture(spriteBackTexture[i]);
		if (spriteFrontTexture[i]) SDL_DestroyTexture(spriteFrontTexture[i]);
	}
	if (marbleTexture) {
		SDL_DestroyTexture(marbleTexture);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
