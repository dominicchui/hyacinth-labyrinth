#include "debug.h"
#include "lve_game_object.hpp"
#include "utils/utils.h"
#include "game/maze.h"

#include <glm/gtx/transform.hpp>

// void TransformComponent::update_matrices() {
//     glm::mat4 tform(1.f);
//     glm::mat4 scale_mat = glm::scale(tform, scale);
//     glm::mat4 rot_x = glm::rotate(tform, rotation.x, glm::vec3(1.f, 0.f, 0.f));
//     glm::mat4 rot_y = glm::rotate(tform, rotation.y, glm::vec3(0.f, 1.f, 0.f));
//     glm::mat4 rot_z = glm::rotate(tform, rotation.z, glm::vec3(0.f, 0.f, 1.f));
//     glm::mat4 trans_mat = glm::translate(tform, translation);

//     mat4 = trans_mat * rot_z * rot_y * rot_x * scale_mat;
//     normalMatrix = glm::inverse(glm::mat3(tform));
// }

void TransformComponent::update_matrices() {
    glm::mat4 tform(1.f);
    glm::mat4 scale_mat = glm::scale(scale);
    glm::mat4 rot_x = glm::rotate(rotation.x, x_axis);
    glm::mat4 rot_y = glm::rotate(rotation.y, y_axis);
    glm::mat4 rot_z = glm::rotate(rotation.z, z_axis);

    glm::mat4 trans_mat = glm::translate(translation);

    mat4 = trans_mat * rot_z * rot_y * rot_x * scale_mat;
    normalMatrix = glm::inverse(glm::mat3(tform));
}


LveGameObject LveGameObject::makePointLight(float intensity, float radius, glm::vec3 color) {
  LveGameObject gameObj = LveGameObject::createGameObject();
  gameObj.color = color;
  gameObj.transform.scale.x = radius;
  gameObj.pointLight = std::make_unique<PointLightComponent>();
  gameObj.pointLight->lightIntensity = intensity;
  return gameObj;
}

bool LveGameObject::apply_force(glm::vec3 force, float delta_time) {
    if (force == zero_vec3 && phys.cur_velocity == zero_vec3) {
        return false;
    }
    glm::vec3 accel = phys.mass * force;
    glm::vec3 new_velocity = accel * delta_time;

    phys.prev_velocity = phys.cur_velocity;
    phys.cur_velocity += new_velocity;

    return true;
}

struct HitInfo {
    glm::vec3 hit_pos;
    glm::vec3 normal;
};

// Returns whether we intersected with a wall, plus the intersection
// normal if so
static std::pair<bool, glm::vec3> collide(
    const LveGameObject& wall,
    const LveGameObject& ball
    ) {
    float radius = 1.f; // TODO
    const glm::mat4& ctm = wall.transform.mat4;

    float ball_rad = ball.phys.radius;
    glm::vec3 ball_center = ball.transform.mat4 * zero_pt;

    glm::vec3 Bmin = ctm * glm::vec4(-radius, -radius, -radius, 1);
    glm::vec3 Bmax = ctm * glm::vec4(radius, radius, radius, 1);

    float r2 = ball_rad * ball_rad;
    float dmin = 0;
    int32_t dargmin = 0;
    int32_t dargmindir = 0;
    for (int i = 0; i < 3; i++) {
        if (ball_center[i] < Bmin[i] ) {
            dmin += pow2(ball_center[i] - Bmin[i]);
            dargmin = i;
            dargmindir = -1;
        } else if (ball_center[i] > Bmax[i]) {
            dmin += pow2(ball_center[i] - Bmax[i]);
            dargmin = i;
            dargmindir = 1;
        }
    }
    glm::vec3 normal(0.f);
    normal[dargmin] = dargmindir;
    return {dmin < r2, normal};
}

void LveGameObject::collision_handler(MazeBlock& maze) {
    glm::vec4 cur_pos = transform.mat4 * glm::vec4(0, 0, 0, 1.f);

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
                //std::cout << "added wall at: " << coord.first << ", " << coord.second << std::endl;

                auto intersects = collide(wall, *this);

                if (intersects.first) {
                    glm::vec3& normal = intersects.second;
                    std::cout << "Hit wall: " << coord.first << ", " << coord.second;
                    std::cout << " normal: ";
                    printVec3(normal);

                    std::cout << "old old_velocity: ";
                    printVec3(phys.cur_velocity, false);
                    if (glm::dot(phys.cur_velocity, normal) < 0) {
                        phys.prev_velocity = glm::reflect(phys.prev_velocity, normal);
                        phys.cur_velocity = glm::reflect(phys.cur_velocity, normal);
                    }
                    std::cout << "new cur_velocity: ";
                    printVec3(phys.cur_velocity);

                    // Find out how much of the ball got inside the wall, and reflect the position by that amount
                    glm::vec3 center_diff = glm::vec3(cur_pos) - glm::vec3(wall.transform.translation);
                    std::cout << "diff: ";
                    printVec3(center_diff);
                    glm::vec3 bounce_amount = (center_diff - normal) * -glm::abs(normal);
                    std::cout << "bounce amount: ";
                    printVec3(bounce_amount);
                    transform.translation += bounce_amount;
                }
            }
        }
    }
}

static float calc_drag_modifier(const glm::vec3& velocity) {
    // When close to 0, slow down faster
    float velocity_len = std::abs(glm::length(velocity));

    float drag_modifier = (1.f / velocity_len) + 1.f;

    //std::cout << "velocity_len: " << velocity_len << "\t drag_mod: " << drag_modifier << std::endl;

    return drag_modifier;
}

static glm::vec3 drag_function(float drag, const glm::vec3& velocity) {
    //float drag_modifier = -(1.f / (1.f + std::exp(velocity_len + 2.f))) + 1;
    float drag_modifier = calc_drag_modifier(velocity);
    return -drag * drag_modifier * velocity;
}

bool LveGameObject::update_physics(float delta_time, MazeBlock* maze) {
    // Account for drag:
    glm::vec3 actual_prev_velocity = phys.prev_velocity;
    float vel_len = glm::length(phys.cur_velocity);

    if (vel_len <= epsilon) {
        return false;
    } else if (glm::length(phys.cur_velocity) < 0.001f) {
        // If we're below some epsilon, call it 0
        phys.prev_velocity = phys.cur_velocity;
        phys.cur_velocity = glm::vec3(0.f);

    } else {
        apply_force(drag_function(phys.drag, phys.cur_velocity), delta_time);
    }

    // Apply translation
    glm::vec3 halfway_velocity = 0.5f * (actual_prev_velocity + phys.cur_velocity);
    glm::vec3 delta_dist = halfway_velocity * delta_time;
    transform.translation += delta_dist;

    // Modify translation if collision happened
    if (maze != nullptr) {
        collision_handler(*maze);
    }

    // Apply rotation
    float delta_dist_xz = glm::length(glm::vec2(-delta_dist.z, delta_dist.x));

    transform.z_axis += glm::normalize(glm::vec3(
                            halfway_velocity.x,
                            0.f,
                            halfway_velocity.z)
                        ) * 0.1f;
    transform.z_axis = glm::normalize(transform.z_axis);
    transform.x_axis = glm::cross(transform.z_axis, transform.y_axis);
    transform.rotation += glm::vec3(delta_dist_xz, 0.f, 0.f);

    return true;
}
