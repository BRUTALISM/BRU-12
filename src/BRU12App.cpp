#include <openvdb/openvdb.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/LevelSetSphere.h>

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/app/Platform.h"
#include "cinder/Log.h"
#include "cinder/Utilities.h"
#include "cinder/gl/gl.h"

#include <random>

#include "Node.hpp"
#include "Process.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

#define WIDTH 1200
#define HEIGHT 768
#define NODE_COUNT 80
#define INDEX_COUNT NODE_COUNT * 3

typedef openvdb::tree::Tree4<float, 5, 4, 3>::Type FloatTreeType;
typedef openvdb::Grid<FloatTreeType> FloatGridType;

class BRU12App : public App {
public:
    void setup() override;
    void mouseDown(MouseEvent event) override;
    void keyDown(KeyEvent event) override;
    void update() override;
    void draw() override;

    BRU12App();


private:
    void recreateVBOs();
    void initializeOpenVDB();
    void gridToMesh(const FloatGridType& grid, vector<Node>& nodes, vector<openvdb::Vec3I>& triangles);

    Process process;

    CameraPersp camera;
    gl::GlslProgRef	glslProg;

    vector<uint32_t> indices;

    gl::VboRef vbo;
    gl::VboRef indexVbo;
    gl::VboMeshRef mesh;
    //    gl::BatchRef meshBatch;
    gl::VboMeshRef volumeMesh;

    bool wireframe = false;
};

BRU12App::BRU12App() : process(NODE_COUNT), camera(WIDTH, HEIGHT, 60, 0.0001, 10000) {}

void BRU12App::setup() {
    camera.lookAt(vec3(4.0, 5.0, 6.0), vec3(4.0, 0.0, 1.0));

    auto glsl = gl::GlslProg::Format()
        .vertex(loadAsset("../Resources/vert.glsl"))
        .fragment(loadAsset("../Resources/frag.glsl"));
    glslProg = gl::GlslProg::create(glsl);

    indices.reserve(INDEX_COUNT);
    for (uint32_t i = 0; i < NODE_COUNT; i++) {
        indices.push_back(i);
        indices.push_back(rand() % NODE_COUNT);
        indices.push_back(rand() % NODE_COUNT);
    }

    recreateVBOs();

//    meshBatch = gl::Batch::create(mesh, glslProg);

    gl::enableDepthWrite();
    gl::enableDepthRead();

    initializeOpenVDB();
}

void BRU12App::initializeOpenVDB() {
    openvdb::initialize();

    FloatGridType grid(1.0f);
    openvdb::CoordBBox bbox(0.0f, 0.0f, 0.0f, 80.0f, 20.0f, 20.0f);
    grid.tree().fill(bbox, -1.0f);

    openvdb::math::Mat4d mat = openvdb::math::Mat4d::identity();
    auto linearTransform = openvdb::math::Transform::createLinearTransform(mat);
    linearTransform->preScale(openvdb::Vec3d(0.1, 0.1, 0.1));
    grid.setTransform(linearTransform);

    vector<openvdb::Vec3I> triangles;
    vector<Node> nodes;
    gridToMesh(grid, nodes, triangles);

    geom::BufferLayout layout;
    layout.append(geom::Attrib::POSITION, 3, sizeof(Node), offsetof(Node, position));
    layout.append(geom::Attrib::COLOR, 3, sizeof(Node), offsetof(Node, color));

    auto vbo = gl::Vbo::create(GL_ARRAY_BUFFER, nodes, GL_STATIC_DRAW);
    volumeMesh = gl::VboMesh::create((uint32_t) nodes.size(), GL_TRIANGLES, {{ layout, vbo }},
                                     (uint32_t) triangles.size() * 3, GL_UNSIGNED_INT);
    volumeMesh->bufferIndices(triangles.size() * 3 * sizeof(uint32_t), triangles.data());
}

