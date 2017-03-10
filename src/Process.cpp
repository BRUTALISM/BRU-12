#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/LevelSetSphere.h>

#include "Process.hpp"

using namespace std;
using namespace ci;

const float GRID_BACKGROUND_VALUE = 0.0f;
const float GRID_FILL_VALUE = 1.0f;
const float ISO_VALUE = GRID_FILL_VALUE * 0.5;
const float DECAY_MULTIPLIER = 0.02f;

Process::Process(const Params& params) : grid(GRID_BACKGROUND_VALUE), params(params), generator(random_device()()) {
    openvdb::initialize();

    decayJitter = uniform_real_distribution<double>(1.0, 1.3);

    vec3 bounds = params.volumeBounds;
    int subdivision = params.densityPerUnit;
    openvdb::CoordBBox bbox(0.0f, 0.0f, 0.0f, bounds.x * subdivision, bounds.y * subdivision, bounds.z * subdivision);
    grid.tree().fill(bbox, GRID_FILL_VALUE);

    openvdb::math::Mat4d mat = openvdb::math::Mat4d::identity();
    auto linearTransform = openvdb::math::Transform::createLinearTransform(mat);
    linearTransform->preScale(openvdb::Vec3d(1.0 / subdivision, 1.0 / subdivision, 1.0 / subdivision));
    grid.setTransform(linearTransform);

    update();
}

/*
// Ovo je jedan način da furaš indekse - preko zasebnog index VBO niza, pa da onda nad njim
// radiš mapiranje i memcpy
//    indexVbo = gl::Vbo::create(GL_ELEMENT_ARRAY_BUFFER, indices);
//    mesh = gl::VboMesh::create((uint32_t) nodes.size(), GL_TRIANGLES, {{ layout, vbo }},
//                               (uint32_t) indices.size(), GL_UNSIGNED_INT, indexVbo);

// Ovo je drugi, verovatno jednostavniji način da se indeksira
//    mesh = gl::VboMesh::create((uint32_t) nodes.size(), GL_LINES, {{ layout, vbo }},
//                               (uint32_t) indices.size(), GL_UNSIGNED_INT);
//    mesh->bufferIndices(indices.size() * sizeof(uint32_t), indices.data());

// Ovo kada hoćeš da iscrtavaš samo tačke, ne treba ti indeks niz
//mesh = gl::VboMesh::create((uint32_t) nodes.size(), GL_POINTS, {{ layout, vbo }});
*/

// Heavily tweaked doVolumeToMesh from VolumeToMesh.h
void Process::gridToMesh(const FloatGridType& grid, vector<Node>& nodes, vector<openvdb::Vec3I>& triangles) {
    const double isoValue = ISO_VALUE;
    const double adaptivity = 0.0;
    openvdb::tools::VolumeToMesh mesher(isoValue, adaptivity, true);
    mesher(grid);

    // Preallocate the point list
    nodes.clear();
    nodes.resize(mesher.pointListSize());

    // Make nodes out of points
    {
        boost::scoped_array<openvdb::Vec3s>& pointList = mesher.pointList();
        for (size_t i = 0; i < mesher.pointListSize(); i++) {
            openvdb::Vec3s& position = pointList[i];
            Node node {
                .position = vec3(position.x(), position.y(), position.z()),
                .color = Color(0.0, 0.0, 1.0 - (position.y() / (params.volumeBounds.y)))
            };
            nodes[i] = node;
        }

        mesher.pointList().reset(nullptr);
    }

    openvdb::tools::PolygonPoolList& polygonPoolList = mesher.polygonPoolList();

    // Preallocate primitive lists
    {
        size_t numQuads = 0, numTriangles = 0;
        for (size_t n = 0, N = mesher.polygonPoolListSize(); n < N; ++n) {
            openvdb::tools::PolygonPool& polygons = polygonPoolList[n];
            numTriangles += polygons.numTriangles();
            numQuads += polygons.numQuads();
        }

        triangles.clear();
        triangles.resize(numTriangles + numQuads * 2);
    }

    // Copy primitives
    size_t tIdx = 0;
    for (size_t n = 0, N = mesher.polygonPoolListSize(); n < N; ++n) {
        openvdb::tools::PolygonPool& polygons = polygonPoolList[n];

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
        value -= ((boundY - coord.y()) / boundY) * DECAY_MULTIPLIER * decayJitter(generator);
//        value -= sqrt(pow(coord.x(), 1.2) + pow(coord.z(), 1.2)) * DECAY_MULTIPLIER * decayJitter(generator);

        if (value < GRID_BACKGROUND_VALUE) {
            iterator.setActiveState(false);
        } else {
            iterator.setValue(value);
        }
    }
}

void Process::update() {
    decay();

    vector<Node> nodes;
    vector<openvdb::Vec3I> triangles;
    gridToMesh(grid, nodes, triangles);

    geom::BufferLayout layout;
    layout.append(geom::Attrib::POSITION, 3, sizeof(Node), offsetof(Node, position));
    layout.append(geom::Attrib::COLOR, 3, sizeof(Node), offsetof(Node, color));

    auto vbo = gl::Vbo::create(GL_ARRAY_BUFFER, nodes, GL_STATIC_DRAW);
    volumeMesh = gl::VboMesh::create((uint32_t) nodes.size(), GL_TRIANGLES, {{ layout, vbo }},
                                     (uint32_t) triangles.size() * 3, GL_UNSIGNED_INT);
    volumeMesh->bufferIndices(triangles.size() * 3 * sizeof(uint32_t), triangles.data());
}
