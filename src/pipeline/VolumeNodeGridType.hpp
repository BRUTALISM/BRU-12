#pragma once

#include <openvdb/openvdb.h>
#include "VolumeNode.hpp"

typedef openvdb::tree::Tree4<VolumeNode, 5, 4, 3>::Type VolumeNodeTreeType;
typedef openvdb::Grid<VolumeNodeTreeType> VolumeNodeGridType;
