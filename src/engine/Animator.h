#pragma once
#include "Animation.h"
#include <memory>

class Animator {
public:
    Animator();
    void setAnimation(std::shared_ptr<Animation> anim);
    void update(float deltaTime);
    void apply(float alpha = 1.0f);
    void reset();
private:
    std::shared_ptr<Animation> currentAnim;
};
