#pragma once

#include "lve_model.hpp"

namespace lve {

struct TransformComponent {
  vec3 translation{};
  vec3 scale{1.f};
  vec3 rotation{};

  mat4 model() const;
  mat3 normalMatrix() const;
};

struct PointLightComponent {
  float lightIntensity = 1.f;
};

class LveGameObject {
public:
  std::shared_ptr<LveModel> model{};
  vec3 color{};
  TransformComponent transform{};
  std::unique_ptr<PointLightComponent> pointLight = nullptr;

public:
  using id_t = unsigned int;
  using Map = std::unordered_map<id_t, LveGameObject>;

  static LveGameObject createGameObject();
  static LveGameObject makePointLight(float intensity = 10.f, float radius = 0.1f, vec3 color = vec3(1.f));

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

