#include "BRU12Pipeline.hpp"
#include "Muncher.hpp"
#include "Decay.hpp"
#include "Mesher.hpp"

using namespace std;
using namespace ci;

BRU12Pipeline::BRU12Pipeline(const Params& params) :
    params(params), grid(params.gridBackgroundValue), inQueue(make_shared<beton::Queue<Input>>()),
	muncherOutQueue(make_shared<beton::Queue<PreparedGrid>>()),
    decayOutQueue(make_shared<beton::Queue<PreparedGrid>>()),
    outQueue(make_shared<beton::Queue<Output>>()),
    muncherStage(inQueue, muncherOutQueue),
    decayStage(muncherOutQueue, decayOutQueue),
	mesherStage(decayOutQueue, outQueue) {
        // Initialize the grid
        openvdb::initialize();

        openvdb::math::Mat4d mat = openvdb::math::Mat4d::identity();
        auto linearTransform = openvdb::math::Transform::createLinearTransform(mat);
        auto res = 1.0 / params.densityPerUnit;
        linearTransform->preScale(openvdb::Vec3d(res, res, res));
        grid.setTransform(linearTransform);

        // Create a separate GL context for the processor
        auto context = ci::gl::Context::create(ci::gl::Context::getCurrent());

        // Start the pipeline
        muncherStage.pushProcessor(make_shared<Muncher>());
        decayStage.pushProcessor(make_shared<Decay>());
        mesherStage.pushProcessor(make_shared<Mesher>(context));
}

void BRU12Pipeline::nextIteration() {
    auto input = Input {
        .grid = grid,
        .params = params
    };

    inQueue->push(input);
}
