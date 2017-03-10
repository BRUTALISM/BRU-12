#ifndef Node_hpp
#define Node_hpp

#include <stdio.h>

struct Node {
    ci::vec3 position;
    ci::Color color;

    void update();
};

#endif /* Node_hpp */
