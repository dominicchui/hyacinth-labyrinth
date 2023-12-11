#include "shapes.h"

void Cube::addTriNorm(glm::vec3 point, glm::vec3 norm) {
    insertVec3(point);
    insertVec3(norm);
}

void Cube::makeTile(glm::vec3 topLeft, glm::vec3 topRight,
                    glm::vec3 bottomLeft, glm::vec3 bottomRight) {
    addTriNorm(topLeft,
        normalize(cross(bottomRight - topLeft, topLeft - bottomLeft)));
    addTriNorm(bottomLeft,
        normalize(cross(topLeft - bottomLeft, bottomLeft - bottomRight)));
    addTriNorm(bottomRight,
        normalize(cross(bottomLeft - bottomRight, bottomRight - topLeft)));

    addTriNorm(topRight,
        normalize(cross(bottomRight - topRight, topRight - topLeft)));
    addTriNorm(topLeft,
        normalize(cross(topRight - topLeft, topLeft - bottomRight)));
    addTriNorm(bottomRight,
        normalize(cross(topLeft - bottomRight, bottomRight - topRight)));
}

void Cube::makeFace(glm::vec3 topLeft, glm::vec3 topRight,
                    glm::vec3 bottomLeft, glm::vec3 bottomRight) {
    glm::vec3 width = (topRight - topLeft) / (float) param1;
    glm::vec3 height = (topLeft - bottomLeft) / (float) param1;

    glm::vec3 tl = topLeft;
    glm::vec3 tr = topLeft + width;
    glm::vec3 bl = topLeft - height;
    glm::vec3 br = bl + width;

    for (int r = 0; r < param1; r++) {
        for (int c = 0; c < param1; c++) {
            glm::vec3 offset = (float) c * width - (float) r * height;
            makeTile(tl + offset, tr + offset, bl + offset, br + offset);
        }
    }
}

void Cube::setVertexData() {
    // front
    makeFace(glm::vec3(-radius,  radius,  radius),
             glm::vec3( radius,  radius,  radius),
             glm::vec3(-radius, -radius,  radius),
             glm::vec3( radius, -radius,  radius));

    // back
    makeFace(glm::vec3(-radius, -radius, -radius),
             glm::vec3( radius, -radius, -radius),
             glm::vec3(-radius,  radius, -radius),
             glm::vec3( radius,  radius, -radius));

    // bottom
    makeFace(glm::vec3(-radius, -radius,  radius),
             glm::vec3( radius, -radius,  radius),
             glm::vec3(-radius, -radius, -radius),
             glm::vec3( radius, -radius, -radius));

    // top
    makeFace(glm::vec3(-radius,  radius, -radius),
             glm::vec3( radius,  radius, -radius),
             glm::vec3(-radius,  radius,  radius),
             glm::vec3( radius,  radius,  radius));

    // left
    makeFace(glm::vec3( radius,  radius,  radius),
             glm::vec3( radius,  radius, -radius),
             glm::vec3( radius, -radius,  radius),
             glm::vec3( radius, -radius, -radius));

    // right
    makeFace(glm::vec3(-radius,  radius, -radius),
             glm::vec3(-radius,  radius,  radius),
             glm::vec3(-radius, -radius, -radius),
             glm::vec3(-radius, -radius,  radius));
}
