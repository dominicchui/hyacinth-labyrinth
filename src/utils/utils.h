#ifndef UTILS_H
#define UTILS_H

#include <glm/glm.hpp>
#include <iostream>

static constexpr float epsilon = 1e-7;
static constexpr float eps_dbl = 1e-22;

static constexpr glm::vec4 zero_pt(0.f, 0.f, 0.f, 1.f);
static constexpr glm::vec4 zero_dir(0.f, 0.f, 0.f, 0.f);

static constexpr glm::vec4 inf_pt(INFINITY, INFINITY, INFINITY, 1.f);
static constexpr glm::vec4 inf_dir(INFINITY, INFINITY, INFINITY, 0.f);

static constexpr glm::vec4 neg_inf_pt(-INFINITY, -INFINITY, -INFINITY, 1.f);
static constexpr glm::vec4 neg_inf_dir(-INFINITY, -INFINITY, -INFINITY, 0.f);


// static GLint myGlGetUniformLocation(GLuint shader, const GLchar* name) {
//     GLint loc = glGetUniformLocation(shader, name);
//     if (loc < 0) {
//         std::cerr << "glGetUniformLocation(\"" << name << "\") returned " << loc << std::endl;
//     }
//     return loc;
// }

#endif // UTILS_H
