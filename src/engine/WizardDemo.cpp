#include "WizardDemo.h"
#include "../characters/Wizard.h"

WizardDemo::WizardDemo(SDL_Renderer* renderer, int windowW, int windowH, TTF_Font* font)
    : renderer(renderer), windowW(windowW), windowH(windowH), font(font)
{
    x = windowW / 2.0f;
    y = windowH / 2.0f;
    width = 120;
    height = 160;
    wizard = new Wizard(renderer, font, windowW, windowH);
    wizard->setPosition(x, y);
}

void WizardDemo::render() {
    wizard->render(renderer, 0, 0, windowW, windowH);
}

void WizardDemo::handleEvent(const SDL_Event& event) {
    wizard->handleEvent(event);
}
