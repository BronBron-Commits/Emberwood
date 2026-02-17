#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include "entities/AOEEffect.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <vector>


#include "CharacterModule.h"


#include <sstream>

// --- Boomerang Energy Bolt State ---
struct BoomerangBolt {
	float x, y;
	float vx, vy;
	float startX, startY;
	float targetX, targetY;
	float traveled;
	bool returning;
	bool active;
	BoomerangBolt(float sx, float sy, float tx, float ty)
		: x(sx), y(sy), startX(sx), startY(sy), targetX(tx), targetY(ty), traveled(0), returning(false), active(true) {
		float dx = tx - sx;
		float dy = ty - sy;
		float dist = std::sqrt(dx*dx + dy*dy);
		float maxDist = 420.0f;
		if (dist > maxDist) { dx *= maxDist/dist; dy *= maxDist/dist; }
		float speed = 18.0f; // pixels per frame
		float mag = std::sqrt(dx*dx + dy*dy);
		vx = dx / mag * speed;
		vy = dy / mag * speed;
	}
	void update(float playerX, float playerY) {
		if (!active) return;
		if (!returning) {
			x += vx;
			y += vy;
			traveled += std::sqrt(vx*vx + vy*vy);
			// If reached or passed max distance, start returning
			float dx = x - startX;
			float dy = y - startY;
			float maxDist = 420.0f;
			if (traveled >= maxDist) {
				returning = true;
			}
		} else {
			// Home in on player
			float dx = playerX - x;
			float dy = playerY - y;
			float dist = std::sqrt(dx*dx + dy*dy);
			float speed = 22.0f;
			if (dist < speed) {
				active = false;
				return;
			}
			vx = dx / dist * speed;
			vy = dy / dist * speed;
			x += vx;
			y += vy;
		}
	}
	void render(SDL_Renderer* renderer, int camX, int camY, int windowW, int windowH) {
		if (!active) return;
		int cx = (int)x - camX;
		int cy = (int)y - camY;
		int r = 18;
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
		for (int ring = r; ring > r-6; --ring) {
			SDL_SetRenderDrawColor(renderer, 60, 220, 255, 180);
			for (int deg = 0; deg < 360; ++deg) {
				float rad = deg * 3.14159265358979323846f / 180.0f;
				int px = cx + (int)(ring * std::cos(rad));
				int py = cy + (int)(ring * std::sin(rad));
				if (px >= 0 && py >= 0 && px < windowW && py < windowH)
					SDL_RenderDrawPoint(renderer, px, py);
			}
		}
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	}
};
static std::vector<BoomerangBolt> boomerangBolts;

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





