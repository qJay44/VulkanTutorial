#include "first_app.hpp"
#include "utils/utils.hpp"

namespace lve {

FirstApp::FirstApp() {
  createPipelineLayout();
  createPipeline();
  createCommandBuffers();
}

FirstApp::~FirstApp() {
  vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
}

void FirstApp::run() {
  while (!lveWindow.shouldClose()) {
    glfwPollEvents();
  }
}

void FirstApp::createPipelineLayout() {
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges = nullptr;

  if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    error("[FirstApp::createPipelineLayout] Failed to create pipeline layout");
}

void FirstApp::createPipeline() {
  PipelineConfigInfo pipelineConfigInfo{};
  LvePipeline::defaultPipelineConfigInfo(pipelineConfigInfo, lveSwapChain.width(), lveSwapChain.height());

  pipelineConfigInfo.renderPass = lveSwapChain.getRenderPass();
  pipelineConfigInfo.pipelineLayout = pipelineLayout;

  lvePipeline = std::make_unique<LvePipeline>(
    lveDevice,
    pipelineConfigInfo,
    "simple_shader.vert",
    "simple_shader.frag"
  );
}

void FirstApp::createCommandBuffers() {}
void FirstApp::drawFrame() {}

} // namespace lve

