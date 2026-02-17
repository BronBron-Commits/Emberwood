#include "Animation.h"

Animation::Animation(const std::vector<AnimationFrame>& frames)
    : frames(frames), currentFrame(0), timeInFrame(0.0f) {}

void Animation::update(float deltaTime) {
    if (frames.empty()) return;
    timeInFrame += deltaTime;
    while (timeInFrame > frames[currentFrame].duration) {
        timeInFrame -= frames[currentFrame].duration;
        currentFrame = (currentFrame + 1) % frames.size();
    }
}

void Animation::applyCurrentFrame(float alpha) {
    if (frames.empty()) return;
    frames[currentFrame].apply(alpha);
}

void Animation::reset() {
    currentFrame = 0;
    timeInFrame = 0.0f;
}
