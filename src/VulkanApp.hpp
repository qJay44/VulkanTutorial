#pragma once

#include "VulkanCore.hpp"
#include "VulkanQueue.hpp"

namespace lve {

class VulkanApp {
public:
  VulkanApp() = default;

  void init(const char* appName, GLFWwindow* window);
  void render();
  void destroy();

private:
  VulkanCore m_core;
  VulkanQueue* m_pQueue = nullptr;
  u32 m_numImages = 0;
  std::vector<VkCommandBuffer> m_cmdBufs;

private:
  ~VulkanApp() = default;

  void createCommandBuffers();
  void recordCommandBuffers();
};

} // namespace lve

