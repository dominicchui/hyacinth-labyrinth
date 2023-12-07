#pragma once

#include "scene/scenedata.h"

class Camera {
private:
    glm::vec3 look;
    glm::vec3 up;
    glm::vec3 pos;
    float heightAngle;

    float aspectRatio = 4.f / 3.f; // default ratio
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 actionRot = glm::mat4(1); // default as identity

    // sets the view matrix according to the currently loaded data.
    // depends on the camera's look, up, and position vectors
    void setView();

    // sets the projection matrix according to the currently loaded
    // data and the global settings object. depends on near plane,
    // far plane, height angle, and aspect ratio
    void setProj();

public:
    // initializes the camera data and sets up the internal data
    void init(SceneCameraData &camdata);

    // updates the camera using data from the global settings object
    void update();

    // updates the camera to accound for a new screen size
    void resize(int w, int h);

    // gets the view matrix
    glm::mat4 getView() { return view; }

    // gets the position of the camera in worlds space
    glm::vec3 getPos() { return pos; }

    // gets the projection matrix
    glm::mat4 getProj() { return proj; }

    // Moves the camera in the direction of the look vector
    void moveLook(float dist);

    // Moves the camera "to the side", which is the vector perpendicular
    // to the look and up vectors
    void moveSide(float dist);

    // Moves the camera vertically in terms of world space {0,1,0}
    void moveVertical(float dist);

    // Rotates the camera's look vector
    void rotate(float deltaX, float deltaY);

    glm::vec3 getLook() { return {look.x, 0, look.z}; }
    glm::vec3 getSide() { auto v = normalize(cross(up, look)); return {v.x, 0, v.z}; }
    glm::vec3 getUp() { return up; }
};
