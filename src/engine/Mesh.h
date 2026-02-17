#pragma once
#include <vector>

struct MeshVertex {
    float x, y;
    std::vector<float> weights; // Bone weights
};

class Mesh {
public:
    std::vector<MeshVertex> vertices;
    Mesh() {}
    void clear() { vertices.clear(); }
    void addVertex(float x, float y) { vertices.push_back({x, y, {}}); }
};
