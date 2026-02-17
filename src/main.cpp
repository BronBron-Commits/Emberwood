#include <algorithm>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <vector>

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

		// (zoneWidth is now fixed to 640 for debugging)

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	// (Marble floor removed)

	// --- Camera state (no zoom) ---
	// Camera position
	int camX = 0;
	int camY = 0;
	float camXf = 0.0f; // For smooth camera movement
	const int zoneWidth = 640;
	int currentZone = 0;
	int targetZone = 0;
	const float cameraLerp = 0.15f; // Smoothness factor (0.0-1.0)

	// World size (bigger than window for scrolling)
	const int worldW = 3000;
	const int worldH = 2000;

	// --- Wizard sprite animation setup ---
	// Camera transform for pure 2D (no perspective)
	auto cameraTransform = [&](float worldX, float worldY, float worldW, float worldH) -> SDL_Rect {
		int sx = static_cast<int>(worldX - camX);
		int sy = static_cast<int>(worldY - camY);
		int sw = static_cast<int>(worldW);
		int sh = static_cast<int>(worldH);
		return SDL_Rect{ sx, sy, sw, sh };
	};
	const int avatarW = 120;
	const int avatarH = 160;
	auto createSpriteSurface = [&](bool isSide, bool isFront, int frame, int headBob, int hatTilt) -> SDL_Surface* {
		SDL_Surface* surf = SDL_CreateRGBSurface(0, avatarW, avatarH, 32,
			0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		if (!surf) return nullptr;
		SDL_FillRect(surf, nullptr, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
		// Body (rounded)
		int pitch = surf->pitch / 4;
		for (int y = 0; y < 80; ++y) {
			int xRadius = 25 - (int)(12.0 * (1.0 - ((float)y / 80.0)));
			int xStart = 60 - xRadius;
			int xEnd = 60 + xRadius;
			for (int x = xStart; x <= xEnd; ++x) {
				if (x >= 0 && x < avatarW) {
					Uint32* pixels = (Uint32*)surf->pixels;
					pixels[(60 + y) * pitch + x] = SDL_MapRGB(surf->format, 128, 0, 128);
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
						pixels[py * pitch + px] = SDL_MapRGB(surf->format, 255, 220, 177);
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
						pixels[py * pitch + px] = SDL_MapRGB(surf->format, 60, 0, 60);
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
							pixels[py * pitch + px] = SDL_MapRGB(surf->format, 60, 0, 60);
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
						pixels[(67 + y) * pitch + x] = SDL_MapRGB(surf->format, 255, 255, 255);
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
			SDL_SetTextureBlendMode(spriteSideTexture[i], SDL_BLENDMODE_BLEND);
			SDL_FreeSurface(spriteSideSurface[i]);
		}
		if (spriteBackSurface[i]) {
			spriteBackTexture[i] = SDL_CreateTextureFromSurface(renderer, spriteBackSurface[i]);
			SDL_SetTextureBlendMode(spriteBackTexture[i], SDL_BLENDMODE_BLEND);
			SDL_FreeSurface(spriteBackSurface[i]);
		}
		if (spriteFrontSurface[i]) {
			spriteFrontTexture[i] = SDL_CreateTextureFromSurface(renderer, spriteFrontSurface[i]);
			SDL_SetTextureBlendMode(spriteFrontTexture[i], SDL_BLENDMODE_BLEND);
			SDL_FreeSurface(spriteFrontSurface[i]);
		}
	}

	// Character state
	int charX = windowW / 2;
	int charY = windowH / 2;
	const int speed = 5;
	// Removed zoom variables
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
					float fireballSpeed = 16.0f;
					float vx = 0, vy = 0;
					if (facing == 0) { vx = fireballSpeed; }
					else if (facing == 1) { vx = -fireballSpeed; }
					else if (facing == 2) { vy = -fireballSpeed; }
					else if (facing == 3) { vy = fireballSpeed; }
					fireballs.push_back({fx, fy, vx, vy, 60});
				}
			}
			// Removed mouse wheel zoom event
		} // <-- MISSING BRACE FIXED
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

		// Camera follows player if near edge of the screen, but clamps to world bounds
		int spriteW = avatarW;
		int spriteH = avatarH;
		// Clamp player to world bounds
		if (charX < 0) charX = 0;
		if (charY < 0) charY = 0;
		if (charX > worldW - avatarW) charX = worldW - avatarW;
		if (charY > worldH - avatarH) charY = worldH - avatarH;

		// Camera zone logic: snap to new zone if player leaves current zone
		int playerZone = charX / zoneWidth;
		if (playerZone != currentZone) {
			targetZone = playerZone;
			currentZone = playerZone;
		}
		float targetCamX = targetZone * zoneWidth;
		// Clamp targetCamX to world bounds
		if (targetCamX < 0) targetCamX = 0;
		if (targetCamX > worldW - windowW) targetCamX = worldW - windowW;
		// Smoothly move camera towards target zone
		camXf += (targetCamX - camXf) * cameraLerp;
		camX = static_cast<int>(camXf + 0.5f);
		// Camera Y is always 0 (no vertical scroll)
		camY = 0;

		SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
		SDL_RenderClear(renderer);

		// Draw solid green ground rectangle (world size, but only visible part is drawn)
		SDL_Rect groundRect = {0 - camX, 0 - camY, worldW, worldH};
		SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255); // solid green (forest green)
		SDL_RenderFillRect(renderer, &groundRect);

		// Draw red rectangle for first camera zone (region 0) with thick border for visibility
		SDL_Rect zoneRect = {0 - camX, 0 - camY, zoneWidth, windowH};
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		// Draw thick border (3px)
		for (int t = 0; t < 3; ++t) {
			SDL_Rect thickRect = {zoneRect.x + t, zoneRect.y + t, zoneRect.w - 2 * t, zoneRect.h - 2 * t};
			SDL_RenderDrawRect(renderer, &thickRect);
		}

		// ...existing code for static grass patches...
		int numPatches = 80;
		for (int i = 0; i < numPatches; ++i) {
			int px = (i * 7919 + 12345) % worldW;
			int py = (i * 15401 + 67890) % worldH;
			int patchW = 18 + (i * 7) % 10;
			int patchH = 8 + (i * 11) % 6;
			SDL_SetRenderDrawColor(renderer, 60, 180, 60, 255); // lighter green
			for (int dy = -patchH / 2; dy <= patchH / 2; ++dy) {
				for (int dx = -patchW / 2; dx <= patchW / 2; ++dx) {
					if ((dx * dx) * (patchH * patchH) + (dy * dy) * (patchW * patchW) <= (patchW * patchW) * (patchH * patchH)) {
						int gx = px + dx - camX;
						int gy = py + dy - camY;
						if (gx >= 0 && gx < windowW && gy >= 0 && gy < windowH)
							SDL_RenderDrawPoint(renderer, gx, gy);
					}
				}
			}
		}

		// Draw grid overlay on top of grass
		const int gridSpacing = 128;
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(renderer, 200, 200, 255, 180); // bright blue, semi-transparent
		// Draw thicker grid lines (3px)
		for (int gx = 0; gx <= worldW; gx += gridSpacing) {
			int x = gx - camX;
			if (x >= 0 && x < windowW) {
				for (int t = -1; t <= 1; ++t) {
					if (x + t >= 0 && x + t < windowW)
						SDL_RenderDrawLine(renderer, x + t, 0, x + t, windowH);
				}
			}
		}
		for (int gy = 0; gy <= worldH; gy += gridSpacing) {
			int y = gy - camY;
			if (y >= 0 && y < windowH) {
				for (int t = -1; t <= 1; ++t) {
					if (y + t >= 0 && y + t < windowH)
						SDL_RenderDrawLine(renderer, 0, y + t, windowW, y + t);
				}
			}
		}
		// Optionally: draw simple coordinate dots at grid intersections
		SDL_SetRenderDrawColor(renderer, 255, 255, 0, 220); // yellow dots, semi-transparent
		for (int gx = 0; gx <= worldW; gx += gridSpacing) {
			for (int gy = 0; gy <= worldH; gy += gridSpacing) {
				int x = gx - camX;
				int y = gy - camY;
				if (x >= 0 && x < windowW && y >= 0 && y < windowH) {
					// Draw a 3x3 dot for visibility
					for (int dx = -1; dx <= 1; ++dx) {
						for (int dy = -1; dy <= 1; ++dy) {
							int px = x + dx, py = y + dy;
							if (px >= 0 && px < windowW && py >= 0 && py < windowH)
								SDL_RenderDrawPoint(renderer, px, py);
						}
					}
				}
			}
		}
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);


		// (Castle rendering removed)
		// Render fireballs
		for (const auto& f : fireballs) {
			SDL_SetRenderDrawColor(renderer, 180, 0, 255, 255); // purple
			SDL_Rect fbRect = cameraTransform(f.x - 10, f.y - 10, 20, 20);
			for (int r = fbRect.w / 2; r > 0; --r) {
				for (int dy = -r; dy <= r; ++dy) {
					for (int dx = -r; dx <= r; ++dx) {
						if (dx*dx + dy*dy <= r*r) {
							int px = fbRect.x + fbRect.w / 2 + dx;
							int py = fbRect.y + fbRect.h / 2 + dy;
							if (px >= 0 && px < windowW && py >= 0 && py < windowH)
								SDL_RenderDrawPoint(renderer, px, py);
						}
					}
				}
			}
		}
		// Render aura particles around player
		int auraParticleCount = 32;
		float auraRadius = (avatarW + avatarH) / 4.0f + 10.0f;
		float auraTime = SDL_GetTicks() / 1000.0f;
		for (int i = 0; i < auraParticleCount; ++i) {
			float angle = (2.0f * M_PI * i) / auraParticleCount + auraTime * 0.8f;
			float px = charX + avatarW / 2 + std::cos(angle) * auraRadius;
			float py = charY + avatarH / 2 + std::sin(angle) * auraRadius;
			SDL_Rect auraRect = cameraTransform(px - 3, py - 3, 6, 6);
			// Animate color for a magical effect
			Uint8 r = 120 + 80 * std::sin(angle + auraTime);
			Uint8 g = 120 + 80 * std::sin(angle + auraTime + 2.0f);
			Uint8 b = 255;
			SDL_SetRenderDrawColor(renderer, r, g, b, 180);
			SDL_RenderFillRect(renderer, &auraRect);
		}

		// Render correct sprite for direction
		SDL_Rect destRect = cameraTransform(charX, charY, avatarW, avatarH);
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
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
