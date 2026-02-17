#include "Knight.h"
#include <cmath>
#include <algorithm>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "../engine/ControllerProfile.h"

Knight::Knight(SDL_Renderer* renderer, TTF_Font* font, int windowW, int windowH) {
	x = windowW / 2.0f;
	y = windowH / 2.0f;
	vx = vy = 0.0f;
	facing = 0;
	walkFrame = 0; walkCounter = 0;
	health = maxHealth = 120;
	stamina = maxStamina = 100;
	staminaF = (float)stamina;
	width = 120; height = 160;
	headBobVals[0] = -1; headBobVals[1] = 1;
	helmetTiltVals[0] = -1; helmetTiltVals[1] = 1;
	// No textures for now (placeholder)
	controllerProfile.setBinding("attack", SDLK_z);
	controllerProfile.setBinding("special", SDLK_x);
	controllerProfile.setBinding("slam", SDLK_SPACE);
	controllerProfile.setBinding("jump", SDLK_c);
}

Knight::~Knight() {}

void Knight::handleEvent(const SDL_Event& event) {
	if (controllerProfile.isAction(event, "attack")) {
		// Normal attack
		printf("Knight performs ATTACK!\n");
		// TODO: Implement attack logic
	} else if (controllerProfile.isAction(event, "special")) {
		// Special attack
		printf("Knight performs SPECIAL!\n");
		// TODO: Implement special logic
	} else if (controllerProfile.isAction(event, "slam")) {
		// Spin move (spacebar)
		if (!knightSpin.spinning) {
			knightSpin.spinning = true;
			knightSpin.spinStartTime = SDL_GetTicks();
			knightSpin.spinAngle = 0.0f;
			printf("Knight performs SPIN MOVE!\n");
		}
	} else if (controllerProfile.isAction(event, "jump")) {
		printf("Knight JUMPS!\n");
		// TODO: Implement jump logic
	}
	// No input handling (non-interactive demo)
}

void Knight::handleInput(const Uint8* keystate) {
    // No input handling (non-interactive demo)
}

void Knight::update() {
	x += vx;
	y += vy;
	vx *= 0.92f;
	vy *= 0.92f;
	walkCounter++;
	if (walkCounter >= 20) {
		walkCounter = 0;
		walkFrame = 1 - walkFrame;
	}
	// Update energy blades
	for (auto& b : energyBlades) {
		if (!b.active) continue;
		b.x += b.vx;
		b.y += b.vy;
		b.life--;
		if (b.life <= 0) b.active = false;
	}
	energyBlades.erase(std::remove_if(energyBlades.begin(), energyBlades.end(), [](const EnergyBlade& b) { return !b.active; }), energyBlades.end());

	// Update spin move
	if (knightSpin.spinning) {
		Uint32 now = SDL_GetTicks();
		float spinDuration = 500.0f; // ms
		float elapsed = (float)(now - knightSpin.spinStartTime);
		knightSpin.spinAngle += 0.45f * M_PI; // fast spin
		if (elapsed > spinDuration) {
			knightSpin.spinning = false;
			knightSpin.spinAngle = 0.0f;
		}
	}
}

void Knight::render(SDL_Renderer* renderer, int camX, int camY, int windowW, int windowH) {
	// Draw knight sprite (placeholder: colored rectangle)
	SDL_Rect destRect = { (int)(x - camX), (int)(y - camY), width, height };
	SDL_SetRenderDrawColor(renderer, 120, 120, 130, 255); // main color
	SDL_RenderFillRect(renderer, &destRect);

	// (Removed energy blade rendering for spin-only visual)
	// Sword spin (visual only, handled in render)
	if (knightSpin.spinning) {
		// Draw sword spin effect (placeholder: blue arc)
		float cx = x + width / 2 - camX;
		float cy = y + height / 2 - camY;
		float swordAngle = knightSpin.spinAngle;
		float swordLength = 60.0f;
		float handX = cx + std::cos(swordAngle) * 60.0f;
		float handY = cy + std::sin(swordAngle) * 60.0f;
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
	}
}

int Knight::getHealth() const { return health; }
int Knight::getMaxHealth() const { return maxHealth; }
int Knight::getStamina() const { return stamina; }
int Knight::getMaxStamina() const { return maxStamina; }
float Knight::getX() const { return x; }
float Knight::getY() const { return y; }
int Knight::getWidth() const { return width; }
int Knight::getHeight() const { return height; }
int Knight::getFacing() const { return facing; }
void Knight::setPosition(float nx, float ny) { x = nx; y = ny; }
void Knight::setVelocity(float nvx, float nvy) { vx = nvx; vy = nvy; }
void Knight::setFacing(int f) { facing = f; }
