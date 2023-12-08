#include "lve_game_object.hpp"
#include "utils/utils.h"

void TransformComponent::update_matrices() {
    glm::mat4 tform(1.f);
    glm::mat4 scale_mat = glm::scale(tform, scale);
    glm::mat4 rot_x = glm::rotate(tform, rotation.x, glm::vec3(1.f, 0.f, 0.f));
    glm::mat4 rot_y = glm::rotate(tform, rotation.y, glm::vec3(0.f, 1.f, 0.f));
    glm::mat4 rot_z = glm::rotate(tform, rotation.z, glm::vec3(0.f, 0.f, 1.f));
    glm::mat4 trans_mat = glm::translate(tform, translation);

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

static float calc_drag_modifier(const glm::vec3& velocity) {
    // When close to 0, slow down faster
    float velocity_len = std::abs(glm::length(velocity));

    float drag_modifier = (1.f / velocity_len) + 1.f;

    std::cout << "velocity_len: " << velocity_len << "\t drag_mod: " << drag_modifier << std::endl;

    return drag_modifier;
}

static glm::vec3 drag_function(float drag, const glm::vec3& velocity) {
    //float drag_modifier = -(1.f / (1.f + std::exp(velocity_len + 2.f))) + 1;
    float drag_modifier = calc_drag_modifier(velocity);
    return -drag * drag_modifier * velocity;
}

bool LveGameObject::update_physics(float delta_time) {
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

    // Apply final tranlation
    transform.translation += 0.5f * (actual_prev_velocity + phys.cur_velocity) * delta_time;

    return true;
}
