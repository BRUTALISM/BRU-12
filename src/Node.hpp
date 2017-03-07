#ifndef Node_hpp
#define Node_hpp

#include <stdio.h>
#include "cinder/Perlin.h"

struct Node {
    ci::vec3 position;
    ci::Color color;

    void update(const ci::Perlin& perlin);
};

#endif /* Node_hpp */
