#include "Decay.hpp"

#include "VolumeNodeGridType.hpp"
#include "MeshNode.hpp"

#include <openvdb/openvdb.h>
#include <openvdb/tools/ValueTransformer.h>
#include <openvdb/tree/ValueAccessor.h>

using namespace std;
using namespace ci;
using namespace ci::gl;
using namespace openvdb::tools;
using namespace openvdb::tree;

class DecayOperation {
    PreparedGrid& input;
    VolumeNodeGridType::ConstAccessor accessor;
    std::mt19937& generator;
    std::uniform_real_distribution<double>& decayJitter;

public:
    DecayOperation(PreparedGrid& input,
                   std::mt19937& generator,
                   std::uniform_real_distribution<double>& decayJitter) :
    input(input),
    accessor(input.grid.getConstAccessor()),
    generator(generator),
    decayJitter(decayJitter) { }

    inline void operator()(const VolumeNodeGridType::ValueOnIter& iterator) const {
        auto coord = iterator.getCoord();
        // auto value = iterator.getValue();
        auto value = accessor.getValue(coord);

        const double boundY = input.params.volumeBounds.y * input.params.densityPerUnit;
        auto decay = ((boundY - coord.y()) / boundY) * input.params.decayMultiplier *
        decayJitter(generator);
        value.life -= decay;

        auto xScale = input.params.volumeBounds.x * input.params.densityPerUnit;
        auto yScale = input.params.volumeBounds.y * input.params.densityPerUnit;
        auto t = coord.y() / yScale;
        value.color = Color(value.color.r, coord.x() / xScale, t);

        if (value.life < input.params.gridBackgroundValue) {
            iterator.setActiveState(false);
        } else {
            iterator.setValue(value);
        }
    }
};

PreparedGrid Decay::process(PreparedGrid& input) {
    foreach(input.grid.beginValueOn(), DecayOperation(input, generator, decayJitter));
    
    return PreparedGrid {
        .grid = input.grid,
        .params = input.params
    };
}
