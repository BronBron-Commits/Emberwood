#include "World.h"

WorldObject::WorldObject()
    : position(0.0f, 0.0f, 0.0f), rotation(0.0f, 0.0f, 0.0f), scale(1.0f, 1.0f, 1.0f) {}

void World::addObject(const std::shared_ptr<WorldObject>& obj) {
    objects.push_back(obj);
}

void World::removeObject(const std::shared_ptr<WorldObject>& obj) {
    objects.erase(std::remove(objects.begin(), objects.end(), obj), objects.end());
}