int main(int argc, char* argv[]) {
			// League of Legends style right-click walk
			bool hasWalkTarget = false;
			float walkTargetX = 0.0f, walkTargetY = 0.0f;

			// --- Floating X indicator state ---
			bool showWalkX = false;
			float walkX = 0.0f, walkY = 0.0f;
			float walkXAnimOffset = 0.0f;
			int walkXAnimTimer = 0;
			const int walkXAnimDuration = 36; // ~0.3s at 120fps

			// --- Minimap walk target indicator ---
			bool showMinimapX = false;
			float minimapX = 0.0f, minimapY = 0.0f;
		// --- Frame timing for 120fps ---
		const int targetFPS = 120;
		const Uint32 frameDelay = 1000 / targetFPS;
		Uint32 frameStart = 0;
		Uint32 frameTime = 0;
// --- Wizard Q Attack State ---
bool wizardQAttack = false;
int wizardQAttackTimer = 0;
const int wizardQAttackDuration = 18; // frames (about 0.15s at 120fps)
std::vector<AOEEffect> aoeEffects;
	// --- No frame cap: run as fast as possible ---
	// (Removed frame timing and delay logic)

	bool showGrid = true;
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
	// Energy regeneration per frame
	const int energyRegenPerFrame = 2; // integer regen per frame
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

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
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
	const float maxSpeed = 4.0f; // Slower right-click walk speed
	const float friction = 0.92f;
	const int margin = 100;
	int facing = 0;
	int walkFrame = 0;
	int walkCounter = 0;
	// At 120fps, double frame delay to keep animation speed the same
	const int walkFrameDelay = 20;
	// Removed fireball vector
	std::vector<DustParticle> dustParticles;
	int dustEmitCooldown = 0;

	bool running = true;
	SDL_Event event;
	while (running) {
				frameStart = SDL_GetTicks();
			// No frame timing: run loop as fast as possible
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
		// WASD movement removed
		while (SDL_PollEvent(&event)) {
						// Right-click to set walk target
						if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {
							int mx = event.button.x;
							int my = event.button.y;
							// Convert screen to world coordinates
							walkTargetX = mx + camX;
							walkTargetY = my + camY;
							hasWalkTarget = true;
							// Show floating X indicator
							showWalkX = true;
							walkX = walkTargetX;
							walkY = walkTargetY;
							walkXAnimOffset = 0.0f;
							walkXAnimTimer = walkXAnimDuration;
							// Show minimap X
							showMinimapX = true;
							minimapX = walkTargetX;
							minimapY = walkTargetY;
								// --- Floating X animation update ---
								if (showWalkX) {
									walkXAnimOffset -= 1.2f; // Move up each frame
									walkXAnimTimer--;
									if (walkXAnimTimer <= 0) {
										showWalkX = false;
									}
								}
								// --- Render floating X indicator ---
								if (showWalkX) {
									SDL_Rect xRect = cameraTransform(walkX - 12, walkY - 12 + walkXAnimOffset, 24, 24);
									// Draw a simple red 'X'
									SDL_SetRenderDrawColor(renderer, 255, 40, 40, 255);
									for (int i = 0; i < 4; ++i) {
										SDL_RenderDrawLine(renderer, xRect.x + 3 + i, xRect.y + 3, xRect.x + xRect.w - 4 - i, xRect.y + xRect.h - 4);
										SDL_RenderDrawLine(renderer, xRect.x + 3 + i, xRect.y + xRect.h - 4, xRect.x + xRect.w - 4 - i, xRect.y + 3);
									}
								}
						}
			if (event.type == SDL_QUIT) {
				running = false;
			} else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
				bool pressed = (event.type == SDL_KEYDOWN);
				switch (event.key.keysym.sym) {
															// Wizard W attack: boomerang energy bolt
															case SDLK_w:
																if (pressed && playerIndex == 0) {
																	// Only allow one boomerang at a time
																	boomerangBolts.clear();
																	float boltX = characters[playerIndex].x + avatarW / 2;
																	float boltY = characters[playerIndex].y + avatarH / 2;
																	// Throw forward in facing direction
																	float dx = 0.0f, dy = 0.0f;
																	switch (characters[playerIndex].facing) {
																		case 0: dx = 1.0f; break; // right
																		case 1: dx = -1.0f; break; // left
																		case 2: dy = -1.0f; break; // up
																		case 3: dy = 1.0f; break; // down
																	}
																	float dist = 420.0f;
																	float tx = boltX + dx * dist;
																	float ty = boltY + dy * dist;
																	// Debug: print boomerang start and target
																	std::cout << "Boomerang thrown from (" << boltX << ", " << boltY << ") to (" << tx << ", " << ty << ")\n";
																	boomerangBolts.emplace_back(boltX, boltY, tx, ty);
																}
																break;
										case SDLK_g:
											if (pressed) showGrid = !showGrid;
											break;
					// WASD movement removed
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

					// Wizard Q attack
					      case SDLK_q:
						   if (pressed && playerIndex == 0 && !wizardQAttack) {
							   wizardQAttack = true;
							   wizardQAttackTimer = wizardQAttackDuration;
							   // Ensure only one AOE exists at a time
							   aoeEffects.clear();
							   float aoeX = characters[playerIndex].x + avatarW / 2;
							   float aoeY = characters[playerIndex].y + avatarH / 2;
							   aoeEffects.emplace_back(aoeX, aoeY, wizardQAttackDuration, 48);
						   }
						   break;
				}
				if (pressed && event.key.keysym.sym == SDLK_SPACE) {
					// (Fireball shooting removed)
				}
			}
		}
					// League of Legends style right-click walk
					Character& player = characters[playerIndex];

					// --- Update boomerang bolts (W attack) ---
					float playerCenterX = player.x + avatarW / 2;
					float playerCenterY = player.y + avatarH / 2;
					for (auto& bolt : boomerangBolts) bolt.update(playerCenterX, playerCenterY);
					boomerangBolts.erase(std::remove_if(boomerangBolts.begin(), boomerangBolts.end(), [](const BoomerangBolt& b){ return !b.active; }), boomerangBolts.end());

					// Instantly trigger AOE attack effect if wizardQAttack was just set
					static bool aoeTriggered = false;
					static float aoeX = 0.0f, aoeY = 0.0f;
					if (wizardQAttack && !aoeTriggered) {
							aoeX = player.x + avatarW / 2;
							aoeY = player.y + avatarH / 2;
							std::cout << "AOE attack triggered at (" << aoeX << ", " << aoeY << ")" << std::endl;
							aoeTriggered = true;
					}
				if (!wizardQAttack) aoeTriggered = false;
				// (AOE rendering moved below grass rendering)
		if (hasWalkTarget) {
			float dx = walkTargetX - (player.x + avatarW / 2);
			float dy = walkTargetY - (player.y + avatarH / 2);
			float dist = std::sqrt(dx * dx + dy * dy);
			if (dist > 6.0f) {
				float move = std::min(maxSpeed, dist);
				float mx = dx / dist * move;
				float my = dy / dist * move;
				player.x += mx;
				player.y += my;
				// Set facing only if actually moving
				if (std::abs(mx) > std::abs(my)) {
					player.facing = (mx > 0) ? 0 : 1;
				} else if (std::abs(my) > 0.01f) {
					player.facing = (my > 0) ? 3 : 2;
				}
			} else {
				hasWalkTarget = false;
			}
		}

		// Animation: always advance wizard walk frame if wizard is moving
		float wizardMoveSpeed = std::sqrt(characters[0].vx * characters[0].vx + characters[0].vy * characters[0].vy);
		if (wizardMoveSpeed > 1.0f) {
			walkCounter++;
			if (walkCounter >= walkFrameDelay) {
				walkCounter = 0;
				walkFrame = 1 - walkFrame;
			}
		} else {
			walkFrame = 0; // idle pose
		}

		// Q attack timer update
		if (wizardQAttack) {
			wizardQAttackTimer--;
			if (wizardQAttackTimer <= 0) {
				wizardQAttack = false;
			}
		}

		// Emit dust trail if moving
		if (maxSpeed > 1.0f && dustEmitCooldown <= 0) {
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

		// Update and remove inactive AOEs
		for (auto& aoe : aoeEffects) {
			aoe.update();
		}
		aoeEffects.erase(std::remove_if(aoeEffects.begin(), aoeEffects.end(), [](const AOEEffect& aoe) { return !aoe.active; }), aoeEffects.end());
		// (Fireball update removed)

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

		// --- Courtyard Pavers around Campfire ---
		int courtyardCenterX = worldW / 2;
		int courtyardCenterY = worldH / 2 + 120;
		int numRings = 3;
		int ringSpacing = 36;
		int paverW = 18, paverH = 18;
		// Draw concentric rings
		for (int ring = 1; ring <= numRings; ++ring) {
			float radius = ring * ringSpacing;
			int numPavers = 16 + ring * 8;
			for (int i = 0; i < numPavers; ++i) {
				float angle = (2 * M_PI * i) / numPavers;
				float px = courtyardCenterX + std::cos(angle) * radius;
				float py = courtyardCenterY + std::sin(angle) * radius;
				SDL_Rect paverRect = cameraTransform(px - paverW/2, py - paverH/2, paverW, paverH);
				SDL_SetRenderDrawColor(renderer, 200, 200, 210, 255); // light gray
				SDL_RenderFillRect(renderer, &paverRect);
			}
		}
		// Draw radial spokes
		int numSpokes = 8;
		for (int s = 0; s < numSpokes; ++s) {
			float angle = (2 * M_PI * s) / numSpokes;
			for (int r = ringSpacing; r <= numRings * ringSpacing; r += ringSpacing/2) {
				float px = courtyardCenterX + std::cos(angle) * r;
				float py = courtyardCenterY + std::sin(angle) * r;
				SDL_Rect paverRect = cameraTransform(px - paverW/2, py - paverH/2, paverW, paverH);
				SDL_SetRenderDrawColor(renderer, 180, 180, 200, 255); // slightly darker
				SDL_RenderFillRect(renderer, &paverRect);
			}
		}

		// --- Performance-Optimized Campfire rendering ---
		int campfireX = worldW / 2 - 60;
		int campfireY = worldH / 2 + 100;
		SDL_Rect campfireRect = cameraTransform(campfireX, campfireY, 120, 60);
		// Draw glowing aura (fewer, larger steps)
		for (int r = 80; r > 60; r -= 10) {
			SDL_SetRenderDrawColor(renderer, 255, 200, 80, 24);
			for (int dy = -r; dy <= r; dy += 2) {
				for (int dx = -r; dx <= r; dx += 2) {
					if (dx*dx + dy*dy <= r*r) {
						int px = campfireRect.x + 60 + dx;
						int py = campfireRect.y + 30 + dy;
						if (px >= 0 && px < windowW && py >= 0 && py < windowH)
							SDL_RenderDrawPoint(renderer, px, py);
					}
				}
			}
		}
		// Draw campfire base (fewer logs, smaller)
		SDL_SetRenderDrawColor(renderer, 110, 70, 30, 255);
		for (int i = 0; i < 4; ++i) {
			int logY = campfireRect.y + 32 + i * 5;
			SDL_Rect logRect = {campfireRect.x + 12 + i * 16, logY, 80 - i * 16, 10};
			SDL_RenderFillRect(renderer, &logRect);
		}
		// Animate fire (smaller, fewer layers)
		float fireTime = SDL_GetTicks() / 300.0f;
		int flameX = campfireRect.x + 60 + (int)(10 * std::sin(fireTime));
		int flameY = campfireRect.y + 20 + (int)(5 * std::cos(fireTime * 1.7f));
		// Draw outer flame (orange, fewer radii)
		SDL_SetRenderDrawColor(renderer, 255, 140, 40, 200);
		for (int r = 24; r > 12; r -= 4) {
			for (int dy = -r; dy <= r; dy += 2) {
				for (int dx = -r; dx <= r; dx += 2) {
					if (dx*dx + dy*dy <= r*r) {
						int px = flameX + dx;
						int py = flameY + dy;
						if (px >= 0 && px < windowW && py >= 0 && py < windowH)
							SDL_RenderDrawPoint(renderer, px, py);
					}
				}
			}
		}
		// Draw inner flame (yellow, fewer radii)
		SDL_SetRenderDrawColor(renderer, 255, 220, 80, 220);
		for (int r = 12; r > 4; r -= 4) {
			for (int dy = -r; dy <= r; dy += 2) {
				for (int dx = -r; dx <= r; dx += 2) {
					if (dx*dx + dy*dy <= r*r) {
						int px = flameX + dx;
						int py = flameY + dy;
						if (px >= 0 && px < windowW && py >= 0 && py < windowH)
							SDL_RenderDrawPoint(renderer, px, py);
					}
				}
			}
		}
		// Draw core flame (white, smallest)
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		for (int r = 4; r > 1; r -= 2) {
			for (int dy = -r; dy <= r; ++dy) {
				for (int dx = -r; dx <= r; ++dx) {
					if (dx*dx + dy*dy <= r*r) {
						int px = flameX + dx;
						int py = flameY + dy;
						if (px >= 0 && px < windowW && py >= 0 && py < windowH)
							SDL_RenderDrawPoint(renderer, px, py);
					}
				}
			}
		}

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


		// --- Render boomerang bolts (above grass, below AOE) ---
		for (auto& bolt : boomerangBolts) bolt.render(renderer, camX, camY, windowW, windowH);
		// (AOE rendering moved to top layer)

		// --- Bushes only (trees removed) ---
		int campfireCenterX = worldW / 2;
		int campfireCenterY = worldH / 2 + 144;
		int campfireRadius = 180;

		// Bushes
		int numBushes = 18;
		for (int i = 0; i < numBushes; ++i) {
			int bx = (i * 54321 + 33333) % worldW;
			int by = (i * 87654 + 22222) % worldH;
			// Avoid campfire area
			int dx = bx - campfireCenterX;
			int dy = by - campfireCenterY;
			if (dx*dx + dy*dy < (campfireRadius-30)*(campfireRadius-30)) continue;
			int bushR = 22 + (i % 2) * 8;
			int camCenterX = camX + windowW / 2;
			int camCenterY = camY + windowH / 2;
			int dist2 = (bx - camCenterX) * (bx - camCenterX) + (by - camCenterY) * (by - camCenterY);
			bool lowDetail = dist2 > 600*600;
			int cx = bx - camX;
			int cy = by - camY;
			// Culling: skip if bush is completely outside the window
			int bushMinX = cx - bushR, bushMaxX = cx + bushR;
			int bushMinY = cy - bushR, bushMaxY = cy + bushR;
			if (bushMaxX < 0 || bushMinX > windowW || bushMaxY < 0 || bushMinY > windowH) continue;
			if (lowDetail) {
				// Simple small circle for far bushes
				int lodR = bushR * 0.5;
				SDL_SetRenderDrawColor(renderer, 60, 180, 60, 255);
				for (int fy = -lodR; fy <= lodR; ++fy) {
					for (int fx = -lodR; fx <= lodR; ++fx) {
						if (fx*fx + fy*fy <= lodR*lodR) {
							int px = cx + fx;
							int py = cy + fy;
							if (px >= 0 && px < windowW && py >= 0 && py < windowH)
								SDL_RenderDrawPoint(renderer, px, py);
						}
					}
				}
			} else {
				SDL_SetRenderDrawColor(renderer, 60, 180, 60, 255);
				for (int fy = -bushR; fy <= bushR; ++fy) {
					for (int fx = -bushR; fx <= bushR; ++fx) {
						if (fx*fx + fy*fy <= bushR*bushR) {
							int px = cx + fx;
							int py = cy + fy;
							if (px >= 0 && px < windowW && py >= 0 && py < windowH)
								SDL_RenderDrawPoint(renderer, px, py);
						}
					}
				}
			}
		}

		// Draw grid overlay on top of grass if enabled
		if (showGrid) {
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
		}


		// (Castle rendering removed)
		// (Fireball rendering removed)
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
				// --- Render AOE visual on top of everything ---
				if (wizardQAttack) {
					for (auto& aoe : aoeEffects) {
						aoe.render(renderer, camX, camY, windowW, windowH);
					}
				}
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
			// Wizard: use default sprite logic, with Q attack animation (arm raise)
			if (wizardQAttack && playerIndex == 0) {
				// Draw Q attack animation: overlay a raised arm (simple rectangle or line)
				if (player.facing == 2 && spriteBackTexture[walkFrame]) {
					SDL_RenderCopy(renderer, spriteBackTexture[walkFrame], nullptr, &destRect);
				} else if (player.facing == 3 && spriteFrontTexture[walkFrame]) {
					SDL_RenderCopy(renderer, spriteFrontTexture[walkFrame], nullptr, &destRect);
				} else if ((player.facing == 0 || player.facing == 1) && spriteSideTexture[walkFrame]) {
					SDL_RendererFlip flip = (player.facing == 1) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
					SDL_RenderCopyEx(renderer, spriteSideTexture[walkFrame], nullptr, &destRect, 0, nullptr, flip);
				}
				// Overlay: draw a raised arm (simple rectangle)
				SDL_SetRenderDrawColor(renderer, 255, 220, 180, 255);
				SDL_Rect armRect;
				if (player.facing == 0) { // right
					armRect = { destRect.x + 90, destRect.y + 60, 18, 38 };
				} else if (player.facing == 1) { // left
					armRect = { destRect.x + 12, destRect.y + 60, 18, 38 };
				} else if (player.facing == 3) { // front
					armRect = { destRect.x + 60 - 28, destRect.y + 40, 18, 38 };
				} else { // back
					armRect = { destRect.x + 60 - 9, destRect.y + 40, 18, 38 };
				}
				SDL_RenderFillRect(renderer, &armRect);
			} else {
				if (player.facing == 2 && spriteBackTexture[walkFrame]) {
					SDL_RenderCopy(renderer, spriteBackTexture[walkFrame], nullptr, &destRect);
				} else if (player.facing == 3 && spriteFrontTexture[walkFrame]) {
					SDL_RenderCopy(renderer, spriteFrontTexture[walkFrame], nullptr, &destRect);
				} else if ((player.facing == 0 || player.facing == 1) && spriteSideTexture[walkFrame]) {
					SDL_RendererFlip flip = (player.facing == 1) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
					SDL_RenderCopyEx(renderer, spriteSideTexture[walkFrame], nullptr, &destRect, 0, nullptr, flip);
				}
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

		// Draw walk target X on minimap
		if (showMinimapX) {
			int xMini = miniMapX + (int)(minimapX * scaleX);
			int yMini = miniMapY + (int)(minimapY * scaleY);
			SDL_SetRenderDrawColor(renderer, 255, 40, 40, 255);
			for (int i = 0; i < 2; ++i) {
				SDL_RenderDrawLine(renderer, xMini - 7 + i, yMini - 7, xMini + 7 - i, yMini + 7);
				SDL_RenderDrawLine(renderer, xMini - 7 + i, yMini + 7, xMini + 7 - i, yMini - 7);
			}
		}

		// (Fireball minimap rendering removed)

		// Energy regeneration
		if (playerEnergy < playerMaxEnergy) {
			playerEnergy += energyRegenPerFrame;
			if (playerEnergy > playerMaxEnergy) playerEnergy = playerMaxEnergy;
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
		float energyFrac = (float)playerEnergy / playerMaxEnergy;
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

		// --- Frame cap for 120fps ---
		frameTime = SDL_GetTicks() - frameStart;
		if (frameTime < frameDelay) {
			SDL_Delay(frameDelay - frameTime);
		}
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
