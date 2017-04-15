#pragma once

#include <openvdb/openvdb.h>

#include "MeshNode.hpp"
#include "VolumeNode.hpp"
#include "Queue.hpp"

#include "Stage.hpp"

typedef openvdb::tree::Tree4<VolumeNode, 5, 4, 3>::Type VolumeNodeTreeType;
typedef openvdb::Grid<VolumeNodeTreeType> VolumeNodeGridType;

class BRU12Pipeline {
public:
    struct Params {
        const ci::vec3 volumeBounds;
        const int densityPerUnit;
        const float gridFillValue;
        const float gridBackgroundValue;
        const float isoValue;
        const float decayMultiplier;
    };

    struct Input {
        VolumeNodeGridType& grid;
        Params params;
    };

    struct Output {
        const ci::gl::VboMeshRef mesh;
    };

    BRU12Pipeline(const Params& params);

    const beton::QueueRef<Output> getOutputQueue() const { return outQueue; }

    void nextIteration();

private:
    Params params;

    VolumeNodeGridType grid;

    beton::QueueRef<Input> inQueue;
    beton::QueueRef<Output> outQueue;

    beton::Stage<Input, Output> stage;
};
