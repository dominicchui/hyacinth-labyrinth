#pragma once

#include "game/lve_game_object.hpp"
#include "window/glfw-window.hpp"
#include "renderer/camera.h"
#include "game/maze.h"

class KeyboardMovementController {
public:
    struct KeyMappings {
        int moveLeft = GLFW_KEY_A;
        int moveRight = GLFW_KEY_D;
        int moveForward = GLFW_KEY_W;
        int moveBackward = GLFW_KEY_S;
        int moveUp = GLFW_KEY_E;
        int moveDown = GLFW_KEY_Q;
        int lookLeft = GLFW_KEY_LEFT;
        int lookRight = GLFW_KEY_RIGHT;
        int lookUp = GLFW_KEY_UP;
        int lookDown = GLFW_KEY_DOWN;
        int leftShift = GLFW_KEY_LEFT_SHIFT;
        int rightShift = GLFW_KEY_RIGHT_SHIFT;
    };

    void moveInPlaneXZ(GLFWwindow* window,
                       float dt,
                       LveGameObject& gameObject,
                       MazeBlock* maze = nullptr);
    bool moveCamera(GLFWwindow* window, float dt, Camera& camera);
    bool moveCameraNoRot(GLFWwindow* window, float dt, Camera& camera);

    KeyMappings keys{};
    float moveSpeed{20.f};
    float lookSpeed{4.5f};
};
