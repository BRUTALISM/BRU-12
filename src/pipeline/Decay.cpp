#include <openvdb/openvdb.h>
#include <openvdb/tools/ValueTransformer.h>
#include <openvdb/tree/ValueAccessor.h>

#include "Decay.hpp"

#include "VolumeNodeGridType.hpp"
#include "MeshNode.hpp"

using namespace std;
using namespace ci;
using namespace ci::gl;
using namespace openvdb::tools;
using namespace openvdb::tree;

class DecayOperation {
    BRU12Pipeline::Input& input;
    ValueAccessorRW<VolumeNodeTreeType> accessor;
	std::mt19937& generator;
	std::uniform_real_distribution<double>& decayJitter;
public:
	DecayOperation(BRU12Pipeline::Input& input,
				   std::mt19937& generator,
				   std::uniform_real_distribution<double>& decayJitter) :
	input(input),
    accessor(ValueAccessorRW<VolumeNodeTreeType>(input.grid.tree())),
    generator(generator),
    decayJitter(decayJitter)
    {}

	inline void operator()(const VolumeNodeGridType::ValueOnIter& iterator) const {
		const double boundY = input.params.volumeBounds.y * input.params.densityPerUnit;
		auto coord = iterator.getCoord();
        // auto value = iterator.getValue();
        auto value = accessor.getValue(coord);

		value.life -= ((boundY - coord.y()) / boundY) * input.params.decayMultiplier *
		decayJitter(generator);

		//        auto coordVec = vec3(coord.x(), coord.y(), coord.z()) * 0.1f;
		//        value.life -= perlin.fBm(coordVec) * input.params.decayMultiplier *
		//            decayJitter(generator);

		auto t = coord.y() / (input.params.volumeBounds.y * input.params.densityPerUnit);
		value.color = Color(value.color.r, 0.0f, t);

		if (value.life < input.params.gridBackgroundValue) {
			iterator.setActiveState(false);
		} else {
			iterator.setValue(value);
		}
	}
};

PreparedGrid Decay::process(BRU12Pipeline::Input& input) {
	foreach(input.grid.beginValueOn(), DecayOperation(input, generator, decayJitter));

	return PreparedGrid {
		.grid = input.grid,
		.params = input.params
	};
}
