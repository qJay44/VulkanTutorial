#pragma once

#include "models.hpp"
#include "../../lve_game_object.hpp"

namespace lve::bonus::gvf {

inline std::vector<LveGameObject> createDefaultPhysicsObjects(LveDevice& lveDevice) {
  std::vector<LveGameObject> physicsObjects{};
  std::shared_ptr<LveModel> circleModel = createCircleModel(lveDevice, 64);

  auto red = LveGameObject::createGameObject();
  red.transform2d.scale = vec2(0.05f);
  red.transform2d.translation = vec2(0.5f);
  red.color = {1.f, 0.f, 0.f};
  red.rigidBody2d.velocity = {-0.5f, 0.f};
  red.model = circleModel;

  auto blue = LveGameObject::createGameObject();
  blue.transform2d.scale = vec2(0.05f);
  blue.transform2d.translation = {-0.45f, -0.25f};
  blue.color = {0.f, 0.f, 1.f};
  blue.rigidBody2d.velocity = {0.5f, 0.f};
  blue.model = circleModel;

  physicsObjects.push_back(std::move(red));
  physicsObjects.push_back(std::move(blue));

  return physicsObjects;
}

inline std::vector<LveGameObject> createDefaultVectorField(LveDevice& lveDevice, int gridCount = 40) {
  std::vector<LveGameObject> vectorField{};
  std::shared_ptr<LveModel> squareModel = createSquareModel(lveDevice, {0.5f, 0.f});

  for (int i = 0; i < gridCount; i++) {
    for (int j = 0; j < gridCount; j++) {
      auto vf = LveGameObject::createGameObject();
      vf.transform2d.scale = vec2(0.005f);
      vf.transform2d.translation = (vec2{i, j} + 0.5f) / (float)gridCount * 2.f - 1.f;
      vf.color = vec3(0.5f);
      vf.model = squareModel;
      vectorField.push_back(std::move(vf));
    }
  }

  return vectorField;
}

}; // namespace lve::bonus::gvf

