#pragma once

#include "model_desc.hpp"

namespace lve {

class VulkanGraphicsPipeline2 {
public:
  VulkanGraphicsPipeline2(
    VkDevice device,
    GLFWwindow* window,
    VkShaderModule vs,
    VkShaderModule fs,
    u32 numImages,
    VkFormat colorFormat,
    VkFormat depthFormat
  );

  ~VulkanGraphicsPipeline2();

  void bind(VkCommandBuffer cmdBuf);
  void allocateDescriptorSets(u32 numSubmeshes, std::vector<std::vector<VkDescriptorSet>>& descriptorSets);
  void updateDescriptorSets(const ModelDesc& modelDesc, std::vector<std::vector<VkDescriptorSet>>& descriptorSets);

  VkPipelineLayout getPipelineLayout() const;

private:
  VkDevice m_device = VK_NULL_HANDLE;
  u32 m_numImages = 0;

  VkPipeline m_pipeline = VK_NULL_HANDLE;
  VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
  VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
  VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;

private:
  void initCommon(
    GLFWwindow* window,
    VkShaderModule vs,
    VkShaderModule fs,
    VkFormat colorFormat,
    VkFormat depthFormat
  );

  void createDescriptorPool(u32 maxSets);
  void createDescriptorSetLayout(bool isVB, bool isIB, bool isTex, bool isUniform);

  void allocateDescriptorSetsInternal(u32 numSubmeshes, std::vector<std::vector<VkDescriptorSet>>& descriptorSets);
};

} // namespace lve

