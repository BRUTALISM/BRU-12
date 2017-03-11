#ifndef FPSCamera_hpp
#define FPSCamera_hpp

#include "cinder/Camera.h"

class FPSCamera {
public:
    FPSCamera();
    FPSCamera(const ci::CameraPersp &initialCam);

    ~FPSCamera();

    void update(bool yContraint = false, float y = 0.0f);

    inline ci::CameraPersp& getCamera() { return currentCam; }
    inline void setCamera(const ci::CameraPersp &aCurrentCam) { currentCam = aCurrentCam; }

    inline void setSpeed(float speed) { this->speed = speed; }
    inline float getSpeed() { return speed; }

    inline void setMouseSensitivity(float sensitivity) { mouseSensitivity = sensitivity; }

    bool mouseDown(ci::app::MouseEvent event);
    bool mouseDrag(ci::app::MouseEvent event);
    bool mouseUp(ci::app::MouseEvent event);
    bool keyDown(ci::app::KeyEvent event);
    bool keyUp(ci::app::KeyEvent event);

    void registerEvents();
    void unregisterEvents();

    void forward();
    void back();
    void left();
    void right();
    void higher();
    void lower();

protected:
    void toggleCursorVisibility();

    float speed;
    ci::CameraPersp currentCam;
    ci::vec3 positionVelocity;
    float mouseSensitivity;
    ci::vec2 mouseDelta;
    ci::vec2 lastMousePos;
    ci::quat orientation;
    bool upIsDown, downIsDown, leftIsDown, rightIsDown, higherIsDown, lowerIsDown;
    bool mouseIsDown;
    bool cursorHidden;
};

#endif /* FPSCamera_hpp */
