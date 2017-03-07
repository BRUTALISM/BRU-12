#include "Node.hpp"

using namespace ci;
using namespace ci::app;

void Node::update(const Perlin& perlin) {
    const float timeFactor = 0.2f;
    const float perlinScale = 0.02f;

    vec3 perlinCoord(position.x,
                     getElapsedSeconds() * timeFactor,
                     position.z);
    position.y += perlin.fBm(perlinCoord) * perlinScale;
}
