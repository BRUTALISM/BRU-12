#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/app/Platform.h"
#include "cinder/Log.h"
#include "cinder/Utilities.h"
#include "cinder/gl/gl.h"

#include <random>

#include "Node.hpp"
#include "Process.hpp"
#include "FPSCamera.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

const int WIDTH = 1200;
const int HEIGHT = 768;
const float CAMERA_MOVEMENT_SPEED = 0.2f;
const float CAMERA_MOUSE_SENSITIVITY = 0.05f;
const Color BACKGROUND_COLOR = Color(0.9f, 0.9f, 0.9f);

class BRU12App : public App {
public:
    void mouseDrag(MouseEvent event) override;
    void mouseMove(MouseEvent event) override;
    void mouseDown(MouseEvent event) override;
    void mouseUp(MouseEvent event) override;
    void keyDown(KeyEvent event) override;
    void keyUp(KeyEvent event) override;

    void setup() override;
    void update() override;
    void draw() override;

    BRU12App();

private:
    shared_ptr<Process> process;

    shared_ptr<FPSCamera> fpsCamera;
    gl::GlslProgRef	glslProg;

    bool wireframe = false;
};

BRU12App::BRU12App() {
    Process::Params params {
        .volumeBounds = vec3(10.0f, 10.0f, 10.0f),
        .densityPerUnit = 10
    };
    process = make_shared<Process>(params);

    auto cinderCamera = CameraPersp(WIDTH, HEIGHT, 60, 0.0001, 10000);
    vec3 bounds = process->getParams().volumeBounds;
    vec3 center = bounds * 0.5f;
    vec3 eye = center + vec3(center.x - 0.2f * bounds.x, -bounds.y * 0.8f, -bounds.z);
    cinderCamera.lookAt(eye, center);

    fpsCamera = make_shared<FPSCamera>(cinderCamera);
    fpsCamera->setSpeed(CAMERA_MOVEMENT_SPEED);
    fpsCamera->setMouseSensitivity(CAMERA_MOUSE_SENSITIVITY);
}

void BRU12App::setup() {
    auto glsl = gl::GlslProg::Format()
        .vertex(loadAsset("../Resources/vert.glsl"))
        .fragment(loadAsset("../Resources/frag.glsl"));
    glslProg = gl::GlslProg::create(glsl);

    gl::enableDepthWrite();
    gl::enableDepthRead();
}

void BRU12App::mouseDown(MouseEvent event) {
    fpsCamera->mouseDown(event);
}

void BRU12App::mouseUp(MouseEvent event) {
    fpsCamera->mouseUp(event);
}

void BRU12App::mouseDrag(MouseEvent event) {
    fpsCamera->mouseDrag(event);
}

void BRU12App::mouseMove(MouseEvent event) {
    //
}

void BRU12App::keyDown(KeyEvent event) {
    switch(event.getCode()) {
//        case KeyEvent::KEY_s:
//            writeImage(Platform::get()->getHomeDirectory() / ("image_" + toString(time(nullptr)) + ".png"),
//                       copyWindowSurface());
//            break;
        case KeyEvent::KEY_f:
            wireframe = !wireframe;
            if (wireframe) gl::enableWireframe();
            else gl::disableWireframe();
            break;
    }

    fpsCamera->keyDown(event);
}

void BRU12App::keyUp(KeyEvent event) {
    fpsCamera->keyUp(event);
}

void BRU12App::update() {
    /*
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
     */

    fpsCamera->update();
//    process->update();

    /*
    const vector<Node>& nodes = process.getNodes();
    auto mappedBuffer = vbo->mapWriteOnly();
    memcpy(mappedBuffer, nodes.data(), nodes.size() * sizeof(Node));
    vbo->unmap();
     */
}

void BRU12App::draw() {
    gl::clear(BACKGROUND_COLOR);
    gl::setMatrices(fpsCamera->getCamera());

    {
        gl::ScopedGlslProg glsl(glslProg);
//        glPolygonMode(GL_FRONT_AND_BACK, GL_LINES);
        gl::pointSize(4.0f);
//        gl::draw(mesh);

        gl::color(0.0f, 0.0f, 0.0f);
        gl::draw(process->getMesh());
    }

//    meshBatch->draw();
}

CINDER_APP(BRU12App, RendererGl, [&] (App::Settings *settings) {
    settings->setWindowSize(WIDTH, HEIGHT);
    settings->setHighDensityDisplayEnabled();
})
