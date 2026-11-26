#pragma once

#include "VulkanCore.hpp"
#include "VulkanModel.hpp"
#include "VulkanQueue.hpp"
#include "VulkanShader.hpp"
#include "VulkanGraphicsPipeline2.hpp"
#include "../Camera.hpp"
#include "gui/ImGuiRenderer.hpp"

namespace lve {

class VulkanApp {
public:
  VulkanApp(const char* appName);
  ~VulkanApp();

  void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  void mouseMoveCallback(vec2 mousePos);

  void render();

private:
  friend class ImGuiRenderer;

  GLFWwindow* m_window = nullptr;
  VulkanCore m_core;
  VulkanQueue* m_pQueue = nullptr;
  VkDevice m_device = nullptr;
  u32 m_numImages = 0;
  VulkanShader m_vs;
  VulkanShader m_fs;
  VulkanGraphicsPipeline2* m_pipeline = nullptr;
  struct {
    std::vector<VkCommandBuffer> withGui;
    std::vector<VkCommandBuffer> withoutGui;
  } m_cmdBufs;
  VulkanModel m_model;
  std::vector<VulkanBufferAndMemory> m_uniformBuffers;

  ImGuiRenderer m_gui;
  vec3 m_clearColor{0.01f};
  bool m_showGui = true;

  VulkanTexture* m_texDebug = nullptr;

  Camera m_camera{};

private:
  void createShaders();

  // Group
  void createModel();
  void loadVertexBuffer();
  void loadTexture();

  void createPipeline(GLFWwindow* window);

  void allocateCommandBuffers();
  void recordCommandBuffersGeneral();
  void recordCommandBuffers(bool withSecondBarrier, std::vector<VkCommandBuffer>& cmdBufs);
  void freeCommandBuffers();

  void updateCameraInput(float dt);
  void updateUniformBuffers(u32 imageIndex);

  void beginRendering(VkCommandBuffer cmdBuf, u32 imageIndex);
  void endRendering(VkCommandBuffer cmdBuf);

  void destroyPipeline();
  void destroyModel();
  void destroyShaders();
};

} // namespace lve

