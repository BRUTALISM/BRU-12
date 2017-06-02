#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/app/Platform.h"
#include "cinder/Log.h"
#include "cinder/Utilities.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"

#include <random>

#include "MeshNode.hpp"
#include "FPSCamera.hpp"
#include "BRU12Pipeline.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

const int WIDTH = 1200;
const int HEIGHT = 768;
const int SCREENSHOT_HEIGHT = 2880;
const float CAMERA_MOVEMENT_SPEED = 0.1f;
const float CAMERA_MOUSE_SENSITIVITY = 0.02f;
const Color BACKGROUND_COLOR = Color(0.0f, 0.0f, 0.0f);

const Params PIPELINE_PARAMS = {
    .volumeBounds = vec3(40.0f, 10.0f, 2.0f),
    .densityPerUnit = 8,
    .gridBackgroundValue = 0.0f,
    .gridFillValue = 1.0f,
    .isoValue = 0.98f,
    .decayMultiplier = 0.003f
};

class BRU12App : public App {
public:
    void mouseDrag(MouseEvent event) override;
    void mouseDown(MouseEvent event) override;
    void mouseUp(MouseEvent event) override;
    void keyDown(KeyEvent event) override;
    void keyUp(KeyEvent event) override;

    void takeScreenshot(int height);

    void setup() override;
    void update() override;
    void draw() override;

    BRU12App();

private:
    shared_ptr<FPSCamera> fpsCamera;
    gl::GlslProgRef	glslProg;

    bool wireframe = false;
    bool updateProcess = true;

    BRU12Pipeline pipeline;
    ci::gl::VboMeshRef latestMesh;
};

BRU12App::BRU12App() : pipeline(PIPELINE_PARAMS) {
    auto cinderCamera = CameraPersp(WIDTH, HEIGHT, 60, 0.0001, 10000);
    vec3 bounds = vec3(10.0f, 10.0f, 10.0f);
    vec3 center = bounds * 0.5f;
    vec3 eye = center + vec3(center.x - 0.2f * bounds.x, -bounds.y * 0.8f, -bounds.z);
    cinderCamera.lookAt(eye, center);

    fpsCamera = make_shared<FPSCamera>(cinderCamera);
    fpsCamera->setSpeed(CAMERA_MOVEMENT_SPEED);
    fpsCamera->setMouseSensitivity(CAMERA_MOUSE_SENSITIVITY);

    getWindow()->getSignalResize().connect([this] {
        auto windowSize = getWindow()->getSize();
        auto aspectRatio = ((float) windowSize.x) / windowSize.y;
        fpsCamera->getCamera().setAspectRatio(aspectRatio);
    });
}

void BRU12App::setup() {
    auto glsl = gl::GlslProg::Format()
        .vertex(loadAsset("vert.glsl"))
        .fragment(loadAsset("frag.glsl"));
    glslProg = gl::GlslProg::create(glsl);

    gl::enableDepthWrite();
    gl::enableDepthRead();

    pipeline.nextIteration();
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

void BRU12App::keyDown(KeyEvent event) {
    switch(event.getCode()) {
        case KeyEvent::KEY_i:
            takeScreenshot(SCREENSHOT_HEIGHT);
            break;
        case KeyEvent::KEY_f:
            wireframe = !wireframe;
            if (wireframe) gl::enableWireframe();
            else gl::disableWireframe();
            break;
        case KeyEvent::KEY_p:
            updateProcess = !updateProcess;
            if (updateProcess) {
                pipeline.nextIteration();
            }
            break;
    }

    fpsCamera->keyDown(event);
}

void BRU12App::keyUp(KeyEvent event) {
    fpsCamera->keyUp(event);
}

void BRU12App::update() {
    fpsCamera->update();

    auto& outQueue = pipeline.getOutputQueue();
    if (!outQueue->isEmpty()) {
        auto result = outQueue->tryPop(chrono::milliseconds(1));
        if (result) {
            auto unwrappedResult = *result;
            latestMesh = unwrappedResult.mesh;

            if (updateProcess) {
                pipeline.nextIteration();
            }
        }
    }
}

void BRU12App::draw() {
    gl::clear(BACKGROUND_COLOR);
    gl::setMatrices(fpsCamera->getCamera());

    if (latestMesh.get()) {
        gl::ScopedGlslProg glsl(glslProg);
        gl::draw(latestMesh);
    }
}

void BRU12App::takeScreenshot(int height) {
    auto window = getWindow();
    auto renderer = getWindow()->getRenderer();
    auto windowSize = window->getSize();
    auto aspectRatio = ((float) windowSize.x) / windowSize.y;
    auto bounds = ivec2((int) height * aspectRatio, height);
    auto framebuffer = gl::Fbo::create(bounds.x, bounds.y);

    auto originalViewport = gl::getViewport();
    framebuffer->bindFramebuffer();
    gl::viewport(std::pair<ivec2, ivec2> { ivec2(), bounds });

    draw();

    auto homeDirectory = Platform::get()->getHomeDirectory();
    auto imageName = "image_" + toString(time(nullptr)) + ".png";
    writeImage(homeDirectory / "Desktop" / imageName, framebuffer->getColorTexture()->createSource());

    framebuffer->unbindFramebuffer();
    gl::viewport(originalViewport);
}

CINDER_APP(BRU12App, RendererGl, [&] (App::Settings *settings) {
    settings->setWindowSize(WIDTH, HEIGHT);
    settings->setHighDensityDisplayEnabled();
})
