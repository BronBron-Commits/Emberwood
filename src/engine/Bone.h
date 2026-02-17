#pragma once
#include <utility>

class Bone {
public:
    float x, y;
    float angle;
    Bone* parent;
    Bone(float x=0, float y=0, float angle=0, Bone* parent=nullptr)
        : x(x), y(y), angle(angle), parent(parent) {}
    // Get world position
    std::pair<float, float> getWorldPos() const {
        if (parent) {
            float px, py;
            std::tie(px, py) = parent->getWorldPos();
            float rad = parent->angle;
            float wx = px + x * ::cos(rad) - y * ::sin(rad);
            float wy = py + x * ::sin(rad) + y * ::cos(rad);
            return {wx, wy};
        } else {
            return {x, y};
        }
    }
};
