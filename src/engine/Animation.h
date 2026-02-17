#pragma once
#include <vector>
#include <functional>

struct AnimationFrame {
    float duration; // milliseconds
    std::function<void(float)> apply; // function to apply pose for this frame
};

class Animation {
public:
    Animation(const std::vector<AnimationFrame>& frames);
    void update(float deltaTime);
    void applyCurrentFrame(float alpha = 1.0f);
    void reset();
private:
    std::vector<AnimationFrame> frames;
    size_t currentFrame;
    float timeInFrame;
};
