#pragma once

#include "VulkanCore.hpp"
#include "VulkanQueue.hpp"
#include "VulkanShader.hpp"

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
  VkRenderPass m_renderPass;
  std::vector<VkFramebuffer> m_framebuffers;
  VulkanShader m_vs;
  VulkanShader m_fs;

private:
  ~VulkanApp() = default;

  void createCommandBuffers();
  void recordCommandBuffers(VkExtent2D windowExtent);
  void createShaders();
};

} // namespace lve

