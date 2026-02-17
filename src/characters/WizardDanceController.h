#pragma once
#include <SDL2/SDL.h>

class WizardDanceController {
public:
    bool isDancing = false;
    Uint32 danceStartTime = 0;

    void handleEvent(const SDL_Event& event);
    void update();
};
