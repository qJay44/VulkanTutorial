#pragma once

#include "lve_camera.hpp"
#include "lve_game_object.hpp"

namespace lve {

#define LVE_MAX_LIGHTS 10

struct PointLight {
  vec4 position; // ignore w
  vec4 color; // w - intensity
};

struct GlobalUbo {
  mat4 proj{1.f};
  mat4 view{1.f};
  vec4 lightAmbientColor{1.f, 1.f, 1.f, 0.02f}; // w - intensity
  PointLight pointLights[LVE_MAX_LIGHTS];
  int numLights;
};

struct FrameInfo {
  int frameIndex;
  float frameTime;
  VkCommandBuffer commandBuffer;
  LveCamera& camera;
  VkDescriptorSet globalDescriptorSet;
  LveGameObject::Map& gameObjects;
};

} // namespace lve

