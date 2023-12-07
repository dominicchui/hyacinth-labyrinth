#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG_LEVEL 1

#if DEBUG_LEVEL > 0

#include <glm/glm.hpp>
#include <iostream>

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

namespace Debug
{
// Task 2: Add file name and line number parameters
static inline void glErrorCheck(const char* fname, int32_t line) {
    GLenum errorNumber = glGetError();
    while (errorNumber != GL_NO_ERROR) {
        // Task 2: Edit this print statement to be more descriptive
        std::cerr << fname << ":" << line << " " <<
            errorNumber << std::endl;

        errorNumber = glGetError();
    }
}

static void verifyVAO(
    const char* fname,
    int32_t line,
    std::vector<GLfloat> &triangleData,
    GLuint index,
    GLsizei size,
    GLsizei stride,
    const void* offset
) {
    int newStride = int(stride / 4);
    int groupNum = 0;
    int newOffset = static_cast<int>(reinterpret_cast<intptr_t>(offset)) / 4;

    std::cerr << fname << ":" << line << ":" << std::endl;

    for (int i = newOffset; i < triangleData.size(); i = i + newStride) {
        std::cerr << "Group " << groupNum << " of Values for VAO index " << index << std::endl;
        std::cerr << "[";
        for (auto j = i; j < i + size; ++j) {
            if (j != i + size - 1) {
                std::cerr << triangleData[j]<< ", ";
            } else {
                std::cerr << triangleData[j]<< "]" << std::endl;
            }
        }
        groupNum = groupNum + 1;
    }
    std::cout << "" << std::endl;
}

}

#define glErrorCheck() Debug::glErrorCheck(__FILE__, __LINE__)
#define verifyVAO(tri_data, index, size, stride, offset)    \
    Debug::verifyVAO(__FILE__, __LINE__, tri_data, index, size, stride, offset)

#else
#define printVec4(...)
#define printMat4(...)
#define glErrorCheck()
#define verifyVAO(...)


#endif

#endif // DEBUG_H
