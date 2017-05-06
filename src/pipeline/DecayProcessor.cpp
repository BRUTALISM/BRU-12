#include "DecayProcessor.hpp"

#include <openvdb/openvdb.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/Interpolation.h>

#include "MeshNode.hpp"

using namespace std;
using namespace ci;
using namespace ci::gl;
using namespace openvdb::tools;

DecayProcessor::DecayProcessor(ci::gl::ContextRef context) : glContext(context) {}

void DecayProcessor::threadSetup() {
	glContext->makeCurrent();
}

// Heavily tweaked doVolumeToMesh from VolumeToMesh.h
void gridToMesh(const VolumeNodeGridType& grid,
                vector<MeshNode>& nodes,
                vector<openvdb::Vec3I>& triangles,
                const float isoValue) {
    const double adaptivity = 0.0;
    VolumeToMesh mesher(isoValue, adaptivity, true);
    mesher(grid);

    // Preallocate the point list
    nodes.clear();
    nodes.resize(mesher.pointListSize());

    // Make nodes out of points
    {
        auto accessor = grid.getConstAccessor();
        GridSampler<VolumeNodeGridType::ConstAccessor, BoxSampler> sampler(accessor,
                                                                           grid.transform());
        boost::scoped_array<openvdb::Vec3s>& pointList = mesher.pointList();

        for (size_t i = 0; i < mesher.pointListSize(); i++) {
            openvdb::Vec3s& position = pointList[i];
            nodes[i] = {
                .position = vec3(position.x(), position.y(), position.z()),
                .color = sampler.wsSample(position).color
            };
        }

        mesher.pointList().reset(nullptr);
    }

    PolygonPoolList& polygonPoolList = mesher.polygonPoolList();

    // Preallocate primitive lists
    {
        size_t numQuads = 0, numTriangles = 0;
        for (size_t n = 0, N = mesher.polygonPoolListSize(); n < N; ++n) {
            PolygonPool& polygons = polygonPoolList[n];
            numTriangles += polygons.numTriangles();
            numQuads += polygons.numQuads();
        }

        triangles.clear();
        triangles.resize(numTriangles + numQuads * 2);
    }

    // Copy primitives
    size_t tIdx = 0;
    for (size_t n = 0, N = mesher.polygonPoolListSize(); n < N; ++n) {
        PolygonPool& polygons = polygonPoolList[n];

        // Convert quads to triangles, since OpenGL doesn't support GL_QUADS anymore
        for (size_t i = 0, I = polygons.numQuads(); i < I; ++i) {
            openvdb::Vec4I& quad = polygons.quad(i);
            triangles[tIdx++] = openvdb::Vec3I(quad.x(), quad.y(), quad.z());
            triangles[tIdx++] = openvdb::Vec3I(quad.x(), quad.z(), quad.w());
        }

        for (size_t i = 0, I = polygons.numTriangles(); i < I; ++i) {
            triangles[tIdx++] = polygons.triangle(i);
        }
    }
}

class DecayOperation {
	BRU12Pipeline::Input& input;
	std::mt19937& generator;
	std::uniform_real_distribution<double>& decayJitter;
public:
	DecayOperation(BRU12Pipeline::Input& input,
				   std::mt19937& generator,
				   std::uniform_real_distribution<double>& decayJitter) :
	input(input), generator(generator), decayJitter(decayJitter) {}

	inline void operator()(const VolumeNodeGridType::ValueOnIter& iterator) const {
		const double boundY = input.params.volumeBounds.y * input.params.densityPerUnit;
		auto value = iterator.getValue();
		auto coord = iterator.getCoord();

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

BRU12Pipeline::Output DecayProcessor::process(BRU12Pipeline::Input& input) {
	foreach(input.grid.beginValueOn(), DecayOperation(input, generator, decayJitter));

    vector<MeshNode> nodes;
    vector<openvdb::Vec3I> triangles;

    gridToMesh(input.grid, nodes, triangles, input.params.isoValue);

	geom::BufferLayout layout;
	layout.append(geom::Attrib::POSITION, 3, sizeof(MeshNode),
				  offsetof(MeshNode, position));
	layout.append(geom::Attrib::COLOR, 3, sizeof(MeshNode), offsetof(MeshNode, color));

	auto vbo = gl::Vbo::create(GL_ARRAY_BUFFER, nodes, GL_STATIC_DRAW);
	auto volumeMesh = gl::VboMesh::create(
										  (uint32_t) nodes.size(), GL_TRIANGLES, {{ layout, vbo }},
										  (uint32_t) triangles.size() * 3, GL_UNSIGNED_INT);

	volumeMesh->bufferIndices(triangles.size() * 3 * sizeof(uint32_t), triangles.data());

    return BRU12Pipeline::Output {
        .mesh = move(volumeMesh)
    };
}
