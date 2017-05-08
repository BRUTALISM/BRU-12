#pragma once

#include "Params.hpp"
#include "MeshNode.hpp"
#include "Queue.hpp"
#include "PreparedGrid.hpp"
#include "Stage.hpp"
#include "VolumeNodeGridType.hpp"

class BRU12Pipeline {
public:
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
	beton::QueueRef<PreparedGrid> preparedGridQueue;
    beton::QueueRef<Output> outQueue;

    beton::Stage<Input, PreparedGrid> decayStage;
	beton::Stage<PreparedGrid, Output> mesherStage;
};
