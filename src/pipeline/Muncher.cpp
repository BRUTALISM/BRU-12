#include "Muncher.hpp"

#include <random>

using namespace ci;

const int STEPS = 14;
const int BOXES_PER_STEP = 32;
const float SCALE_DECAY = 0.9f;

Muncher::Muncher() : fill(true), stepsLeft(STEPS), currentScale(1.0f) {}

PreparedGrid Muncher::process(BRU12Pipeline::Input& input) {
    if (stepsLeft > 0) {
        std::random_device device;
        std::mt19937 generator(device());

        std::uniform_real_distribution<float> randomPosition(0.0f, 1.0f);
        std::uniform_real_distribution<float> randomDimensions(0.5f, 1.0f);
        std::uniform_real_distribution<float> randomDimensionsX(0.01f, 0.1f);

        for (int i = 0; i < BOXES_PER_STEP; i++) {
            int subdivision = input.params.densityPerUnit;
            vec3 bounds = input.params.volumeBounds * (float)subdivision;

            auto xPos = bounds.x * randomPosition(generator);
            auto zPos = bounds.z * randomPosition(generator);
            auto xDim = bounds.x * randomDimensionsX(generator);
            auto yDim = bounds.y * randomDimensions(generator);
            auto zDim = bounds.z * randomDimensions(generator);

            openvdb::CoordBBox bbox(xPos, -yDim / 2, zPos,
                                    xPos + xDim, yDim / 2, zPos + zDim);

            if (fill) {
                input.grid.denseFill(bbox, input.params.gridFillValue, true);
            } else {
                input.grid.denseFill(bbox, 0.0f, false);
            }
        }

        fill = !fill;
        stepsLeft--;
        currentScale *= SCALE_DECAY;
    }

    return PreparedGrid {
        .grid = input.grid,
        .params = input.params
    };
}
