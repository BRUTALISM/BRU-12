#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/app/Platform.h"
#include "cinder/Log.h"
#include "cinder/Utilities.h"
#include "cinder/gl/gl.h"

#include "Node.hpp"
#include "Process.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

#define WIDTH 1200
#define HEIGHT 768
#define NODE_COUNT 80
#define INDEX_COUNT NODE_COUNT * 3

class BRU12App : public App {
public:
    void setup() override;
    void mouseDown(MouseEvent event) override;
    void keyDown(KeyEvent event) override;
    void update() override;
    void draw() override;

    BRU12App();

    Process process;

    CameraPersp camera;
    gl::GlslProgRef	glslProg;

    vector<uint32_t> indices;

    gl::VboRef vbo;
    gl::VboRef indexVbo;
    gl::VboMeshRef mesh;
//    gl::BatchRef meshBatch;

private:
    void recreateVBOs();
};

BRU12App::BRU12App() : process(NODE_COUNT), camera(WIDTH, HEIGHT, 90, 0.0001, 10000) {}

void BRU12App::setup() {
    camera.lookAt(vec3(5.0, 0.0, 6.0), vec3(5.0, 0.0, 1.0));

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
}

void BRU12App::recreateVBOs() {
    const vector<Node>& nodes = process.getNodes();
    auto layout = process.getLayout();

    vbo = gl::Vbo::create(GL_ARRAY_BUFFER, nodes, GL_STREAM_DRAW);

    // Ovo je jedan nacin da furaš indekse - preko zasebnog index VBO niza, pa da onda nad njim
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
//        glPolygonMode(GL_FRONT_AND_BACK, GL_TRIANGLES);
        gl::pointSize(4.0f);
        gl::draw(mesh);
    }

//    meshBatch->draw();
}

CINDER_APP(BRU12App, RendererGl, [&] (App::Settings *settings) {
    settings->setWindowSize(WIDTH, HEIGHT);
    settings->setHighDensityDisplayEnabled();
})
