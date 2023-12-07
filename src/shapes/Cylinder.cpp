#include "shapes.h"

void Cylinder::addTriNorm(glm::vec3 &point, glm::vec3 &mask) {
    insertVec3(point);
    glm::vec3 normal = normalize(point * mask);
    insertVec3(normal);
}

void Cylinder::makeTile(glm::vec3 &topLeft, glm::vec3 &topRight,
                        glm::vec3 &bottomLeft, glm::vec3 &bottomRight,
                        glm::vec3 &normmask) {
    addTriNorm(topLeft, normmask);
    addTriNorm(bottomLeft, normmask);
    addTriNorm(bottomRight, normmask);

    addTriNorm(topRight, normmask);
    addTriNorm(topLeft, normmask);
    addTriNorm(bottomRight, normmask);
}

void Cylinder::makeFace(glm::vec3 &topLeft, glm::vec3 &topRight,
                        glm::vec3 &bottomLeft, glm::vec3 &bottomRight) {
    glm::vec3 height = (topLeft - bottomLeft) / (float) param1;
    glm::vec3 facemask = {1, 0, 1};

    for (int i = 0; i < param1; i++) {
        glm::vec3 offset = float(i) * height;
        glm::vec3 tl = topLeft           - offset;
        glm::vec3 tr = topRight          - offset;
        glm::vec3 bl = topLeft - height  - offset;
        glm::vec3 br = topRight - height - offset;

        makeTile(tl, tr, bl, br, facemask);
    }
}

void Cylinder::makeFan(float height, glm::vec3 &left, glm::vec3 &right) {
    bool top = height > 0;
    glm::vec3 center = {0, height, 0}; // y up
    glm::vec3 upnorm = {0, top ? 1 : -1, 0};
    insertVec3(top ? left : right);
    insertVec3(upnorm);
    insertVec3(center);
    insertVec3(upnorm);
    insertVec3(top ? right : left);
    insertVec3(upnorm);
}

void Cylinder::makeCap(float height, glm::vec3 &left, glm::vec3 &right) {
    glm::vec3 center = {0, height, 0};
    glm::vec3 capmask = {0, 1, 0};
    glm::vec3 leftSide = (left - center) / (float) param1;
    glm::vec3 rightSide = (right - center) / (float) param1;

    glm::vec3 leftCap = center + leftSide;
    glm::vec3 rightCap = center + rightSide;
    makeFan(height, leftCap, rightCap);

    for (int i = 1; i < param1; i++) {
        glm::vec3 leftClose  = center + float(i) * leftSide;
        glm::vec3 rightClose = center + float(i) * rightSide;
        glm::vec3 leftFar    = center + float(i+1) * leftSide;
        glm::vec3 rightFar   = center + float(i+1) * rightSide;

        if (height > 0) {
            makeTile(rightClose, leftClose, rightFar, leftFar, capmask);
        } else {
            makeTile(leftClose, rightClose, leftFar, rightFar, capmask);
        }
    }
}

void Cylinder::makeWedge(float startTheta, float endTheta) {
    glm::vec3 topLeft  = {sin(endTheta),    1, cos(endTheta)};
    glm::vec3 topRight = {sin(startTheta),  1, cos(startTheta)};
    glm::vec3 botLeft  = {sin(endTheta),   -1, cos(endTheta)};
    glm::vec3 botRight = {sin(startTheta), -1, cos(startTheta)};
    topLeft  *= radius;
    topRight *= radius;
    botLeft  *= radius;
    botRight *= radius;

    makeCap(0.5, topLeft, topRight);
    makeCap(-0.5, botLeft, botRight);
    makeFace(topRight, topLeft, botRight, botLeft);
}

void Cylinder::setVertexData() {
    param1 = std::max(param1, 1);
    param2 = std::max(param2, 3);
    float thetaStep = glm::radians(360.f / param2);

    for (int i = 0; i < param2; i++) {
        float startTheta = thetaStep * float(i);
        float endTheta = thetaStep * (float(i) + 1.f);
        makeWedge(startTheta, endTheta);
    }
}
