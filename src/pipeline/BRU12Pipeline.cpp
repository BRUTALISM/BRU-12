#include "BRU12Pipeline.hpp"
#include "DecayProcessor.hpp"

using namespace std;
using namespace ci;

BRU12Pipeline::BRU12Pipeline(const Params& params) :
    params(params), grid(params.gridBackgroundValue), inQueue(make_shared<beton::Queue<Input>>()),
    outQueue(make_shared<beton::Queue<Output>>()), stage(inQueue, outQueue) {
        // Initialize the grid
        openvdb::initialize();

        vec3 bounds = params.volumeBounds;
        int subdivision = params.densityPerUnit;
        openvdb::CoordBBox bbox(0.0f, 0.0f, 0.0f,
                                bounds.x * subdivision,
                                bounds.y * subdivision,
                                bounds.z * subdivision);
        grid.denseFill(bbox, params.gridFillValue, true);

        openvdb::math::Mat4d mat = openvdb::math::Mat4d::identity();
        auto linearTransform = openvdb::math::Transform::createLinearTransform(mat);
        auto res = 1.0 / subdivision;
        linearTransform->preScale(openvdb::Vec3d(res, res, res));
        grid.setTransform(linearTransform);
        
        // Start the pipeline
        stage.pushProcessor(make_shared<DecayProcessor>());
}

void BRU12Pipeline::nextIteration() {
    auto input = Input {
        .grid = grid,
        .params = params
    };

    inQueue->push(input);
}
