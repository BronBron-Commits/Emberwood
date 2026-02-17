#pragma once
#include <cmath>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include "Cape.h"
#include "../engine/Mesh.h"
#include "../engine/Bone.h"
#include "../engine/Color.h"
#include "../engine/ControllerProfile.h"


class Wizard {
            // Base mesh for wizard (simple rectangle for now)
        #include "../engine/Mesh.h"
        Mesh baseMesh;

            // Auto-fit bones to mesh bounds
            void autoFitBonesToMesh() {
                if (baseMesh.vertices.empty()) return;
                // Find mesh bounds
                float minX = baseMesh.vertices[0].x, maxX = baseMesh.vertices[0].x;
                float minY = baseMesh.vertices[0].y, maxY = baseMesh.vertices[0].y;
                for (const auto& v : baseMesh.vertices) {
                    if (v.x < minX) minX = v.x;
                    if (v.x > maxX) maxX = v.x;
                    if (v.y < minY) minY = v.y;
                    if (v.y > maxY) maxY = v.y;
                }
                // Place bones relative to mesh bounds
                float bodyW = maxX - minX;
                float bodyH = maxY - minY;
                float bodyX = minX;
                float bodyY = minY;
                setupBones(bodyX, bodyY, bodyW, bodyH);
            }
        // Skeletal structure for rigging
        #include "../engine/Bone.h"
        Bone boneRoot;
        Bone boneSpine;
        Bone boneHead;
        std::vector<Color> outfitColors;
        Bone boneLArm;
        Bone boneRArm;
        Bone boneLLeg;
        Bone boneRLeg;
    Cape cape;
public:
            // Initialize base mesh (rectangle for now)
        void initBaseMesh(float bodyX, float bodyY, float bodyW, float bodyH) {
            baseMesh.clear();
            baseMesh.addVertex(bodyX, bodyY);
            baseMesh.addVertex(bodyX + bodyW, bodyY);
            baseMesh.addVertex(bodyX + bodyW, bodyY + bodyH);
            baseMesh.addVertex(bodyX, bodyY + bodyH);
            // TODO: assign bone weights for each vertex
            autoFitBonesToMesh();
            }
        void setupBones(float bodyX, float bodyY, float bodyW, float bodyH) {
            // Root at feet (bottom center of body)
            boneRoot = Bone(bodyX + bodyW / 2.0f, bodyY + bodyH, 0.0f);
            // Spine up from root (center of body)
            boneSpine = Bone(0.0f, -bodyH, 0.0f, &boneRoot);
            // Head at top of spine (centered, height matches rendering)
            boneHead = Bone(0.0f, -bodyH * 0.32f, 0.0f, &boneSpine);
            // Arms from spine (shoulder height, width matches rendering)
            boneLArm = Bone(-bodyW * 0.45f, -bodyH * 0.18f, 0.0f, &boneSpine);
            boneRArm = Bone(bodyW * 0.45f, -bodyH * 0.18f, 0.0f, &boneSpine);
            // Legs from root (offset from center, matches rendering)
            boneLLeg = Bone(-bodyW * 0.18f, 0.0f, 0.0f, &boneRoot);
            boneRLeg = Bone(bodyW * 0.18f, 0.0f, 0.0f, &boneRoot);
        }
    // Animation state
    bool isDancing = false;
    bool isSpinning = false;
    Uint32 danceStartTime = 0;
    Wizard(SDL_Renderer* renderer, TTF_Font* font, int windowW, int windowH);
    ~Wizard();

    void handleEvent(const SDL_Event& event);
    void handleInput(const Uint8* keystate);
    void update();
    void render(SDL_Renderer* renderer, int camX, int camY, int windowW, int windowH);

    // Accessors for HUD, camera, etc.
    int getHealth() const;
    int getMaxHealth() const;
    int getEnergy() const;
    int getMaxEnergy() const;
    float getX() const;
    float getY() const;
    int getWidth() const;
    int getHeight() const;
    int getFacing() const;
    void setPosition(float x, float y);
    void setVelocity(float vx, float vy);
    void setFacing(int f);

private:
    // State (mirrors main.cpp for now)
    float x, y, vx, vy;
    int health, maxHealth, energy, maxEnergy;
    float energyF;
    int facing;
    int walkFrame, walkCounter;
    int currentOutfit;
    int width, height;
    // Animation
    int headBobVals[2];
    int hatTiltVals[2];
    SDL_Texture* spriteSideTexture[2];
    SDL_Texture* spriteBackTexture[2];
    SDL_Texture* spriteFrontTexture[2];
    void regenerateSprites(SDL_Renderer* renderer);

    // Modular animation system
    std::unique_ptr<Animator> animator; // Use unique_ptr for animator
    std::shared_ptr<Animation> qAttackAnim;
    bool qAttackActive = false;
    Uint32 qAttackStartTime = 0;
    // Projectiles, particles, etc.
    struct Fireball {
        float x, y, vx, vy;
        int life;
        float size = 1.0f;
        bool exploding = false;
        int explosionFrame = 0;
    };
    std::vector<Fireball> fireballs;
    struct DustParticle {
        float x, y, vx, vy;
        int life;
    };
    std::vector<DustParticle> dustParticles;
    int dustEmitCooldown;
    // Outfit colors
    // outfitColors is now modularized as std::vector<Color>
    ControllerProfile controllerProfile;
};
