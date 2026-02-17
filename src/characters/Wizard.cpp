#include <utility>
#include <vector>
#include <SDL2/SDL.h>
#include "Wizard.h"
#include "../engine/ControllerProfile.h"
Wizard::Wizard(SDL_Renderer* renderer, TTF_Font* font, int windowW, int windowH) {
    // TODO: Initialize member variables, mesh, bones, etc.
    width = 120;
    height = 160;
    x = windowW / 2.0f;
    y = windowH / 2.0f;
    vx = vy = 0.0f;
    health = maxHealth = 100;
    energy = maxEnergy = 80;
    energyF = (float)energy;
    facing = 0;
    walkFrame = walkCounter = 0;
    currentOutfit = 0;
    controllerProfile.setBinding("q_attack", SDLK_q);
    controllerProfile.setBinding("w_attack", SDLK_w);
    controllerProfile.setBinding("e_attack", SDLK_e);
    controllerProfile.setBinding("r_ult", SDLK_r);
    // Optionally initialize mesh/bones here
    animator = std::make_unique<Animator>();
    // Q attack animation: simple arm raise for 0.2s, then return
    qAttackAnim = std::make_shared<Animation>(std::vector<AnimationFrame>{
        AnimationFrame{0.12f, [this](float){ boneRArm.setAngle(-M_PI/2); }},
        AnimationFrame{0.08f, [this](float){ boneRArm.setAngle(0); }}
    });
}

void Wizard::handleEvent(const SDL_Event& event) {
    if (controllerProfile.isAction(event, "q_attack")) {
        printf("Wizard Q attack!\n");
        // Start Q attack animation
        animator->setAnimation(qAttackAnim);
        qAttackActive = true;
        qAttackStartTime = SDL_GetTicks();
        // Update Q attack animation if active
        if (qAttackActive) {
            Uint32 now = SDL_GetTicks();
            float elapsed = (now - qAttackStartTime) / 1000.0f;
            animator->update(1.0f / 60.0f); // Assume 60fps step
            if (elapsed > 0.2f) {
                qAttackActive = false;
                animator->reset();
            }
        }
    } else if (controllerProfile.isAction(event, "w_attack")) {
        printf("Wizard W attack!\n");
        // TODO: Implement W attack logic
    } else if (controllerProfile.isAction(event, "e_attack")) {
        printf("Wizard E attack!\n");
        // TODO: Implement E attack logic
    } else if (controllerProfile.isAction(event, "r_ult")) {
        printf("Wizard R ULTIMATE!\n");
        // TODO: Implement R ultimate logic
    }
}

void Wizard::setPosition(float nx, float ny) {
    x = nx;
    y = ny;
}

void Wizard::render(SDL_Renderer* renderer, int camX, int camY, int windowW, int windowH) {
        // Apply animation pose before rendering
        if (qAttackActive && animator) animator->apply();
    // Calculate body mesh position
    int bodyW = static_cast<int>(width / 3);
    int bodyH = static_cast<int>(height * 0.68f);
    int bodyX = static_cast<int>(x - camX + width / 2 - bodyW / 2);
    int bodyY = static_cast<int>(y - camY + height * 0.32f);
    float time = SDL_GetTicks() * 0.001f;

    // Render cape using modular Cape class
    cape.render(renderer, bodyX, bodyY, bodyW, bodyH, height, time);

    // Render mesh using modular Mesh and Color system
    Color bodyColor = outfitColors.empty() ? Color(128, 0, 128, 255) : outfitColors[0];
    SDL_SetRenderDrawColor(renderer, bodyColor.r, bodyColor.g, bodyColor.b, bodyColor.a);
    if (baseMesh.vertices.size() >= 4) {
        // Transform mesh vertices to screen space
        SDL_Point pts[4];
        for (int i = 0; i < 4; ++i) {
            pts[i].x = static_cast<int>(baseMesh.vertices[i].x - camX);
            pts[i].y = static_cast<int>(baseMesh.vertices[i].y - camY);
        }
        // Fill mesh polygon (simple scanline fill)
        int minY = pts[0].y, maxY = pts[0].y;
        for (int i = 1; i < 4; ++i) {
            minY = std::min(minY, pts[i].y);
            maxY = std::max(maxY, pts[i].y);
        }
        for (int y = minY; y <= maxY; ++y) {
            int minX = windowW, maxX = 0;
            for (int i = 0; i < 4; ++i) {
                int y1 = pts[i].y, y2 = pts[(i + 1) % 4].y;
                int x1 = pts[i].x, x2 = pts[(i + 1) % 4].x;
                if ((y1 <= y && y2 > y) || (y2 <= y && y1 > y)) {
                    int x = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
                    minX = std::min(minX, x);
                    maxX = std::max(maxX, x);
                }
            }
            if (minX < maxX) SDL_RenderDrawLine(renderer, minX, y, maxX, y);
        }
    }

    // Visualize bones (draw lines and joints)
    struct BoneDraw {
        std::pair<float, float> start;
        std::pair<float, float> end;
    };
    std::vector<BoneDraw> boneLines = {
        {boneRoot.getWorldPos(), boneSpine.getWorldPos()},
        {boneSpine.getWorldPos(), boneHead.getWorldPos()},
        {boneSpine.getWorldPos(), boneLArm.getWorldPos()},
        {boneSpine.getWorldPos(), boneRArm.getWorldPos()},
        {boneRoot.getWorldPos(), boneLLeg.getWorldPos()},
        {boneRoot.getWorldPos(), boneRLeg.getWorldPos()}
    };
    SDL_SetRenderDrawColor(renderer, 40, 200, 255, 255); // cyan bones
    for (const auto& b : boneLines) {
        SDL_RenderDrawLine(renderer, static_cast<int>(b.start.first), static_cast<int>(b.start.second), static_cast<int>(b.end.first), static_cast<int>(b.end.second));
    }
    // Draw bone joints
    std::vector<std::pair<float, float>> boneJoints = {
        boneRoot.getWorldPos(), boneSpine.getWorldPos(), boneHead.getWorldPos(), boneLArm.getWorldPos(), boneRArm.getWorldPos(), boneLLeg.getWorldPos(), boneRLeg.getWorldPos()
    };
    SDL_SetRenderDrawColor(renderer, 255, 80, 80, 255); // red joints
    for (const auto& p : boneJoints) {
        for (int dx = -3; dx <= 3; ++dx) {
            for (int dy = -3; dy <= 3; ++dy) {
                SDL_RenderDrawPoint(renderer, static_cast<int>(p.first) + dx, static_cast<int>(p.second) + dy);
            }
        }
    }
    // ...existing rendering code...
}