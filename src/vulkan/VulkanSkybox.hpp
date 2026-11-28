#pragma once

#include "VulkanCore.hpp"
#include "VulkanShader.hpp"
#include "VulkanGraphicsPipeline2.hpp"

namespace lve {

class VulkanSkybox {
public:
  VulkanSkybox() = default;
  ~VulkanSkybox() = default;

  void init(VulkanCore* core, const fspath& path);
  void recordCommandBuffer(VkCommandBuffer cmdBuf, u32 imageIndex);
  void update(u32 imageIndex, const mat4& transform);

  void destroy();

private:
  VulkanCore* m_core = nullptr;
  VulkanTexture* m_texCubemap = nullptr;
  u32 m_numImages = 0;
  std::vector<VulkanBufferAndMemory> m_uniformBuffers;
  std::vector<std::vector<VkDescriptorSet>> m_descriptorSets;
  VulkanShader m_vs;
  VulkanShader m_fs;
  VulkanGraphicsPipeline2* m_pipeline = nullptr;

private:
  void createDescriptorSets();
};

} // namespace lve

