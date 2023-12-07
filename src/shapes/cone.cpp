#include "shapes.h"

#include <algorithm>

void Cone::addSideNorm(glm::vec3 &point) {
    insertVec3(point);
    glm::vec3 normal = normalize(glm::vec3(2.f * point[0], (0.5f - point[1]) / 2.f, 2.f * point[2]));
    insertVec3(normal);
}

void Cone::addBaseNorm(glm::vec3 &point) {
    insertVec3(point);
    glm::vec3 normal = normalize(glm::vec3(0, point[1], 0));
    insertVec3(normal);
}

void Cone::makeSideTile(glm::vec3 &topLeft, glm::vec3 &topRight,
                        glm::vec3 &bottomLeft, glm::vec3 &bottomRight) {
    addSideNorm(topLeft);
    addSideNorm(bottomLeft);
    addSideNorm(bottomRight);
    addSideNorm(topRight);
    addSideNorm(topLeft);
    addSideNorm(bottomRight);
}

void Cone::makeBaseTile(glm::vec3 &topLeft, glm::vec3 &topRight,
                        glm::vec3 &bottomLeft, glm::vec3 &bottomRight) {
    addBaseNorm(topLeft);
    addBaseNorm(bottomLeft);
    addBaseNorm(bottomRight);
    addBaseNorm(topRight);
    addBaseNorm(topLeft);
    addBaseNorm(bottomRight);
}

void Cone::makeTip(glm::vec3 &left, glm::vec3 &right) {
    glm::vec3 c = {0, 0.5, 0};
    glm::vec3 lnorm = normalize(glm::vec3(2.f * left[0], (0.5f - left[1]) / 2.f, 2.f * left[2]));
    glm::vec3 rnorm = normalize(glm::vec3(2.f * right[0], (0.5f - right[1]) / 2.f, 2.f * right[2]));
    glm::vec3 cnorm = normalize((lnorm + rnorm) / 2.f);

    insertVec3(left);
    insertVec3(lnorm);
    insertVec3(c);
    insertVec3(cnorm);
    insertVec3(right);
    insertVec3(rnorm);
}

void Cone::makeBaseCenter(glm::vec3 &left, glm::vec3 &right) {
    glm::vec3 c = {0, -0.5, 0};
    addBaseNorm(left);
    addBaseNorm(c);
    addBaseNorm(right);
    addBaseNorm(right);
    addBaseNorm(c);
    addBaseNorm(left);
}

void Cone::makeBase(glm::vec3 &left, glm::vec3 &right) {
    glm::vec3 center = {0, -0.5, 0};
    glm::vec3 leftSide = (left - center) / (float) param1;
    glm::vec3 rightSide = (right - center) / (float) param1;

    glm::vec3 leftBase = center + leftSide;
    glm::vec3 rightBase = center + rightSide;
    makeBaseCenter(leftBase, rightBase);

    for (int i = 1; i < param1; i++) {
        glm::vec3 leftClose  = center + float(i) * leftSide;
        glm::vec3 rightClose = center + float(i) * rightSide;
        glm::vec3 leftFar    = center + float(i+1) * leftSide;
        glm::vec3 rightFar   = center + float(i+1) * rightSide;

        makeBaseTile(leftClose, rightClose, leftFar, rightFar);
    }
}

void Cone::makeSide(glm::vec3 &left, glm::vec3 &right) {
    glm::vec3 center = {0, 0.5, 0};
    glm::vec3 leftSide = (left - center) / (float) param1;
    glm::vec3 rightSide = (right - center) / (float) param1;

    glm::vec3 leftTip = center + leftSide;
    glm::vec3 rightTip = center + rightSide;
    makeTip(leftTip, rightTip);

    for (int i = 1; i < param1; i++) {
        glm::vec3 leftClose  = center + float(i) * leftSide;
        glm::vec3 rightClose = center + float(i) * rightSide;
        glm::vec3 leftFar    = center + float(i+1) * leftSide;
        glm::vec3 rightFar   = center + float(i+1) * rightSide;

        makeSideTile(rightClose, leftClose, rightFar, leftFar);
    }
}

void Cone::makeWedge(float startTheta, float endTheta) {
    glm::vec3 left  = {sin(endTheta),   -1, cos(endTheta)};
    glm::vec3 right = {sin(startTheta), -1, cos(startTheta)};
    left *= radius;
    right *= radius;

    makeSide(left, right);
    makeBase(left, right);
}

void Cone::setVertexData() {
    param1 = std::max(param1, 1);
    param2 = std::max(param2, 3);
    float thetaStep = glm::radians(360.f / param2);

    for (int i = 0; i < param2; i++) {
        float startTheta = thetaStep * float(i);
        float endTheta = thetaStep * (float(i) + 1.f);
        makeWedge(startTheta, endTheta);
    }
}
