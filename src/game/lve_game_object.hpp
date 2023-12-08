#pragma once

#include "vulkan/vulkan-model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

struct TransformComponent {
  glm::vec3 translation{};
  glm::vec3 scale{1.f, 1.f, 1.f};
  glm::vec3 rotation{};

  // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
  // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
  // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
  glm::mat4 mat4;
  glm::mat3 normalMatrix;

  void update_matrices();
};

struct PhysicalProperties {
    float mass;
    float drag;
    glm::vec3 cur_velocity;
    glm::vec3 prev_velocity;
};

struct PointLightComponent {
  float lightIntensity = 1.0f;
};

class LveGameObject {
 public:
  using id_t = unsigned int;
  using Map = std::unordered_map<id_t, LveGameObject>;

  static LveGameObject createGameObject() {
    static id_t currentId = 0;
    return LveGameObject{currentId++};
  }

  static LveGameObject makePointLight(
      float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

  LveGameObject(const LveGameObject &) = delete;
  LveGameObject &operator=(const LveGameObject &) = delete;
  LveGameObject(LveGameObject &&) = default;
  LveGameObject &operator=(LveGameObject &&) = default;

  id_t getId() { return id; }

  glm::vec3 color{};
  TransformComponent transform{};
  PhysicalProperties phys{1.f, 0.1f, glm::vec3(0.f), glm::vec3(0.f)};

  // Optional pointer components
  std::shared_ptr<VKModel> model{};
  std::unique_ptr<PointLightComponent> pointLight = nullptr;

  bool apply_force(glm::vec3 force, float delta_time);
  bool update_physics(float delta_time);

 private:
  LveGameObject(id_t objId) : id{objId} {}

  id_t id;
};