// Heavily tweaked doVolumeToMesh from VolumeToMesh.h
void BRU12App::gridToMesh(const FloatGridType& grid, vector<Node>& nodes, vector<openvdb::Vec3I>& triangles) {
    openvdb::tools::VolumeToMesh mesher(0.0, 0.0, true);
    mesher(grid);

    // Preallocate the point list
    nodes.clear();
    nodes.resize(mesher.pointListSize());

    // Make nodes out of points
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        auto colorDistribution = uniform_real_distribution<double>(0.0, 1.0);

        boost::scoped_array<openvdb::Vec3s>& pointList = mesher.pointList();
        for (size_t i = 0; i < mesher.pointListSize(); i++) {
            openvdb::Vec3s& position = pointList[i];
            Node node {
                .position = vec3(position.x(), position.y(), position.z()),
//                .color = Color(colorDistribution(gen), colorDistribution(gen), colorDistribution(gen))
                .color = Color(0.0, 0.0, position.y() / 2.0)
            };
            nodes[i] = node;
        }

        mesher.pointList().reset(nullptr);
    }

    openvdb::tools::PolygonPoolList& polygonPoolList = mesher.polygonPoolList();

    { // Preallocate primitive lists
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

void BRU12App::recreateVBOs() {
    const vector<Node>& nodes = process.getNodes();
    auto layout = process.getLayout();

    vbo = gl::Vbo::create(GL_ARRAY_BUFFER, nodes, GL_STREAM_DRAW);

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
    mesh = gl::VboMesh::create((uint32_t) nodes.size(), GL_POINTS, {{ layout, vbo }});
}

void BRU12App::mouseDown(MouseEvent event) {
    uint32_t newIndexCount = 3 + rand() % (INDEX_COUNT - 3);
    indices.resize(newIndexCount);
    for (uint32_t i = 0; i < indices.size(); i++) {
        indices[i] = rand() % NODE_COUNT;
    }

    recreateVBOs();
}

void BRU12App::keyDown(KeyEvent event) {
    switch(event.getCode()) {
        case KeyEvent::KEY_s:
            writeImage(Platform::get()->getHomeDirectory() / ("image_" + toString(time(nullptr)) + ".png"),
                       copyWindowSurface());
            break;
        case KeyEvent::KEY_w:
            wireframe = !wireframe;
            if (wireframe) gl::enableWireframe();
            else gl::disableWireframe();
            break;
    }
}


void BRU12App::update() {
    // Radi, ali nije najsjajnije jer je interleaved buffer:
//    auto mappedPosition = mesh->mapAttrib3f(geom::Attrib::POSITION, false);
////    auto mappedColor = mesh->mapAttrib3f(geom::Attrib::COLOR, false);
//
//
//    for(int i = 0; i < mesh->getNumVertices(); i++) {
//        vec3& position = *mappedPosition;
//        mappedPosition->y = position.y + 0.01;
//        ++mappedPosition;
//
////        vec3& color = *mappedColor;
////        ++mappedColor;
//    }
//
////    mappedColor.unmap();
//    mappedPosition.unmap();

    // Radi, ali treperi:
//    auto nodeVbo = mesh->getVertexArrayVbos()[0];
//    Node* node = (Node*) nodeVbo->mapReplace();
//    for (int i = 0; i < nodes.size(); i++) {
//        node->position.y += 0.01;
//        node++;
//    }
//    nodeVbo->unmap();

    process.update();

    const vector<Node>& nodes = process.getNodes();
    auto mappedBuffer = vbo->mapWriteOnly();
    memcpy(mappedBuffer, nodes.data(), nodes.size() * sizeof(Node));
    vbo->unmap();
}

void BRU12App::draw() {
    gl::clear(Color(0.9f, 0.9f, 0.9f));
    gl::setMatrices(camera);

    {
        gl::ScopedGlslProg glsl(glslProg);
//        glPolygonMode(GL_FRONT_AND_BACK, GL_LINES);
        gl::pointSize(4.0f);
//        gl::draw(mesh);

        gl::color(0.0f, 0.0f, 0.0f);
        gl::draw(volumeMesh);
    }

//    meshBatch->draw();
}

CINDER_APP(BRU12App, RendererGl, [&] (App::Settings *settings) {
    settings->setWindowSize(WIDTH, HEIGHT);
    settings->setHighDensityDisplayEnabled();
})
