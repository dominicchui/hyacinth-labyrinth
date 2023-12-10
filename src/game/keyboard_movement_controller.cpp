#include "keyboard_movement_controller.hpp"
#include "renderer/camera.h"
#include "game/maze.h"
// std
#include <limits>
#include <vector>

static bool collide(const LveGameObject& wall, const LveGameObject& ball) {
    float radius = 1.f; // TODO
    const glm::mat4& ctm = wall.transform.mat4;

    float ball_rad = ball.phys.radius;
    glm::vec3 ball_center = ball.transform.mat4 * zero_pt;

    glm::vec3 Bmin = ctm * glm::vec4(-radius, -radius, -radius, 1);
    glm::vec3 Bmax = ctm * glm::vec4(radius, radius, radius, 1);
    float r2 = ball_rad * ball_rad;
    float dmin = 0;
    for( int i = 0; i < 3; i++ ) {
        if (ball_center[i] < Bmin[i] ) {
            dmin += pow2(ball_center[i] - Bmin[i]);
        }
        else if (ball_center[i] > Bmax[i]) {
            dmin += pow2(ball_center[i] - Bmax[i]);
        }
    }
    return dmin <= r2;
}

void KeyboardMovementController::collisionHandler(
    LveGameObject& ball,
    float dt,
    MazeBlock& maze
) {
    glm::vec4 cur_pos = ball.transform.mat4 * glm::vec4(0, 0, 0, 1.f);

    float cur_w = cur_pos.x;
    float cur_h = cur_pos.z;

    // We can only intersect with up to 3 walls at any given time
    // Sort those cases out:
    float cur_w_flr = std::floor(cur_w);
    float cur_h_flr = std::floor(cur_h);

    float w_diff = cur_w - cur_w_flr;
    float h_diff = cur_h - cur_h_flr;

    std::vector<int32_t> w_indices;
    std::vector<int32_t> h_indices;

    // TODO: Tighten... (we really only should be checking 3 walls)
    // if (w_diff < (0.5f - epsilon)) {
    //     // Only check the top side
    //     w_indices.push_back(cur_w_flr);
    // } else if (w_diff > (0.5f + epsilon)) {
    //     w_indices.push_back(cur_w_flr+1.f);
    // } else {
        w_indices.push_back(cur_w_flr - 1.f);
        w_indices.push_back(cur_w_flr);
        w_indices.push_back(cur_w_flr + 1.f);
        w_indices.push_back(cur_w_flr + 2.f);

    //}

    // if (h_diff < (0.5f - epsilon)) {
    //     // Only check the top side
    //     h_indices.push_back(cur_h_flr);
    // } else if (h_diff > (0.5f + epsilon)) {
    //     h_indices.push_back(cur_h_flr+1.f);
    // } else {
        h_indices.push_back(cur_h_flr - 1.f);
        h_indices.push_back(cur_h_flr);
        h_indices.push_back(cur_h_flr + 1.f);
        h_indices.push_back(cur_h_flr + 2.f);

        //}

    // Search through the set of walls we could possibly collide with
    for (int32_t h : h_indices) {
        for (int32_t w : w_indices) {
            std::pair<int32_t, int32_t> coord = maze.world_coords_to_indices(w, h);

            // TODO: do better
            if (coord.first < 0 || coord.second < 0) {
                continue;
            }

            int32_t wall_idx = maze.spatial_map[coord.second][coord.first];
            if (wall_idx >= 0) {
                const LveGameObject& wall = maze.wall_blocks.at(wall_idx);
                std::cout << "added wall at: " << coord.first << ", " << coord.second << std::endl;

                bool intersects = collide(wall, ball);

                if (intersects) {
                    bool intersects_again = collide(wall, ball);
                    std::cout << "Hit wall: " << coord.first << ", " << coord.second << intersects_again <<std::endl;
                }
            }
        }
    }
}

void KeyboardMovementController::moveInPlaneXZ(
    GLFWwindow* window,
    float dt,
    LveGameObject& gameObject,
    MazeBlock* maze
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
    if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
    if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
        gameObject.apply_force(speed_multiplier * moveSpeed * glm::normalize(moveDir), dt);
    }

    gameObject.update_physics(dt);

    // Perform a round of collision detection
    if (maze) {
        collisionHandler(gameObject, dt, *maze);
    }

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

    // if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) f = true;
    // if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) b = true;
    // if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) r = true;
    // if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) l = true;
    if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) u = true;
    if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) d = true;

    return camera.translate(f, b, l, r, u, d, dt);
}
