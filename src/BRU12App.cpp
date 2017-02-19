#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Perlin.h"
#include <random>

using namespace ci;
using namespace ci::app;
using namespace std;

#define WIDTH 1200
#define HEIGHT 768
#define NODE_COUNT 40
#define INDEX_COUNT NODE_COUNT * 3

struct Node {
    vec3 position;
    Color color;
};

class BRU12App : public App {
public:
    void setup() override;
    void mouseDown(MouseEvent event) override;
    void update() override;
    void draw() override;

    BRU12App();

    CameraPersp camera;
    gl::GlslProgRef	glslProg;

    vector<uint32_t> indices;
    vector<Node> nodes;

    gl::VboRef vbo;
    gl::VboRef indexVbo;
    geom::BufferLayout layout;
    gl::VboMeshRef mesh;
//    gl::BatchRef meshBatch;

    Perlin perlin;

private:
    void recreateVBOs();
};

BRU12App::BRU12App() : camera(WIDTH, HEIGHT, 90, 0.0001, 10000) {}

void BRU12App::setup() {
    camera.lookAt(vec3(5.0, 0.0, 7.0), vec3(5.0, 0.0, 1.0));

    auto glsl = gl::GlslProg::Format()
        .vertex(loadAsset("../Resources/vert.glsl"))
        .fragment(loadAsset("../Resources/frag.glsl"));
    glslProg = gl::GlslProg::create( glsl );

    std::random_device rd;
    std::mt19937 gen(rd());
    auto xDistribution = uniform_real_distribution<double>(0.0, 10.0);
    auto yzDistribution = uniform_real_distribution<double>(0.0, 2.0);
    auto colorDistribution = uniform_real_distribution<double>(0.0, 1.0);

    nodes.reserve(NODE_COUNT);
    indices.reserve(INDEX_COUNT);

    for (uint32_t i = 0; i < NODE_COUNT; i++) {
        Node node;
        node.position = vec3(xDistribution(gen), yzDistribution(gen), yzDistribution(gen));
        node.color = Color(colorDistribution(gen), 0.0, 0.0);
        nodes.push_back(node);

        indices.push_back(i);
        indices.push_back(rand() % NODE_COUNT);
        indices.push_back(rand() % NODE_COUNT);
    }

    layout.append(geom::Attrib::POSITION, 3, sizeof(Node), offsetof(Node, position));
    layout.append(geom::Attrib::COLOR, 3, sizeof(Node), offsetof(Node, color));

    recreateVBOs();

//    meshBatch = gl::Batch::create(mesh, glslProg);

    gl::enableDepthWrite();
    gl::enableDepthRead();
}

void BRU12App::recreateVBOs() {
    vbo = gl::Vbo::create(GL_ARRAY_BUFFER, nodes, GL_STREAM_DRAW);

    // Ovo je jedan nacin da furaš indekse - preko zasebnog index VBO niza, pa da onda nad njim
    // radiš mapiranje i memcpy
//    indexVbo = gl::Vbo::create(GL_ELEMENT_ARRAY_BUFFER, indices);
//    mesh = gl::VboMesh::create((uint32_t) nodes.size(), GL_TRIANGLES, {{ layout, vbo }},
//                               (uint32_t) indices.size(), GL_UNSIGNED_INT, indexVbo);

    // Ovo je drugi, verovatno jednostavniji način da se indeksira
    mesh = gl::VboMesh::create((uint32_t) nodes.size(), GL_TRIANGLES, {{ layout, vbo }},
                               (uint32_t) indices.size(), GL_UNSIGNED_INT);
    mesh->bufferIndices(indices.size() * sizeof(uint32_t), indices.data());

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void BRU12App::mouseDown(MouseEvent event) {
    uint32_t newIndexCount = 3 + rand() % (INDEX_COUNT - 3);
    indices.resize(newIndexCount);
    for (uint32_t i = 0; i < indices.size(); i++) {
        indices[i] = rand() % NODE_COUNT;
    }

    recreateVBOs();
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

    const float timeFactor = 2.9f;
    const float perlinScale = 0.07f;
    for (Node& node : nodes) {
        vec3 perlinCoord(node.position.x,
                         getElapsedSeconds() * timeFactor,
                         node.position.z);
        node.position.y += perlin.fBm(perlinCoord) * perlinScale;
    }

    auto mappedBuffer = vbo->mapWriteOnly();
    memcpy(mappedBuffer, nodes.data(), nodes.size() * sizeof(Node));
    vbo->unmap();
}

void BRU12App::draw() {
    gl::clear(Color(0.9f, 0.9f, 0.9f));
    gl::setMatrices(camera);

    {
        gl::ScopedGlslProg glsl(glslProg);
        gl::draw(mesh);
    }

//    meshBatch->draw();
}

CINDER_APP(BRU12App, RendererGl, [&] (App::Settings *settings) {
    settings->setWindowSize(WIDTH, HEIGHT);
    settings->setHighDensityDisplayEnabled();
})
