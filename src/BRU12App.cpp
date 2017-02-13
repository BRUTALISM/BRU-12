#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include <random>

using namespace ci;
using namespace ci::app;
using namespace std;

#define WIDTH 1200
#define HEIGHT 768
#define NODE_COUNT 100
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

    geom::BufferLayout layout;
    gl::VboMeshRef mesh;
//    gl::BatchRef meshBatch;
};

BRU12App::BRU12App() : camera(WIDTH, HEIGHT, 90, 0.0001, 10000) {}

void BRU12App::setup() {
    camera.lookAt(vec3(5.0, 1.0, 7.0), vec3(5.0, 1.0, 1.0));

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
    indices.reserve(NODE_COUNT * 3);
    for (uint32_t i = 0; i < NODE_COUNT; i++) {
        Node node;
        node.position = vec3(xDistribution(gen), yzDistribution(gen), yzDistribution(gen));
        node.color = Color(colorDistribution(gen), colorDistribution(gen), colorDistribution(gen));
        nodes.push_back(node);

        indices.push_back(i);
        indices.push_back(rand() % NODE_COUNT);
        indices.push_back(rand() % NODE_COUNT);
    }

    gl::VboRef vbo = gl::Vbo::create(GL_ARRAY_BUFFER, nodes, GL_DYNAMIC_DRAW);

    layout.append(geom::Attrib::POSITION, 3, sizeof(vec3), offsetof(Node, position));
    layout.append(geom::Attrib::COLOR, 4, sizeof(Color), offsetof(Node, color));

    mesh = gl::VboMesh::create((uint32_t) nodes.size(), GL_TRIANGLES, {{ layout, vbo }},
                               (uint32_t) indices.size(), GL_UNSIGNED_INT);
//    mesh->bufferAttrib(geom::Attrib::POSITION, sizeof(vec3) * vertices.size(), vertices.data());
//    mesh->bufferAttrib(geom::Attrib::COLOR, sizeof(Color) * colors.size(), colors.data());
    mesh->bufferIndices(indices.size() * sizeof(uint32_t), indices.data());

//    meshBatch = gl::Batch::create(mesh, glslProg);

    gl::enableDepthWrite();
    gl::enableDepthRead();

    GLint numAttribs;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numAttribs);
    console() << "Number of vertex attributes: " << numAttribs << endl;
}

void BRU12App::mouseDown(MouseEvent event) {
    uint32_t newIndexCount = 3 + rand() % (INDEX_COUNT - 3);
    indices.resize(newIndexCount);
    for (uint32_t i = 0; i < indices.size(); i++) {
        indices[i] = rand() % NODE_COUNT;
    }

    gl::VboRef vbo = gl::Vbo::create(GL_ARRAY_BUFFER, nodes, GL_STATIC_DRAW);
    mesh = gl::VboMesh::create((uint32_t) nodes.size(), GL_TRIANGLES, {{ layout, vbo }},
                               (uint32_t) indices.size(), GL_UNSIGNED_INT);
    mesh->bufferIndices(indices.size() * sizeof(uint32_t), indices.data());
}

void BRU12App::update() {
//    auto positionAttribute = mesh->mapAttrib3f(geom::Attrib::POSITION, false);
//
//    for(int i = 0; i < mesh->getNumVertices(); i++) {
//        vec3& position = *positionAttribute;
//        positionAttribute->y = (float) 1;
//        ++positionAttribute;
//    }
//
//    positionAttribute.unmap();
}

void BRU12App::draw() {
    gl::clear(Color(0.2f, 0.2f, 0.2f));
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
