#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG_LEVEL 1

#if DEBUG_LEVEL > 0

#include <glm/glm.hpp>
#include <iostream>

static void printVec2(const glm::vec2& vec, bool add_endl=true) {
    std::cout << "[" << vec[0] << ", " << vec[1] << "]";
    if (add_endl) {
        std::cout << std::endl;
    }
}

static void printVec3(const glm::vec3& vec, bool add_endl=true) {
    std::cout << "[" << vec[0];
    for (int32_t x = 1; x < 3; x++) {
        std::cout << ", " << vec[x];
    }
    std::cout << "]";
    if (add_endl) {
        std::cout << std::endl;
    }
}

static void printVec4(const glm::vec4& vec) {
    std::cout << "[";
    for (int32_t x = 0; x < 4; x++) {
        std::cout << vec[x] << ", ";
    }
    std::cout << "]" << std::endl;
}

static void printMat4(const glm::mat4& mat) {
    std::cout << "[" << std::endl;
    for (int32_t y = 0; y < 4; y++) {
        for (int32_t x = 0; x < 4; x++) {
            std::cout << mat[x][y] << ", ";
        }
        std::cout << std::endl;
    }
    std::cout << "]" << std::endl;
}

#include <GL/glew.h>

#else
#define printVec3(...)
#define printVec4(...)
#define printMat4(...)
#define glErrorCheck()
#define verifyVAO(...)


#endif

#endif // DEBUG_H
