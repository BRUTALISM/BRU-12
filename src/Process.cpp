#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/Interpolation.h>

#include "Process.hpp"

using namespace std;
using namespace ci;
using namespace openvdb::tools;

const float GRID_BACKGROUND_VALUE = 0.0f;
const float GRID_FILL_VALUE = 1.0f;
const float ISO_VALUE = GRID_FILL_VALUE * 0.95;
const float DECAY_MULTIPLIER = 0.01f;

Process::Process(const Params& params) : grid(GRID_BACKGROUND_VALUE), params(params), generator(random_device()()) {
    openvdb::initialize();

    decayJitter = uniform_real_distribution<double>(1.0, 1.3);

    vec3 bounds = params.volumeBounds;
    int subdivision = params.densityPerUnit;
    openvdb::CoordBBox bbox(0.0f, 0.0f, 0.0f,
                            bounds.x * subdivision, bounds.y * subdivision, bounds.z * subdivision);
    grid.denseFill(bbox, GRID_FILL_VALUE, true);

    openvdb::math::Mat4d mat = openvdb::math::Mat4d::identity();
    auto linearTransform = openvdb::math::Transform::createLinearTransform(mat);
    linearTransform->preScale(openvdb::Vec3d(1.0 / subdivision,
                                             1.0 / subdivision,
                                             1.0 / subdivision));
    grid.setTransform(linearTransform);

    update();
}

// Heavily tweaked doVolumeToMesh from VolumeToMesh.h
void Process::gridToMesh(const VolumeNodeGridType& grid,
                         vector<MeshNode>& nodes,
                         vector<openvdb::Vec3I>& triangles) {
    const double isoValue = ISO_VALUE;
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

            auto volumeNode = sampler.wsSample(position);

            MeshNode meshNode {
                .position = vec3(position.x(), position.y(), position.z()),
                .color = volumeNode.color
            };
            nodes[i] = meshNode;
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

void Process::decay() {
    const double boundY = params.volumeBounds.y * params.densityPerUnit;
    for (auto iterator = grid.beginValueOn(); iterator.test(); ++iterator) {
        auto value = iterator.getValue();
        auto coord = iterator.getCoord();

        value.life -= ((boundY - coord.y()) / boundY) * DECAY_MULTIPLIER * decayJitter(generator);
//        auto coordVec = vec3(coord.x(), coord.y(), coord.z()) * 0.1f;
//        value.life -= perlin.fBm(coordVec) * DECAY_MULTIPLIER * decayJitter(generator);

        auto t = coord.y() / (params.volumeBounds.y * params.densityPerUnit);
        value.color = Color(value.color.r, 0.0f, t);

        if (value.life < GRID_BACKGROUND_VALUE) {
            iterator.setActiveState(false);
        } else {
            iterator.setValue(value);
        }
    }
}

void Process::update() {
    decay();

    vector<MeshNode> nodes;
    vector<openvdb::Vec3I> triangles;
    gridToMesh(grid, nodes, triangles);

    geom::BufferLayout layout;
    layout.append(geom::Attrib::POSITION, 3, sizeof(MeshNode), offsetof(MeshNode, position));
    layout.append(geom::Attrib::COLOR, 3, sizeof(MeshNode), offsetof(MeshNode, color));

    auto vbo = gl::Vbo::create(GL_ARRAY_BUFFER, nodes, GL_STATIC_DRAW);
    volumeMesh = gl::VboMesh::create((uint32_t) nodes.size(), GL_TRIANGLES, {{ layout, vbo }},
                                     (uint32_t) triangles.size() * 3, GL_UNSIGNED_INT);
    volumeMesh->bufferIndices(triangles.size() * 3 * sizeof(uint32_t), triangles.data());
}
