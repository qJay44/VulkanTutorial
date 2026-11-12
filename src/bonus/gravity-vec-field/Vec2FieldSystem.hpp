#pragma once

#include "GravityPhysicsSystem.hpp"

namespace lve::bonus::gvf {

class Vec2FieldSystem {
public:
  static void update(
      const GravityPhysicsSystem& physicsSystem,
      std::vector<LveGameObject>& physicsObjs,
      std::vector<LveGameObject>& vectorField
  ) {
    for (auto& vf : vectorField) {
      vec2 dir{};

      for (auto& obj : physicsObjs)
        dir += physicsSystem.computeForce(obj, vf);

      vf.transform2d.scale.x = 0.005f + 0.045f * glm::clamp(glm::log(glm::length(dir) + 1) / 3.f, 0.f, 1.f);
      vf.transform2d.rotation = atan2(dir.y, dir.x);
    }
  }
};

}; // namespace lve::bonus::gvf

