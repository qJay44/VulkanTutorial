#pragma once

#include "lve_model.hpp"

namespace lve {

struct Transform2dComponent {
  vec2 translation{};
  vec2 scale{1.f};
  float rotation{};

  mat2 model() const;
};

class LveGameObject {
public:
  std::shared_ptr<LveModel> model{};
  vec3 color{};
  Transform2dComponent transform2d{};

public:
  using id_t = unsigned int;

  static LveGameObject createGameObject();

  id_t getId() const;

  LveGameObject(const LveGameObject&) = delete;
  LveGameObject& operator=(const LveGameObject&) = delete;
  LveGameObject(LveGameObject&&) = default;
  LveGameObject& operator=(LveGameObject&&) = default;

private:
  id_t id;

private:
  LveGameObject(id_t objId);
};

} // namespace lve

