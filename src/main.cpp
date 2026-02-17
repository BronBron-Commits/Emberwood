#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <vector>


#include "CharacterModule.h"

#include <sstream>

// Fallback for std::to_string on older compilers
#ifndef HAS_STD_TO_STRING
#if __cplusplus < 201103L || (defined(_MSC_VER) && _MSC_VER < 1800)
namespace std {
	template <typename T>
	std::string to_string(const T& value) {
		std::ostringstream os;
		os << value;
		return os.str();
	}
}
#endif
#endif




struct DustParticle {
	float x, y;
	float vx, vy;
	int life;
};

struct Fireball {
	float x, y;
	float vx, vy;
	int life;
	float size = 1.0f;
	bool exploding = false;
	int explosionFrame = 0;
};



int main(int argc, char* argv[]) {
	// --- FPS display state ---
	TTF_Init();
	TTF_Font* fpsFont = TTF_OpenFont("assets/fonts/DejaVuSans.ttf", 18);
	if (!fpsFont) {
		std::cerr << "Failed to load font for FPS: " << TTF_GetError() << std::endl;
		TTF_Quit();
		return 1;
	}
	SDL_Color fpsColor = {255, 255, 255, 255};
	Uint32 fpsLastTime = SDL_GetTicks();
	int fpsFrames = 0;
	float fpsValue = 0.0f;
	SDL_Texture* fpsTexture = nullptr;
	int fpsTextW = 0, fpsTextH = 0;
	// --- HUD state ---
	int playerHealth = 100;
	int playerMaxHealth = 100;
	int playerEnergy = 80;
	int playerMaxEnergy = 100;
	const int fireballEnergyCost = 40;
	const float energyRegenPerFrame = 3.0f; // Even faster regen (about 180 per second at 60fps)
	float playerEnergyF = (float)playerEnergy;
	// --- Fireball cooldown ---
	const Uint32 fireballCooldownMs = 500; // 0.5 seconds
	Uint32 lastFireballTime = 0;
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
	int windowW = 1280;
	int windowH = 720;
	SDL_Window* window = SDL_CreateWindow("Emberwood", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowW, windowH, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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
	// Camera position (chase camera)
	int camX = 0;
	int camY = 0;
	float camXf = 0.0f; // For smooth camera movement
	float camYf = 0.0f;
	const float cameraLerp = 0.12f; // Faster, more responsive chase camera


	// World size (bigger than window for scrolling)
	const int worldW = 3000;
	const int worldH = 2000;

	// Unwalkable area (off the island)
	const int unwalkableX = 2600;
	const int unwalkableY = 400;
	const int unwalkableW = 340;
	const int unwalkableH = 800;

	// --- Outfit color selection ---
	struct OutfitColor {
		Uint8 r, g, b;
	};
	std::vector<OutfitColor> outfitColors = {
		{128, 0, 128},    // purple (default)
		{0, 120, 200},    // blue
		{200, 60, 40},    // red
		{40, 180, 60},    // green
		{200, 200, 60}    // yellow
	};
	int currentOutfit = 0;

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
	auto createSpriteSurface = [&](bool isSide, bool isFront, int frame, int headBob, int hatTilt, OutfitColor outfit) -> SDL_Surface* {
		SDL_Surface* surf = SDL_CreateRGBSurface(0, avatarW, avatarH, 32,
			0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		if (!surf) return nullptr;
		SDL_FillRect(surf, nullptr, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
		// Medieval robe body (tunic upper, skirt lower, belt, trim)
		int pitch = surf->pitch / 4;
		// Tunic upper body (rounded)
		for (int y = 0; y < 50; ++y) {
			int xRadius = 22 - (int)(10.0 * (1.0 - ((float)y / 50.0)));
			int xStart = 60 - xRadius;
			int xEnd = 60 + xRadius;
			for (int x = xStart; x <= xEnd; ++x) {
				if (x >= 0 && x < avatarW) {
					Uint32* pixels = (Uint32*)surf->pixels;
					pixels[(70 + y) * pitch + x] = SDL_MapRGB(surf->format, outfit.r, outfit.g, outfit.b);
				}
			}
		}
		// Robe skirt (wider, with trim)
		for (int y = 0; y < 30; ++y) {
			int xRadius = 28 - (int)(6.0 * ((float)y / 30.0));
			int xStart = 60 - xRadius;
			int xEnd = 60 + xRadius;
			for (int x = xStart; x <= xEnd; ++x) {
				if (x >= 0 && x < avatarW) {
					Uint32* pixels = (Uint32*)surf->pixels;
					// Add gold trim at the bottom
					if (y == 29 || y == 28)
						pixels[(120 + y) * pitch + x] = SDL_MapRGB(surf->format, 200, 180, 60);
					else
						pixels[(120 + y) * pitch + x] = SDL_MapRGB(surf->format, outfit.r, outfit.g, outfit.b);
				}
			}
		}
		// Belt (brown)
		for (int y = 0; y < 5; ++y) {
			for (int x = 40; x < 80; ++x) {
				Uint32* pixels = (Uint32*)surf->pixels;
				pixels[(115 + y) * pitch + x] = SDL_MapRGB(surf->format, 110, 70, 30);
			}
		}
		// Buttons (vertical, gold)
		for (int y = 0; y < 3; ++y) {
			int px = 60;
			int py = 90 + y * 10;
			Uint32* pixels = (Uint32*)surf->pixels;
			pixels[py * pitch + px] = SDL_MapRGB(surf->format, 220, 200, 80);
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
		// Limbs (robe sleeves and boots)
		if (isSide) {
			int legOffset = (frame == 0 ? 10 : -10);
			// Robe sleeve (wide, matching robe)
			for (int y = 0; y < 30; ++y) {
				int armX = 35 + legOffset + y / 8;
				int armW = 16 - y / 8;
				for (int x = 0; x < armW; ++x) {
					int px = armX + x;
					int py = 80 + y;
					if (px >= 0 && px < avatarW && py >= 0 && py < avatarH) {
						Uint32* pixels = (Uint32*)surf->pixels;
						pixels[py * pitch + px] = SDL_MapRGB(surf->format, outfit.r, outfit.g, outfit.b);
					}
				}
			}
			// Leg (under robe, boots)
			for (int y = 0; y < 20; ++y) {
				int legX = 65 + legOffset + y / 8;
				int legW = 8 - y / 8;
				for (int x = 0; x < legW; ++x) {
					int px = legX + x;
					int py = 150 + y;
					if (px >= 0 && px < avatarW && py >= 0 && py < avatarH) {
						Uint32* pixels = (Uint32*)surf->pixels;
						pixels[py * pitch + px] = SDL_MapRGB(surf->format, 60, 0, 60);
					}
				}
			}
			// Boot (circle)
			int footX = 65 + legOffset + 3;
			int footY = 168;
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
			// Robe sleeves (wide, matching robe)
			SDL_Rect leftSleeve = { 30, 80 + (frame == 0 ? 0 : 10), 18, 40 };
			SDL_Rect rightSleeve = { 72, 80 + (frame == 1 ? 0 : 10), 18, 40 };
			SDL_FillRect(surf, &leftSleeve, SDL_MapRGB(surf->format, outfit.r, outfit.g, outfit.b));
			SDL_FillRect(surf, &rightSleeve, SDL_MapRGB(surf->format, outfit.r, outfit.g, outfit.b));
			// Legs (under robe, boots)
			SDL_Rect leftLeg = { 50, 150 + (frame == 1 ? 0 : 10), 10, 18 };
			SDL_Rect rightLeg = { 70, 150 + (frame == 0 ? 0 : 10), 10, 18 };
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
	SDL_Texture* spriteSideTexture[2] = { nullptr, nullptr };
	SDL_Texture* spriteBackTexture[2] = { nullptr, nullptr };
	SDL_Texture* spriteFrontTexture[2] = { nullptr, nullptr };
	auto regenerateSprites = [&]() {
		SDL_Surface* spriteSideSurface[2] = {
			createSpriteSurface(true, false, 0, headBobVals[0], hatTiltVals[0], outfitColors[currentOutfit]),
			createSpriteSurface(true, false, 1, headBobVals[1], hatTiltVals[1], outfitColors[currentOutfit])
		};
		SDL_Surface* spriteBackSurface[2] = {
			createSpriteSurface(false, false, 0, headBobVals[0], hatTiltVals[0], outfitColors[currentOutfit]),
			createSpriteSurface(false, false, 1, headBobVals[1], hatTiltVals[1], outfitColors[currentOutfit])
		};
		SDL_Surface* spriteFrontSurface[2] = {
			createSpriteSurface(false, true, 0, headBobVals[0], hatTiltVals[0], outfitColors[currentOutfit]),
			createSpriteSurface(false, true, 1, headBobVals[1], hatTiltVals[1], outfitColors[currentOutfit])
		};
		for (int i = 0; i < 2; ++i) {
			if (spriteSideTexture[i]) SDL_DestroyTexture(spriteSideTexture[i]);
			if (spriteBackTexture[i]) SDL_DestroyTexture(spriteBackTexture[i]);
			if (spriteFrontTexture[i]) SDL_DestroyTexture(spriteFrontTexture[i]);
			spriteSideTexture[i] = SDL_CreateTextureFromSurface(renderer, spriteSideSurface[i]);
			SDL_SetTextureBlendMode(spriteSideTexture[i], SDL_BLENDMODE_BLEND);
			SDL_FreeSurface(spriteSideSurface[i]);
			spriteBackTexture[i] = SDL_CreateTextureFromSurface(renderer, spriteBackSurface[i]);
			SDL_SetTextureBlendMode(spriteBackTexture[i], SDL_BLENDMODE_BLEND);
			SDL_FreeSurface(spriteBackSurface[i]);
			spriteFrontTexture[i] = SDL_CreateTextureFromSurface(renderer, spriteFrontSurface[i]);
			SDL_SetTextureBlendMode(spriteFrontTexture[i], SDL_BLENDMODE_BLEND);
			SDL_FreeSurface(spriteFrontSurface[i]);
		}
	};
	regenerateSprites();



	// Use a vector of Character objects
	std::vector<Character> characters;
	characters.push_back(createNewCharacter("Wizard", windowW / 2.0f, windowH / 2.0f, {128, 0, 128}, {200, 180, 60}));
	// Add a second character: Knight at the same position as the wizard
	characters.push_back(createNewCharacter("Knight", windowW / 2.0f, windowH / 2.0f, {120, 120, 130}, {110, 70, 30}));
	// For now, the first character is the player
	int playerIndex = 0;

	const float accel = 2.2f;
	const float maxSpeed = 15.0f;
	const float friction = 0.92f;
	const int margin = 100;
	int facing = 0;
	int walkFrame = 0;
	int walkCounter = 0;
	const int walkFrameDelay = 10;
	std::vector<Fireball> fireballs;
	std::vector<DustParticle> dustParticles;
	int dustEmitCooldown = 0;

	bool running = true;
	SDL_Event event;
	while (running) {
		// Update window size in case of resize/fullscreen
		SDL_GetWindowSize(window, &windowW, &windowH);

		// --- FPS calculation ---
		fpsFrames++;
		Uint32 fpsCurrent = SDL_GetTicks();
		if (fpsCurrent - fpsLastTime >= 500) { // update every 0.5s
			fpsValue = fpsFrames * 1000.0f / (fpsCurrent - fpsLastTime);
			fpsFrames = 0;
			fpsLastTime = fpsCurrent;
			if (fpsTexture) {
				SDL_DestroyTexture(fpsTexture);
				fpsTexture = nullptr;
			}
			std::string fpsText = "FPS: " + std::to_string((int)fpsValue);
			SDL_Surface* fpsSurface = TTF_RenderText_Blended(fpsFont, fpsText.c_str(), fpsColor);
			if (fpsSurface) {
				fpsTexture = SDL_CreateTextureFromSurface(renderer, fpsSurface);
				fpsTextW = fpsSurface->w;
				fpsTextH = fpsSurface->h;
				SDL_FreeSurface(fpsSurface);
			}
		}

		// --- Input and update for all characters ---
		bool up = false, down = false, left = false, right = false;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
			} else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
				bool pressed = (event.type == SDL_KEYDOWN);
				switch (event.key.keysym.sym) {
					case SDLK_w: up = pressed; break;
					case SDLK_s: down = pressed; break;
					case SDLK_a: left = pressed; break;
					case SDLK_d: right = pressed; break;
					// Switch to Wizard (index 0)
					case SDLK_1:
						if (pressed) playerIndex = 0;
						break;
					// Switch to Knight (index 1)
					case SDLK_2:
						if (pressed && characters.size() > 1) {
							playerIndex = 1;
							// Reset knight's velocity to prevent flying out of camera
							characters[1].vx = 0;
							characters[1].vy = 0;
						}
						break;
					// Outfit color cycling
					case SDLK_F1:
						if (pressed) {
							currentOutfit = (currentOutfit + 1) % outfitColors.size();
							regenerateSprites();
						}
						break;
					case SDLK_F2:
						if (pressed) {
							currentOutfit = (currentOutfit - 1 + outfitColors.size()) % outfitColors.size();
							regenerateSprites();
						}
						break;
					// Toggle borderless fullscreen with F11
					case SDLK_F11:
						if (pressed) {
							Uint32 flags = SDL_GetWindowFlags(window);
							if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
								SDL_SetWindowFullscreen(window, 0); // back to windowed
							} else {
								SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP); // borderless fullscreen
							}
						}
						break;
				}
				if (pressed && event.key.keysym.sym == SDLK_SPACE) {
					// Shoot fireball if enough energy and cooldown expired
					Uint32 now = SDL_GetTicks();
					if (now - lastFireballTime >= fireballCooldownMs) {
						Character& player = characters[playerIndex];
						if (playerEnergy >= fireballEnergyCost) {
							float fx = player.x + avatarW / 2;
							float fy = player.y + avatarH / 2;
							float fireballSpeed = 16.0f;
							float vx = 0, vy = 0;
							if (facing == 0) { vx = fireballSpeed; }
							else if (facing == 1) { vx = -fireballSpeed; }
							else if (facing == 2) { vy = -fireballSpeed; }
							else if (facing == 3) { vy = fireballSpeed; }
							Fireball fb = {fx, fy, vx, vy, 60, 1.0f, false, 0};
							fireballs.push_back(fb);
							playerEnergy -= fireballEnergyCost;
							playerEnergyF -= fireballEnergyCost;
							if (playerEnergy < 0) playerEnergy = 0;
							if (playerEnergyF < 0) playerEnergyF = 0;
							lastFireballTime = now;
						}
					}
				}
			}
		}
		// Movement input (ice/inertia) for player character
		const Uint8* keystate = SDL_GetKeyboardState(NULL);
		up = keystate[SDL_SCANCODE_W];
		down = keystate[SDL_SCANCODE_S];
		left = keystate[SDL_SCANCODE_A];
		right = keystate[SDL_SCANCODE_D];
		Character& player = characters[playerIndex];
		if (up)    { player.vy -= accel; facing = 2; }
		if (down)  { player.vy += accel; facing = 3; }
		if (left)  { player.vx -= accel; facing = 1; }
		if (right) { player.vx += accel; facing = 0; }
		// Clamp velocity
		float speedVal = std::sqrt(player.vx * player.vx + player.vy * player.vy);
		if (speedVal > maxSpeed) {
			player.vx = (player.vx / speedVal) * maxSpeed;
			player.vy = (player.vy / speedVal) * maxSpeed;
		}
		// Apply velocity
		player.x += player.vx;
		player.y += player.vy;
		// Apply friction
		player.vx *= friction;
		player.vy *= friction;

		// Animation: always advance frame (persistent animation)
		walkCounter++;
		if (walkCounter >= walkFrameDelay) {
			walkCounter = 0;
			walkFrame = 1 - walkFrame;
		}

		// Emit dust trail if moving
		float moveSpeed = std::sqrt(player.vx * player.vx + player.vy * player.vy);
		if (moveSpeed > 1.0f && dustEmitCooldown <= 0) {
			// Emit 4-6 much larger particles per frame from feet
			int numParticles = 4 + (rand() % 3);
			for (int i = 0; i < numParticles; ++i) {
				float footX = player.x + avatarW / 2 + ((rand() % 41) - 20) * 0.2f;
				float footY = player.y + avatarH - 18 + ((rand() % 21) - 10) * 0.2f;
				float angle = ((rand() % 360) / 180.0f) * M_PI;
				float speed = 1.2f + (rand() % 10) * 0.15f;
				float vx = std::cos(angle) * speed * 0.7f + player.vx * 0.15f;
				float vy = std::abs(std::sin(angle)) * speed * 0.7f + 0.7f;
				int life = 22 + (rand() % 10);
				dustParticles.push_back({footX, footY, vx, vy, life});
			}
			dustEmitCooldown = 0; // emit every frame
		} else if (dustEmitCooldown > 0) {
			dustEmitCooldown--;
		}

		// Update dust particles
		for (auto& d : dustParticles) {
			d.x += d.vx;
			d.y += d.vy;
			d.vx *= 0.92f;
			d.vy *= 0.92f;
			d.life--;
		}
		dustParticles.erase(std::remove_if(dustParticles.begin(), dustParticles.end(), [](const DustParticle& d) { return d.life <= 0; }), dustParticles.end());

		// Update fireballs (grow and explode)
		for (auto& f : fireballs) {
			if (!f.exploding) {
				f.x += f.vx;
				f.y += f.vy;
				f.size += 0.22f; // grow as it travels
				f.life--;
				if (f.life <= 0) {
					f.exploding = true;
					f.explosionFrame = 0;
				}
			} else {
				f.explosionFrame++;
			}
		}
		// Remove fireballs after explosion animation
		fireballs.erase(std::remove_if(fireballs.begin(), fireballs.end(), [](const Fireball& f) { return f.exploding && f.explosionFrame > 18; }), fireballs.end());

		// Camera follows player if near edge of the screen, but clamps to world bounds
		int spriteW = avatarW;
		int spriteH = avatarH;

		// Clamp player to world bounds and prevent entry into unwalkable area
		if (player.x < 0) player.x = 0;
		if (player.y < 0) player.y = 0;
		if (player.x > worldW - avatarW) player.x = worldW - avatarW;
		if (player.y > worldH - avatarH) player.y = worldH - avatarH;
		// Prevent entry into unwalkable area (simple AABB check)
		float px1 = player.x, py1 = player.y, px2 = player.x + avatarW, py2 = player.y + avatarH;
		float ux1 = unwalkableX, uy1 = unwalkableY, ux2 = unwalkableX + unwalkableW, uy2 = unwalkableY + unwalkableH;
		if (px2 > ux1 && px1 < ux2 && py2 > uy1 && py1 < uy2) {
			// Push player out to the left or right, whichever is closer
			float leftDist = std::abs(px2 - ux1);
			float rightDist = std::abs(px1 - ux2);
			if (leftDist < rightDist) {
				player.x = ux1 - avatarW;
			} else {
				player.x = ux2;
			}
		}

		// Chase camera: smoothly follow player center
		float targetCamX = player.x + avatarW / 2 - windowW / 2;
		float targetCamY = player.y + avatarH / 2 - windowH / 2;
		// Clamp to world bounds
		if (targetCamX < 0) targetCamX = 0;
		if (targetCamX > worldW - windowW) targetCamX = worldW - windowW;
		if (targetCamY < 0) targetCamY = 0;
		if (targetCamY > worldH - windowH) targetCamY = worldH - windowH;
		camXf += (targetCamX - camXf) * cameraLerp;
		camYf += (targetCamY - camYf) * cameraLerp;
		camX = static_cast<int>(camXf + 0.5f);
		camY = static_cast<int>(camYf + 0.5f);

		SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
		SDL_RenderClear(renderer);


		// Draw solid green ground rectangle (world size, but only visible part is drawn)
		SDL_Rect groundRect = {0 - camX, 0 - camY, worldW, worldH};
		SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255); // solid green (forest green)
		SDL_RenderFillRect(renderer, &groundRect);

		// Draw unwalkable area (off the island)
		SDL_Rect unwalkableRect = {unwalkableX - camX, unwalkableY - camY, unwalkableW, unwalkableH};
		SDL_SetRenderDrawColor(renderer, 200, 40, 40, 220); // red, semi-transparent
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_RenderFillRect(renderer, &unwalkableRect);
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

		// (Zone rectangle removed for chase camera)

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
		// Stylized fireballs: grow and explode (Kamehameha style)
		for (const auto& f : fireballs) {
			float t = SDL_GetTicks() / 1000.0f;
			float radius = f.size * 18.0f;
			if (!f.exploding) {
				SDL_Rect fbRect = cameraTransform(f.x - radius, f.y - radius, radius * 2, radius * 2);
				// Optimize: skip every other pixel for perf
				for (int y = 0; y < fbRect.h; y += 2) {
					for (int x = 0; x < fbRect.w; x += 2) {
						int px = fbRect.x + x;
						int py = fbRect.y + y;
						if (px < 0 || px >= windowW || py < 0 || py >= windowH) continue;
						float dx = x - fbRect.w / 2 + 0.5f;
						float dy = y - fbRect.h / 2 + 0.5f;
						float dist = std::sqrt(dx*dx + dy*dy);
						float norm = dist / (fbRect.w / 2);
						if (norm > 1.0f) continue;
						float pulse = 0.7f + 0.3f * std::sin(t * 4.0f + f.x + f.y);
						Uint8 r = (Uint8)(200 + 55 * (1.0f - norm) * pulse);
						Uint8 g = (Uint8)(40 + 30 * (1.0f - norm) * pulse);
						Uint8 b = (Uint8)(255 - 80 * norm * pulse);
						Uint8 a = (Uint8)(220 * (1.0f - norm * norm));
						SDL_SetRenderDrawColor(renderer, r, g, b, a);
						SDL_RenderDrawPoint(renderer, px, py);
					}
				}
			} else {
				// Explosion: expanding, fading ring
				float explosionProgress = f.explosionFrame / 18.0f;
				float maxExplosion = radius * 2.2f;
				float minExplosion = radius * 1.1f;
				float explosionRadius = minExplosion + (maxExplosion - minExplosion) * explosionProgress;
				SDL_Rect expRect = cameraTransform(f.x - explosionRadius, f.y - explosionRadius, explosionRadius * 2, explosionRadius * 2);
				// Optimize: skip every other pixel for perf
				for (int y = 0; y < expRect.h; y += 2) {
					for (int x = 0; x < expRect.w; x += 2) {
						int px = expRect.x + x;
						int py = expRect.y + y;
						if (px < 0 || px >= windowW || py < 0 || py >= windowH) continue;
						float dx = x - expRect.w / 2 + 0.5f;
						float dy = y - expRect.h / 2 + 0.5f;
						float dist = std::sqrt(dx*dx + dy*dy);
						float norm = dist / (expRect.w / 2);
						// Draw a ring
						if (norm > 0.7f && norm < 1.0f) {
							float fade = 1.0f - explosionProgress;
							Uint8 r = (Uint8)(255 * fade);
							Uint8 g = (Uint8)(180 * fade);
							Uint8 b = (Uint8)(255 * fade);
							Uint8 a = (Uint8)(180 * fade * (1.0f - (norm - 0.7f) / 0.3f));
							SDL_SetRenderDrawColor(renderer, r, g, b, a);
							SDL_RenderDrawPoint(renderer, px, py);
						}
					}
				}
			}
		}
		// Only render wizard's aura and particles if wizard is active
		if (playerIndex == 0) {
			// Render aura particles around player
			int auraParticleCount = 32;
			float auraRadius = (avatarW + avatarH) / 4.0f + 10.0f;
			float auraTime = SDL_GetTicks() / 1000.0f;
			// Center the aura around the player's waist (about 70% down the sprite)
			float waistY = player.y + avatarH * 0.7f;
			float centerX = player.x + avatarW / 2;
			for (int i = 0; i < auraParticleCount; ++i) {
				float angle = (2.0f * M_PI * i) / auraParticleCount + auraTime * 0.8f;
				float px = centerX + std::cos(angle) * auraRadius;
				float py = waistY + std::sin(angle) * auraRadius;
				SDL_Rect auraRect = cameraTransform(px - 3, py - 3, 6, 6);
				// Animate color for a magical effect
				Uint8 r = 120 + 80 * std::sin(angle + auraTime);
				Uint8 g = 120 + 80 * std::sin(angle + auraTime + 2.0f);
				Uint8 b = 255;
				SDL_SetRenderDrawColor(renderer, r, g, b, 180);
				SDL_RenderFillRect(renderer, &auraRect);
			}

			// Render dust particles (purple, semi-transparent, fade out)
			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
			for (const auto& d : dustParticles) {
				float alpha = 200.0f * (d.life / 32.0f);
				if (alpha < 0) alpha = 0;
				if (alpha > 200) alpha = 200;
				SDL_SetRenderDrawColor(renderer, 180, 0, 255, (Uint8)alpha);
				SDL_Rect dustRect = cameraTransform(d.x - 12, d.y - 6, 24, 12);
				// Draw as a large ellipse
				int dw = 24, dh = 12;
				for (int dy = -dh / 2; dy <= dh / 2; ++dy) {
					for (int dx = -dw / 2; dx <= dw / 2; ++dx) {
						if ((dx * dx) * (dh * dh) + (dy * dy) * (dw * dw) <= (dw * dw) * (dh * dh)) {
							int px = dustRect.x + dw / 2 + dx;
							int py = dustRect.y + dh / 2 + dy;
							if (px >= 0 && px < windowW && py >= 0 && py < windowH)
								SDL_RenderDrawPoint(renderer, px, py);
						}
					}
				}
			}
		}

		// Render character shadow (ellipse) only for wizard
		SDL_Rect destRect = cameraTransform(player.x, player.y, avatarW, avatarH);
		if (playerIndex == 0) {
			int shadowW = avatarW * 0.7;
			int shadowH = avatarH * 0.22;
			int shadowX = player.x + (avatarW - shadowW) / 2;
			int shadowY = player.y + avatarH - shadowH / 2;
			SDL_Rect shadowRect = cameraTransform(shadowX, shadowY, shadowW, shadowH);
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 90); // semi-transparent black
			for (int dy = -shadowH / 2; dy <= shadowH / 2; ++dy) {
				for (int dx = -shadowW / 2; dx <= shadowW / 2; ++dx) {
					if ((dx * dx) * (shadowH * shadowH) + (dy * dy) * (shadowW * shadowW) <= (shadowW * shadowW) * (shadowH * shadowH)) {
						int px = shadowRect.x + shadowW / 2 + dx;
						int py = shadowRect.y + shadowH / 2 + dy;
						if (px >= 0 && px < windowW && py >= 0 && py < windowH)
							SDL_RenderDrawPoint(renderer, px, py);
					}
				}
			}
			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
		}

		// Render the active character
		   // Render both wizard and knight using the wizard sprite logic, but modify knight's appearance
		   if (playerIndex == 1) {
			   // Knight: walking animation (swing legs and arms when moving)
			   float moveSpeed = std::sqrt(player.vx * player.vx + player.vy * player.vy);
			   bool isWalking = moveSpeed > 1.0f;
			   float walkSwing = isWalking ? (walkFrame == 0 ? 1.0f : -1.0f) : 0.0f;
			   // Draw main armor body (torso)
			   SDL_SetRenderDrawColor(renderer, 180, 180, 200, 255);
			   SDL_Rect torsoRect = { destRect.x + 30, destRect.y + 70, 60, 60 };
			   SDL_RenderFillRect(renderer, &torsoRect);

			   // Draw armored legs (swing)
			   SDL_SetRenderDrawColor(renderer, 160, 160, 180, 255);
			   SDL_Rect leftLeg = { destRect.x + 40 + (int)(6 * walkSwing), destRect.y + 130, 14, 28 };
			   SDL_Rect rightLeg = { destRect.x + 66 - (int)(6 * walkSwing), destRect.y + 130, 14, 28 };
			   SDL_RenderFillRect(renderer, &leftLeg);
			   SDL_RenderFillRect(renderer, &rightLeg);

			   // Draw armored boots
			   SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
			   SDL_Rect leftBoot = { destRect.x + 40 + (int)(6 * walkSwing), destRect.y + 154, 14, 10 };
			   SDL_Rect rightBoot = { destRect.x + 66 - (int)(6 * walkSwing), destRect.y + 154, 14, 10 };
			   SDL_RenderFillRect(renderer, &leftBoot);
			   SDL_RenderFillRect(renderer, &rightBoot);

			   // Draw arms (swing)
			   SDL_SetRenderDrawColor(renderer, 170, 170, 200, 255);
			   SDL_Rect leftArm = { destRect.x + 18 - (int)(6 * walkSwing), destRect.y + 90, 16, 38 };
			   SDL_Rect rightArm = { destRect.x + 86 + (int)(6 * walkSwing), destRect.y + 90, 16, 38 };
			   SDL_RenderFillRect(renderer, &leftArm);
			   SDL_RenderFillRect(renderer, &rightArm);

			   // Draw hands (gauntlets)
			   SDL_SetRenderDrawColor(renderer, 200, 200, 220, 255);
			   SDL_Rect leftHand = { destRect.x + 18 - (int)(6 * walkSwing), destRect.y + 124, 16, 12 };
			   SDL_Rect rightHand = { destRect.x + 86 + (int)(6 * walkSwing), destRect.y + 124, 16, 12 };
			   SDL_RenderFillRect(renderer, &leftHand);
			   SDL_RenderFillRect(renderer, &rightHand);

			   // Draw helmet (covers head, no wizard hat)
			   SDL_SetRenderDrawColor(renderer, 180, 180, 220, 255);
			   SDL_Rect helmetRect = { destRect.x + 38, destRect.y + 30, 44, 36 };
			   SDL_RenderFillRect(renderer, &helmetRect);
			   // Visor (darker slit)
			   SDL_SetRenderDrawColor(renderer, 60, 60, 80, 255);
			   SDL_Rect visorRect = { destRect.x + 48, destRect.y + 48, 24, 8 };
			   SDL_RenderFillRect(renderer, &visorRect);

			   // Draw face (small slit below visor)
			   SDL_SetRenderDrawColor(renderer, 220, 200, 160, 255);
			   SDL_Rect faceRect = { destRect.x + 54, destRect.y + 56, 12, 8 };
			   SDL_RenderFillRect(renderer, &faceRect);

			   // Draw sword in right hand (pointing up at 45 degrees, thicker and longer)
			   float swordLength = 60.0f;
			   float swordAngle = -M_PI / 4.0f; // 45 degrees up
			   float handX = destRect.x + 86 + 16 + (int)(6 * walkSwing);
			   float handY = destRect.y + 130;
			   float swordTipX = handX + swordLength * std::cos(swordAngle);
			   float swordTipY = handY + swordLength * std::sin(swordAngle);
			   SDL_SetRenderDrawColor(renderer, 200, 200, 220, 255);
			   for (int t = -2; t <= 2; ++t) {
				   SDL_RenderDrawLine(renderer, (int)handX + t, (int)handY, (int)swordTipX + t, (int)swordTipY);
			   }
			   float hiltLen = 18.0f;
			   float hiltAngle = swordAngle + M_PI/2.0f;
			   float hiltX1 = handX - hiltLen/2 * std::cos(hiltAngle);
			   float hiltY1 = handY - hiltLen/2 * std::sin(hiltAngle);
			   float hiltX2 = handX + hiltLen/2 * std::cos(hiltAngle);
			   float hiltY2 = handY + hiltLen/2 * std::sin(hiltAngle);
			   SDL_SetRenderDrawColor(renderer, 120, 80, 40, 255);
			   for (int t = -1; t <= 1; ++t) {
				   SDL_RenderDrawLine(renderer, (int)hiltX1 + t, (int)hiltY1, (int)hiltX2 + t, (int)hiltY2);
			   }
		   } else {
			// Wizard: use default sprite logic
			if (facing == 2 && spriteBackTexture[walkFrame]) {
				SDL_RenderCopy(renderer, spriteBackTexture[walkFrame], nullptr, &destRect);
			} else if (facing == 3 && spriteFrontTexture[walkFrame]) {
				SDL_RenderCopy(renderer, spriteFrontTexture[walkFrame], nullptr, &destRect);
			} else if ((facing == 0 || facing == 1) && spriteSideTexture[walkFrame]) {
				SDL_RendererFlip flip = (facing == 1) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
				SDL_RenderCopyEx(renderer, spriteSideTexture[walkFrame], nullptr, &destRect, 0, nullptr, flip);
			}
		}
		// --- Mini Map Rendering ---
		const int miniMapW = 240;
		const int miniMapH = 160;
		const int miniMapMargin = 24;
		// Top-right corner
		int miniMapX = windowW - miniMapW - miniMapMargin;
		int miniMapY = miniMapMargin;
		// Draw minimap background
		SDL_Rect miniMapRect = { miniMapX, miniMapY, miniMapW, miniMapH };
		SDL_SetRenderDrawColor(renderer, 20, 20, 30, 220);
		SDL_RenderFillRect(renderer, &miniMapRect);
		// Draw minimap border
		SDL_SetRenderDrawColor(renderer, 200, 200, 255, 255);
		SDL_RenderDrawRect(renderer, &miniMapRect);

		// World-to-minimap scale
		float scaleX = (float)miniMapW / worldW;
		float scaleY = (float)miniMapH / worldH;


		// (Zone rectangle on minimap removed for chase camera)

		// Draw grid lines on minimap
		SDL_SetRenderDrawColor(renderer, 80, 80, 120, 180);
		for (int gx = 0; gx <= worldW; gx += 128) {
			int x = miniMapX + (int)(gx * scaleX);
			SDL_RenderDrawLine(renderer, x, miniMapY, x, miniMapY + miniMapH);
		}
		for (int gy = 0; gy <= worldH; gy += 128) {
			int y = miniMapY + (int)(gy * scaleY);
			SDL_RenderDrawLine(renderer, miniMapX, y, miniMapX + miniMapW, y);
		}

		// Draw player position on minimap
		int playerMiniX = miniMapX + (int)((player.x + avatarW / 2) * scaleX);
		int playerMiniY = miniMapY + (int)((player.y + avatarH / 2) * scaleY);
		SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
		SDL_Rect playerDot = { playerMiniX - 3, playerMiniY - 3, 7, 7 };
		SDL_RenderFillRect(renderer, &playerDot);

		// Optionally: draw fireballs on minimap
		SDL_SetRenderDrawColor(renderer, 180, 0, 255, 200);
		for (const auto& f : fireballs) {
			int fx = miniMapX + (int)(f.x * scaleX);
			int fy = miniMapY + (int)(f.y * scaleY);
			SDL_Rect fbDot = { fx - 2, fy - 2, 4, 4 };
			SDL_RenderFillRect(renderer, &fbDot);
		}

		// Energy regeneration
		if (playerEnergyF < playerMaxEnergy) {
			playerEnergyF += energyRegenPerFrame;
			if (playerEnergyF > playerMaxEnergy) playerEnergyF = (float)playerMaxEnergy;
			playerEnergy = (int)playerEnergyF;
		}

		// --- HUD Rendering ---
		// Health bar
		int hudBarW = 320, hudBarH = 28, hudBarPad = 8;
		int healthBarX = 32, healthBarY = 32;
		float healthFrac = (float)playerHealth / playerMaxHealth;
		SDL_Rect healthBack = {healthBarX, healthBarY, hudBarW, hudBarH};
		SDL_Rect healthFill = {healthBarX + hudBarPad, healthBarY + hudBarPad, (int)((hudBarW - 2 * hudBarPad) * healthFrac), hudBarH - 2 * hudBarPad};
		// Background
		SDL_SetRenderDrawColor(renderer, 40, 40, 40, 220);
		SDL_RenderFillRect(renderer, &healthBack);
		// Fill
		SDL_SetRenderDrawColor(renderer, 220, 40, 40, 255);
		SDL_RenderFillRect(renderer, &healthFill);
		// Border
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderDrawRect(renderer, &healthBack);

		// Energy bar (below health)
		int energyBarX = 32, energyBarY = 32 + hudBarH + 12;
		float energyFrac = playerEnergyF / playerMaxEnergy;
		SDL_Rect energyBack = {energyBarX, energyBarY, hudBarW, hudBarH};
		SDL_Rect energyFill = {energyBarX + hudBarPad, energyBarY + hudBarPad, (int)((hudBarW - 2 * hudBarPad) * energyFrac), hudBarH - 2 * hudBarPad};
		// Background
		SDL_SetRenderDrawColor(renderer, 40, 40, 40, 220);
		SDL_RenderFillRect(renderer, &energyBack);
		// Fill
		SDL_SetRenderDrawColor(renderer, 60, 180, 255, 255);
		SDL_RenderFillRect(renderer, &energyFill);
		// Border
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderDrawRect(renderer, &energyBack);

		// Draw FPS in bottom left
		if (fpsTexture) {
			SDL_Rect fpsRect = {8, windowH - fpsTextH - 8, fpsTextW, fpsTextH};
			SDL_RenderCopy(renderer, fpsTexture, nullptr, &fpsRect);
		}

		SDL_RenderPresent(renderer);
	}

	// Cleanup FPS resources after main loop
	if (fpsTexture) {
		SDL_DestroyTexture(fpsTexture);
		fpsTexture = nullptr;
	}
	if (fpsFont) {
		TTF_CloseFont(fpsFont);
		fpsFont = nullptr;
	}
	TTF_Quit();
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
