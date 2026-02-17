#include "Animator.h"

Animator::Animator() : currentAnim(nullptr) {}

void Animator::setAnimation(std::shared_ptr<Animation> anim) {
    currentAnim = anim;
    if (currentAnim) currentAnim->reset();
}

void Animator::update(float deltaTime) {
    if (currentAnim) currentAnim->update(deltaTime);
}

void Animator::apply(float alpha) {
    if (currentAnim) currentAnim->applyCurrentFrame(alpha);
}

void Animator::reset() {
    if (currentAnim) currentAnim->reset();
}
