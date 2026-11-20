#pragma once

#include "VulkanCore.hpp"
#include "VulkanQueue.hpp"
#include "VulkanShader.hpp"
#include "VulkanGraphicsPipeline.hpp"

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
  VkDevice m_device = nullptr;
  u32 m_numImages = 0;
  std::vector<VkCommandBuffer> m_cmdBufs;
  VkRenderPass m_renderPass;
  std::vector<VkFramebuffer> m_framebuffers;
  VulkanShader m_vs;
  VulkanShader m_fs;
  VulkanGraphicsPipeline m_pipeline;

private:
  ~VulkanApp() = default;

  void createCommandBuffers();
  void recordCommandBuffers(VkExtent2D windowExtent);
  void createShaders();
  void createPipeline(GLFWwindow* window);
};

} // namespace lve

