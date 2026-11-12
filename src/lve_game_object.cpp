#include "lve_game_object.hpp"

namespace lve {

mat2 Transform2dComponent::model() const {
  const float s = glm::sin(rotation);
  const float c = glm::cos(rotation);

  mat2 matRotation{{c, s}, {-s, c}};
  mat2 matScale{{scale.x, 0.f}, {0.f, scale.y}};

  return matRotation * matScale;
}

LveGameObject LveGameObject::createGameObject() {
  static id_t currentId = 0;
  return LveGameObject(currentId++);
}

id_t LveGameObject::getId() const {
  return id;
}

LveGameObject::LveGameObject(id_t objId) : id(objId) {
}

}; // namespace lve

