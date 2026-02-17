#pragma once
#include <SDL2/SDL.h>
#include <map>
#include <string>

// Maps action names (e.g. "attack") to SDL_Keycode
class ControllerProfile {
public:
    std::map<std::string, SDL_Keycode> keyBindings;

    ControllerProfile() {}
    void setBinding(const std::string& action, SDL_Keycode key) {
        keyBindings[action] = key;
    }
    SDL_Keycode getBinding(const std::string& action) const {
        auto it = keyBindings.find(action);
        if (it != keyBindings.end()) return it->second;
        return SDLK_UNKNOWN;
    }
    // Check if an event matches an action
    bool isAction(const SDL_Event& event, const std::string& action) const {
        if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
            return event.key.keysym.sym == getBinding(action);
        }
        return false;
    }
};
