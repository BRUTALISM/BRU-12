#ifndef Process_hpp
#define Process_hpp

#include <stdio.h>
#include "Node.hpp"
#include "cinder/Perlin.h"

class Process {
private:
    std::vector<Node> nodes;
    ci::geom::BufferLayout layout;
    ci::Perlin perlin;

public:
    Process(int nodeCount);

    void update();
    const ci::geom::BufferLayout& getLayout() { return layout; }
    const std::vector<Node>& getNodes() { return nodes; }
};

#endif /* Process_hpp */
