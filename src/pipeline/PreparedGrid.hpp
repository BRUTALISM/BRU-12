#pragma once

#include "Params.hpp"
#include "VolumeNodeGridType.hpp"

struct PreparedGrid {
	const VolumeNodeGridType& grid;
	const Params& params;
};
