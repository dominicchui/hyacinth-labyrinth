#include "keyboard_movement_controller.hpp"
#include "renderer/camera.h"
#include "game/maze.h"
// std
#include <limits>
#include <vector>

void KeyboardMovementController::moveInPlaneXZ(
    GLFWwindow* window,
    float dt,
    LveGameObject& gameObject,
    GameMaze* maze
) {
    float speed_multiplier = 1.f;
    if (glfwGetKey(window, keys.leftShift) == GLFW_PRESS ||
        glfwGetKey(window, keys.rightShift) == GLFW_PRESS
    ) {
        speed_multiplier = 2.f;
    }

    glm::vec3 rotate{0};

    if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
    if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
    if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
    if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

    if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
      gameObject.transform.rotation +=
            speed_multiplier * lookSpeed * dt * glm::normalize(rotate);
    }

    float yaw = gameObject.transform.rotation.y;
    // const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
    // const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
    const glm::vec3 forwardDir{0, 0.f, -1.f};
    const glm::vec3 rightDir{-forwardDir.z, 0.f, forwardDir.x};
    const glm::vec3 upDir{0.f, 1.f, 0.f};

    glm::vec3 moveDir{0.f};

    if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
    if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
    if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
    if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
    // if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
    // if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
        gameObject.apply_force(speed_multiplier * moveSpeed * glm::normalize(moveDir), dt);
    }

    gameObject.update_physics(dt, maze);

    gameObject.transform.update_matrices();
}

bool KeyboardMovementController::moveCamera(
    GLFWwindow* window,
    float dt,
    Camera& camera
) {
    float speed_multiplier = 1.f;
    if (glfwGetKey(window, keys.leftShift) == GLFW_PRESS ||
        glfwGetKey(window, keys.rightShift) == GLFW_PRESS
        ) {
        speed_multiplier = 2.f;
    }
    dt = dt * speed_multiplier;

    bool moved = false;

    glm::vec3 rotate{0};
    if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
    if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
    if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
    if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

    if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
        moved = camera.rotate(rotate.y, rotate.x, dt);
    }

    bool f = false;
    bool b = false;
    bool l = false;
    bool r = false;
    bool u = false;
    bool d = false;

    if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) f = true;
    if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) b = true;
    if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) r = true;
    if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) l = true;
    if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) u = true;
    if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) d = true;

    moved = camera.translate(f, b, l, r, u, d, dt) || moved;

    return moved;
}

bool KeyboardMovementController::moveCameraNoRot(
    GLFWwindow* window,
    float dt,
    Camera& camera
) {
    float speed_multiplier = 1.f;
    if (glfwGetKey(window, keys.leftShift) == GLFW_PRESS ||
        glfwGetKey(window, keys.rightShift) == GLFW_PRESS
        ) {
        speed_multiplier = 2.f;
    }
    dt = dt * speed_multiplier;

    bool f = false;
    bool b = false;
    bool l = false;
    bool r = false;
    bool u = false;
    bool d = false;

    if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) f = true;
    if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) b = true;
    if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) r = true;
    if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) l = true;
    if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) u = true;
    if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) d = true;

    return camera.translate(f, b, l, r, u, d, dt);
}
