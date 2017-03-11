#include "FPSCamera.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

const vec3 WORLD_RIGHT = vec3(1, 0, 0);
const vec3 WORLD_UP = vec3(0, 1, 0);

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
        orientation = currentCam.getOrientation();
        auto pitch = -mouseDelta.y * mouseSensitivity;
        auto yaw = -mouseDelta.x * mouseSensitivity;
        auto deltaRotation = quat(vec3(pitch, yaw, 0));

        orientation *= deltaRotation;
        orientation = normalize(orientation);

        mouseDelta = vec2();
    }

    quat targetOrientation = mix(currentCam.getOrientation(), orientation, 0.2f);
    currentCam.setOrientation(targetOrientation);

    vec3 eyePoint = currentCam.getEyePoint() + positionVelocity;
    positionVelocity *= 0.2f;
    if (yContraint) {
        eyePoint.y = y;
    }
    currentCam.setEyePoint(eyePoint);
    currentCam.setWorldUp(WORLD_UP);
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
