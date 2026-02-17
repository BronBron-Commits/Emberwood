#include "WizardDanceController.h"

void WizardDanceController::handleEvent(const SDL_Event& event) {
    // Dance trigger: press 1 to dance for 5 seconds
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_1) {
        isDancing = true;
        danceStartTime = SDL_GetTicks();
    }
}

void WizardDanceController::update() {
    // Stop dancing after 5 seconds
    if (isDancing && SDL_GetTicks() - danceStartTime > 5000) {
        isDancing = false;
    }
}
