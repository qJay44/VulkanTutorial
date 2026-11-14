#pragma once

#include "lve_camera.hpp"

namespace lve {

struct FrameInfo {
  int frameIndex;
  float frameTime;
  VkCommandBuffer commandBuffer;
  LveCamera& camera;
};

} // namespace lve

