#pragma once

#include "VulkanCore.hpp"

namespace lve {

class VulkanApp {
public:
  VulkanApp();
  ~VulkanApp();

  void init(const char* appName);
  void render();

private:
  VulkanCore core;
};

} // namespace lve

