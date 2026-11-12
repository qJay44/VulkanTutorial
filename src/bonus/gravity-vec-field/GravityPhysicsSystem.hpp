#pragma once

#include "../../lve_game_object.hpp"

namespace lve::bonus::gvf {

class GravityPhysicsSystem {
public:
  GravityPhysicsSystem(float strength) : strength(strength) {};

  void update(std::vector<LveGameObject>& objs, float dt, int substeps = 1) {
    float dtSub = dt / substeps;
    for (int i = 0; i < substeps; i++) {
      stepSimulation(objs, dtSub);
    }
  }

  vec2 computeForce(LveGameObject& fromObj, LveGameObject& toObj) const {
    vec2 offset = fromObj.transform2d.translation - toObj.transform2d.translation;
    float distSq = glm::dot(offset, offset);

    if (distSq < 1e-6f)
      return {0.f, 0.f};

    float force = strength * toObj.rigidBody2d.mass * fromObj.rigidBody2d.mass / distSq;
    return force * offset / glm::sqrt(distSq);
  }

private:
  float strength;

private:
  void stepSimulation(std::vector<LveGameObject>& objs, float dt) {
    for (size_t i = 0; i < objs.size(); i++) {
      for (size_t j = i + 1; j < objs.size(); j++) {
        auto& obj1 = objs[i];
        auto& obj2 = objs[j];

        vec2 force = computeForce(obj1, obj2);
        obj1.rigidBody2d.velocity += -force / obj1.rigidBody2d.mass * dt;
        obj2.rigidBody2d.velocity +=  force / obj2.rigidBody2d.mass * dt;
      }
    }

    for (auto& obj : objs)
      obj.transform2d.translation += obj.rigidBody2d.velocity * dt;
  }
};

}; // namespace lve::bonus::gvf

