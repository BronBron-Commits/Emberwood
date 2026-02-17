#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>

// Represents a 3D object in world space
class WorldObject {
public:
    glm::vec3 position; // World-space position (origin is always world 0,0,0)
    glm::vec3 rotation; // Euler angles (optional)
    glm::vec3 scale;    // Scale (optional)

    WorldObject();
    virtual ~WorldObject() = default;
};

// The world itself, containing all objects
class World {
public:
    std::vector<std::shared_ptr<WorldObject>> objects;

    void addObject(const std::shared_ptr<WorldObject>& obj);
    void removeObject(const std::shared_ptr<WorldObject>& obj);
};
