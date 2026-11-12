#include "simple_render_system.hpp"

namespace lve {

struct SimplePushConstantData {
  mat2 transform{1.f};
  vec2 offset;
  alignas(16) vec3 color;
};
static_assert(sizeof(SimplePushConstantData) % 16 == 0);

SimpleRenderSystem::SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass) : lveDevice(device) {
  createPipelineLayout();
  createPipeline(renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem() {
  vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
}

void SimpleRenderSystem::createPipelineLayout() {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(SimplePushConstantData);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

  if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    error("[SimpleRenderSystem::createPipelineLayout] Failed to create pipeline layout");
}

void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
  assert(pipelineLayout);

  PipelineConfigInfo pipelineConfigInfo{};
  LvePipeline::defaultPipelineConfigInfo(pipelineConfigInfo);

  pipelineConfigInfo.renderPass = renderPass;
  pipelineConfigInfo.pipelineLayout = pipelineLayout;

  lvePipeline = std::make_unique<LvePipeline>(
    lveDevice,
    pipelineConfigInfo,
    "simple_shader.vert",
    "simple_shader.frag"
  );
}

void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<LveGameObject>& gameObjects) {
  lvePipeline->bind(commandBuffer);

  int i = 0;
  for (auto& obj : gameObjects) {
    i++;
    obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.000015f * i, glm::two_pi<float>());

    SimplePushConstantData push{};
    push.offset = obj.transform2d.translation;
    push.color = obj.color;
    push.transform = obj.transform2d.model();

    vkCmdPushConstants(
        commandBuffer,
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(SimplePushConstantData),
        &push
    );

    obj.model->bind(commandBuffer);
    obj.model->draw(commandBuffer);
  }
}

} // namespace lve

