#pragma once

class Entity {
public:
    virtual void update() = 0;
    virtual ~Entity() = default;
};
