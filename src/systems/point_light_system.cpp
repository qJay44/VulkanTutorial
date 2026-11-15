#include "point_light_system.hpp"

namespace lve {

PointLightSystem::PointLightSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : lveDevice(device) {
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

PointLightSystem::~PointLightSystem() {
  vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
}

void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
  // VkPushConstantRange pushConstantRange{};
  // pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  // pushConstantRange.offset = 0;
  // pushConstantRange.size = sizeof(SimplePushConstantData);

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = static_cast<u32>(descriptorSetLayouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges = nullptr;

  if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    error("[PointLightSystem::createPipelineLayout] Failed to create pipeline layout");
}

void PointLightSystem::createPipeline(VkRenderPass renderPass) {
  assert(pipelineLayout);

  PipelineConfigInfo pipelineConfigInfo{};
  LvePipeline::defaultPipelineConfigInfo(pipelineConfigInfo);
  pipelineConfigInfo.bindingDescriptions.clear();
  pipelineConfigInfo.bindingAttributes.clear();
  pipelineConfigInfo.renderPass = renderPass;
  pipelineConfigInfo.pipelineLayout = pipelineLayout;

  lvePipeline = std::make_unique<LvePipeline>(
    lveDevice,
    pipelineConfigInfo,
    "point_light.vert",
    "point_light.frag"
  );
}

void PointLightSystem::render(FrameInfo& frameInfo) {
  lvePipeline->bind(frameInfo.commandBuffer);

  vkCmdBindDescriptorSets(
    frameInfo.commandBuffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipelineLayout,
    0,
    1,
    &frameInfo.globalDescriptorSet,
    0,
    nullptr
  );

  vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
}

} // namespace lve

