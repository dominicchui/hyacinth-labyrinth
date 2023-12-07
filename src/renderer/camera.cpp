#include "camera.h"

#include "settings.h"

void Camera::init(SceneCameraData &camdata) {
    look = camdata.look;
    up = camdata.up;
    pos = camdata.pos;
    heightAngle = camdata.heightAngle;

    setView();
    setProj();
}

void Camera::update() {
    setProj();
}

void Camera::setView() {
    glm::vec3 w = normalize(-look);
    glm::vec3 v = normalize(up - dot(up, w) * w);
    glm::vec3 u = cross(v, w);

    glm::mat4 Mr = glm::mat4(
        u[0], v[0], w[0], 0,
        u[1], v[1], w[1], 0,
        u[2], v[2], w[2], 0,
        0,    0,    0,    1
        );

    glm::mat4 Mt = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        -pos.x, -pos.y, -pos.z, 1
    };

    view = Mr * Mt;
}

void Camera::resize(int w, int h) {
    aspectRatio = float(w) / float(h);
    setProj();
}

void Camera::setProj() {
    float invFar = 1.f / settings.farPlane;
    float tan_h = tan(heightAngle / 2.f);
    float tan_w = aspectRatio * tan_h;
    glm::mat4 scale = {
        invFar / tan_w, 0, 0, 0,
        0, invFar / tan_h, 0, 0,
        0, 0, invFar, 0,
        0, 0, 0, 1
    };

    float c = - settings.nearPlane / settings.farPlane;
    glm::mat4 parallel = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1 / (1.f + c), -1,
        0, 0, - c / (1.f + c), 0
    };

    glm::mat4 clip = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, -2, 0,
        0, 0, -1, 1
    };

    proj = clip * parallel * scale;
}

void Camera::moveLook(float dist) {
    pos += dist * normalize(look);
    setView();
}

void Camera::moveSide(float dist) {
    glm::vec3 dir = normalize(cross(look, up));
    pos += dist * dir;
    setView();
}

void Camera::moveVertical(float dist) {
    pos += dist * glm::vec3(0, 1, 0);
    setView();
}

// Calculates the Rodrigues rotation matrix, rotating by
// θ radians around an axis
glm::mat3 rodriguesRot(glm::vec3 axis, float θ) {
    glm::vec3 u = normalize(axis);
    float cosθ = cos(θ);
    float sinθ = sin(θ);

    glm::vec3 u1c = u * (1.f - cosθ);
    glm::vec3 us = u * sinθ;

    return {
        u.x * u1c.x + cosθ,  u.x * u1c.y + us.z,  u.x * u1c.z - us.y,
        u.x * u1c.y - us.z,  u.y * u1c.y + cosθ,  u.y * u1c.z + us.x,
        u.x * u1c.z + us.y,  u.y * u1c.z - us.x,  u.z * u1c.z + cosθ,
    };
}

void Camera::rotate(float deltaX, float deltaY) {
    glm::vec3 worldOrigin = glm::vec4(0,1,0,0) * view;
    glm::vec3 ydir = normalize(cross(look, up));

    look = rodriguesRot(worldOrigin, deltaX / 200.f) * rodriguesRot(ydir, deltaY / 200.f) * look;

    setView();
}
