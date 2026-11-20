#pragma once

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
    VkShaderModule fs
  );

  void bind(VkCommandBuffer cmdBuf);

private:
  VkDevice m_device = nullptr;
  VkPipeline m_pipeline = nullptr;
  VkPipelineLayout m_pipelineLayout = nullptr;
};

} // namespace lve

