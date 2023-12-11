#include "shapes.h"

glm::vec3 Sphere::getCoord(float r, float theta, float phi) {
    return {
        r * sin(phi) * sin(theta),
        r * cos(phi),
        r * sin(phi) * cos(theta)
    };
}

void Sphere::addTriNorm(glm::vec3 &point) {
    insertVec3(point);
    glm::vec3 normal = normalize(point);
    insertVec3(normal);
}

void Sphere::makeTile(glm::vec3 &topLeft, glm::vec3 &topRight,
                      glm::vec3 &bottomLeft, glm::vec3 &bottomRight) {
    addTriNorm(topLeft);
    addTriNorm(bottomLeft);
    addTriNorm(bottomRight);
    addTriNorm(topRight);
    addTriNorm(topLeft);
    addTriNorm(bottomRight);
}

void Sphere::makeWedge(float startTheta, float endTheta) {
    float phiStep = glm::radians(180.f / param1);
    float r = 0.5;

    for (int i = 0; i < param1; i++) {
        float startPhi = phiStep * (float) i;
        float endPhi = (i + 1) * phiStep;
        glm::vec3 tl = getCoord(r, startTheta, startPhi);
        glm::vec3 tr = getCoord(r, endTheta,   startPhi);
        glm::vec3 bl = getCoord(r, startTheta, endPhi);
        glm::vec3 br = getCoord(r, endTheta,   endPhi);
        makeTile(tl, tr, bl, br);
    }
}

void Sphere::setVertexData() {
    param1 = std::max(param1, 2);
    param2 = std::max(param2, 3);
    float thetaStep = glm::radians(360.f / param2);

    for (int i = 0; i < param2; i++) {
        float startTheta = thetaStep * (float) i;
        float endTheta = thetaStep * ((float) i + 1.f);
        makeWedge(startTheta, endTheta);
    }
}

