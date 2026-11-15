#include "point_light_system.hpp"

namespace lve {

struct PointLightConstants {
  vec4 position;
  vec4 color;
  float radius;
};

PointLightSystem::PointLightSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : lveDevice(device) {
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

PointLightSystem::~PointLightSystem() {
  vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
}

void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(PointLightConstants);

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = static_cast<u32>(descriptorSetLayouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

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

void PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo) {
  auto rotateLight = glm::rotate(
    mat4(1.f),
    frameInfo.frameTime,
    {0.f, -1.f, 0.f}
  );

  int i = 0;

  for (auto& [id, obj] : frameInfo.gameObjects) {
    if (!obj.pointLight)
      continue;

    assert(i < LVE_MAX_LIGHTS);

    obj.transform.translation = vec3(rotateLight * vec4(obj.transform.translation, 1.f));

    ubo.pointLights[i].position = vec4(obj.transform.translation, 1.f);
    ubo.pointLights[i].color = vec4(obj.color, obj.pointLight->lightIntensity);

    i++;
  }

  ubo.numLights = i;
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

  for (auto& [id, obj] : frameInfo.gameObjects) {
    if (!obj.pointLight)
      continue;

    PointLightConstants push{};
    push.position = vec4(obj.transform.translation, 1.f);
    push.color = vec4(obj.color, obj.pointLight->lightIntensity);
    push.radius = obj.transform.scale.x;

    vkCmdPushConstants(
      frameInfo.commandBuffer,
      pipelineLayout,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      0,
      sizeof(PointLightConstants),
      &push
    );

    vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
  }
}

} // namespace lve

