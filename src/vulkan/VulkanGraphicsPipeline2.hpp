#pragma once

#include "model_desc.hpp"

namespace lve {

class VulkanGraphicsPipeline2 {
public:
  struct PipelineDescriptor {
    VkDevice device = VK_NULL_HANDLE;
    GLFWwindow* window = nullptr;
    VkShaderModule vs = VK_NULL_HANDLE;
    VkShaderModule fs = VK_NULL_HANDLE;
    u32 numImages = 0;
    VkFormat colorFormat = VK_FORMAT_UNDEFINED;
    VkFormat depthFormat = VK_FORMAT_UNDEFINED;
    VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;
    bool isVB = false;
    bool isIB = false;
    bool isUniform = false;
    bool isTex2D = false;
    bool isTexCubemap = false;
  };

  enum Binding {
    Binding_VB,
    Binding_IB,
    Binding_Uniform,
    Binding_Texture2D,
    Binding_TextureCubemap,
    Binding_BindingCount
  };

  VulkanGraphicsPipeline2(const PipelineDescriptor& desc);

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
  std::vector<VkDescriptorPoolSize> m_descriptorPoolSizes;
  VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
  VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;

private:
  void initCommon(const PipelineDescriptor& desc);

  void createDescriptorPool(u32 maxSets);
  void createDescriptorSetLayout(const PipelineDescriptor& desc);

  void allocateDescriptorSetsInternal(u32 numSubmeshes, std::vector<std::vector<VkDescriptorSet>>& descriptorSets);
};

} // namespace lve

