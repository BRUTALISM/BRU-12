#ifndef MeshNode_hpp
#define MeshNode_hpp

#include <stdio.h>

struct MeshNode {
    ci::vec3 position;
    ci::Color color;

    void update();
};

#endif /* MeshNode_hpp */
