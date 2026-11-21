#pragma once

#include "VulkanMesh.hpp"

namespace lve {

class VulkanGraphicsPipeline {
public:
  VulkanGraphicsPipeline() = default;
  ~VulkanGraphicsPipeline();

  void init(
    VkDevice device,
    GLFWwindow* window,
    VkRenderPass renderPass,
    VkShaderModule vs,
    VkShaderModule fs,
    const VulkanMesh* mesh,
    u32 numImages
  );

  void bind(VkCommandBuffer cmdBuf, u32 imageIndex);
  void destroy();

private:
  VkDevice m_device = nullptr;
  VkPipeline m_pipeline = nullptr;
  VkPipelineLayout m_pipelineLayout = nullptr;
  VkDescriptorPool m_descriptorPool;
  VkDescriptorSetLayout m_descriptorSetLayout;
  std::vector<VkDescriptorSet> m_descriptorSets;

private:
  void createDescriptorSets(const VulkanMesh* mesh, u32 numImages);
  void createDescriptorPool(u32 numImages);
  void createDescriptorSetLayout();
  void allocateDescriptorSets(u32 numImages);
  void updateDescriptorSets(const VulkanMesh* mesh, u32 numImages);
};

} // namespace lve

