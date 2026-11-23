#pragma once

#include "VulkanCore.hpp"
#include "VulkanMesh.hpp"
#include "VulkanQueue.hpp"
#include "VulkanShader.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "Camera.hpp"

namespace lve {

class VulkanApp {
public:
  VulkanApp(const char* appName);
  ~VulkanApp();

  void mouseMoveCallback(vec2 mousePos);

  void render();

private:
  GLFWwindow* m_window = nullptr;
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
  std::vector<VulkanBufferAndMemory> m_uniformBuffers;

  Camera m_camera{};

private:
  void createSimpleRenderPass();
  void createFramebuffers();
  void createShaders();

  // Group
  void createMesh();
  void loadVertexBuffer();
  void loadTexture();

  void createUniformBuffers();
  void createPipeline(GLFWwindow* window);

  void allocateCommandBuffers();
  void recordCommandBuffers(VkExtent2D windowExtent);
  void freeCommandBuffers();

  void updateCameraInput(float dt);
  void updateUniformBuffers(u32 imageIndex);

  void destroyPipeline();
  void destroyUniformBuffers();
  void destroyMesh();
  void destroyShaders();
  void destroyFramebuffers();
  void destroySimpleRenderPass();
};

} // namespace lve

