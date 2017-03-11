#include "FPSCamera.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

FPSCamera::FPSCamera() {
    currentCam = ci::CameraPersp();
    speed = 1.0f;
    positionVelocity = vec3();
    orientation = quat();
    upIsDown = downIsDown = leftIsDown = rightIsDown = mouseIsDown = higherIsDown = lowerIsDown = false;
    mouseSensitivity = 0.1f;
}

FPSCamera::FPSCamera(const ci::CameraPersp &initialCam) {
    currentCam = initialCam;
    speed = 1.0f;
    positionVelocity = vec3();
    orientation = currentCam.getOrientation();
    upIsDown = downIsDown = leftIsDown = rightIsDown = mouseIsDown = higherIsDown = lowerIsDown = false;
    mouseSensitivity = 0.1f;
}

FPSCamera::~FPSCamera() {}

void FPSCamera::forward() {
    positionVelocity += currentCam.getViewDirection() * speed;
}

void FPSCamera::back() {
    positionVelocity -= currentCam.getViewDirection() * speed;
}

void FPSCamera::left() {
    positionVelocity -= cross(currentCam.getViewDirection(), vec3(0, 1, 0)) * speed;
}

void FPSCamera::right() {
    positionVelocity += cross(currentCam.getViewDirection(), vec3(0, 1, 0)) * speed;
}

void FPSCamera::higher() {
    positionVelocity += vec3(0, 1, 0)* speed;
}

void FPSCamera::lower() {
    positionVelocity -= vec3(0, 1, 0) * speed;
}

void FPSCamera::update(bool yContraint, float y) {
    if (upIsDown) forward();
    if (downIsDown) back();
    if (leftIsDown) left();
    if (rightIsDown) right();
    if (higherIsDown) higher();
    if (lowerIsDown) lower();

    if (mouseIsDown) {
        timeElapsed = App::get()->getElapsedSeconds() - lastTime;

        orientation = quat(vec3(mouseDelta.y * mouseSensitivity, 0.0f, 0.0f)) * currentCam.getOrientation() * quat(vec3(0.0f, -mouseDelta.x * mouseSensitivity, 0.0f));
        mouseDelta = vec2();

        lastTime = App::get()->getElapsedSeconds();
    }

    vec3 eyePoint = currentCam.getEyePoint() + positionVelocity;
    if (yContraint) {
        eyePoint.y = y;
    }

    quat targetOrientation = mix(currentCam.getOrientation(), orientation, 0.2f);
    currentCam.setOrientation(targetOrientation);

    currentCam.setEyePoint(eyePoint);
    positionVelocity *= 0.2f;
}

bool FPSCamera::mouseDown(ci::app::MouseEvent event) {
    mouseIsDown = true;
    lastMousePos = event.getPos();
    App::get()->hideCursor();
    return false;
}

bool FPSCamera::mouseDrag(ci::app::MouseEvent event){
    vec2 currentPos = event.getPos();
    mouseDelta = currentPos - lastMousePos;
    lastMousePos = currentPos;
    return false;
}

bool FPSCamera::mouseUp(ci::app::MouseEvent event) {
    mouseIsDown = false;
    lastMousePos = event.getPos();
    mouseDelta = vec2();
    App::get()->showCursor();
    return false;
}

bool FPSCamera::keyDown(KeyEvent event) {
    int code = event.getCode();
    if (code == 'w' || code == KeyEvent::KEY_UP) upIsDown = true;
    else if (code == 's' || code == KeyEvent::KEY_DOWN) downIsDown = true;
    else if (code == 'a' || code == KeyEvent::KEY_LEFT) leftIsDown = true;
    else if (code == 'd' || code == KeyEvent::KEY_RIGHT) rightIsDown = true;
    else if (code == 'q') higherIsDown = true;
    else if (code == 'z') lowerIsDown = true;
    return false;
}

bool FPSCamera::keyUp(KeyEvent event) {
    int code = event.getCode();
    if (code == 'w' || code == KeyEvent::KEY_UP) upIsDown = false;
    else if (code == 's' || code == KeyEvent::KEY_DOWN) downIsDown = false;
    else if (code == 'a' || code == KeyEvent::KEY_LEFT) leftIsDown = false;
    else if (code == 'd' || code == KeyEvent::KEY_RIGHT) rightIsDown = false;
    else if (code == 'q') higherIsDown = false;
    else if (code == 'z') lowerIsDown = false;
    return false;
}
