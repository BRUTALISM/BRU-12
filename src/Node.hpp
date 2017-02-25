#ifndef Node_hpp
#define Node_hpp

#include <stdio.h>
#include "cinder/Perlin.h"

const float LIFE_DECAY = 0.01f;

struct Node {
    ci::vec3 position;
    ci::Color color;

    // -1: dead, 1: alive; and everything in-between
    float life = 1.0f;

    void update(const ci::Perlin& perlin);
};

#endif /* Node_hpp */
