#ifndef Process_hpp
#define Process_hpp

#include <stdio.h>
#include <openvdb/openvdb.h>
#include <random>
#include "cinder/Perlin.h"

#include "MeshNode.hpp"
#include "VolumeNode.hpp"

typedef openvdb::tree::Tree4<VolumeNode, 5, 4, 3>::Type VolumeNodeTreeType;
typedef openvdb::Grid<VolumeNodeTreeType> VolumeNodeGridType;

class Process {
public:
    struct Params {
        const ci::vec3 volumeBounds;
        const int densityPerUnit;
    };

    Process(const Params& params);

    void update();

    inline const ci::geom::BufferLayout& getLayout() { return layout; }
    inline const std::vector<MeshNode>& getNodes() { return nodes; }
    inline const ci::gl::VboMeshRef getMesh() { return volumeMesh; }
    inline const Params& getParams() { return params; }

private:
    VolumeNodeGridType grid;

    std::vector<MeshNode> nodes;
    ci::geom::BufferLayout layout;
    Params params;

    std::mt19937 generator;
    std::uniform_real_distribution<double> decayJitter;
    ci::Perlin perlin;

    ci::gl::VboMeshRef volumeMesh;

    void decay();
    void gridToMesh(const VolumeNodeGridType& grid, std::vector<MeshNode>& nodes, std::vector<openvdb::Vec3I>& triangles);
};

#endif /* Process_hpp */
