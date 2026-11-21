#pragma once

#include "VulkanCore.hpp"
#include "VulkanMesh.hpp"
#include "VulkanQueue.hpp"
#include "VulkanShader.hpp"
#include "VulkanGraphicsPipeline.hpp"

namespace lve {

class VulkanApp {
public:
  VulkanApp() = default;
  ~VulkanApp();

  void init(const char* appName, GLFWwindow* window);
  void render();
  void destroy();

private:
  GLFWwindow* m_pWindow = nullptr;
  VulkanCore m_core;
  VulkanQueue* m_pQueue = nullptr;
  VkDevice m_device = nullptr;
  u32 m_numImages = 0;
  VkRenderPass m_renderPass;
  std::vector<VkFramebuffer> m_framebuffers;
  VulkanShader m_vs;
  VulkanShader m_fs;
  VulkanGraphicsPipeline m_pipeline;
  std::vector<VkCommandBuffer> m_cmdBufs;
  VulkanMesh m_mesh;

private:
  void createSimpleRenderPass();
  void createFramebuffers();
  void createShaders();
  void createVertexBuffer();
  void createPipeline(GLFWwindow* window);

  void allocateCommandBuffers();
  void recordCommandBuffers(VkExtent2D windowExtent);
  void freeCommandBuffers();

  void destroyPipeline();
  void destroyVertexBuffer();
  void destroyShaders();
  void destroyFramebuffers();
  void destroySimpleRenderPass();
};

} // namespace lve

