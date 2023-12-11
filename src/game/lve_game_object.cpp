#include "lve_game_object.hpp"

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
