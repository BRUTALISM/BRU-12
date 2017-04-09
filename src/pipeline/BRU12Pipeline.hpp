#pragma once

#include <openvdb/openvdb.h>

#include "MeshNode.hpp"
#include "VolumeNode.hpp"
#include "Queue.hpp"

typedef openvdb::tree::Tree4<VolumeNode, 5, 4, 3>::Type VolumeNodeTreeType;
typedef openvdb::Grid<VolumeNodeTreeType> VolumeNodeGridType;

class BRU12Pipeline {
public:
    struct Input {
        //
    };

    struct Output {
        const ci::gl::VboMeshRef mesh;
    };

    BRU12Pipeline();

    const beton::QueueRef<Output> getOutputQueue() const { return outQueue; }

private:
    VolumeNodeGridType grid;

    beton::QueueRef<Input> inQueue;
    beton::QueueRef<Output> outQueue;
};
