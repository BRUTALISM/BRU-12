#ifndef Process_hpp
#define Process_hpp

#include <stdio.h>
#include <openvdb/openvdb.h>

#include "Node.hpp"

typedef openvdb::tree::Tree4<float, 5, 4, 3>::Type FloatTreeType;
typedef openvdb::Grid<FloatTreeType> FloatGridType;

class Process {
public:
    struct Params {
        const ci::vec3 volumeBounds;
        const int densityPerUnit;
    };

    Process(const Params& params);

    void update();

    inline const ci::geom::BufferLayout& getLayout() { return layout; }
    inline const std::vector<Node>& getNodes() { return nodes; }
    inline const ci::gl::VboMeshRef getMesh() { return volumeMesh; }
    inline const Params& getParams() { return params; }

private:
    std::vector<Node> nodes;
    ci::geom::BufferLayout layout;
    Params params;

    ci::gl::VboMeshRef volumeMesh;

    void gridToMesh(const FloatGridType& grid, std::vector<Node>& nodes, std::vector<openvdb::Vec3I>& triangles);
};

#endif /* Process_hpp */
