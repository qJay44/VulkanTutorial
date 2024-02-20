#include "first_app.hpp"
#include <memory>
#include <stdexcept>

namespace lve {
  FirstApp::FirstApp() {
    createPipelineLayout();
    createPipeline();
    createCommandBuffer();
  }

  FirstApp::~FirstApp() {
    vkDestroyPipelineLayout(lveDevice.device(), pipeLineLayout, nullptr);
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

    if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipeLineLayout) != VK_SUCCESS)
      throw std::runtime_error("failed to create pipeline layout");
  }

  void FirstApp::createPipeline() {
    auto pipelineConfig = LvePipeline::defaultPipelineConfigInfo(lveSwapChain.width(), lveSwapChain.height());
    pipelineConfig.renderPass = lveSwapChain.getRenderPass();
    pipelineConfig.pipelineLayout = pipeLineLayout;
    lvePipeline = std::make_unique<LvePipeline>(
      lveDevice,
      "simple_shader.vert.spv",
      "simple_shader.frag.spv",
      pipelineConfig
    );
  }

  void FirstApp::createCommandBuffer() {}
  void FirstApp::drawFram() {}
}

