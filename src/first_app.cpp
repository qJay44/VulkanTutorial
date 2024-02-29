#include "first_app.hpp"

// std
#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>

namespace lve {

  // https://pastebin.com/0bu2a2ZP
  void sierpinski(
    std::vector<LveModel::Vertex>& vertices,
    signed depth,
    glm::vec2 left,
    glm::vec2 right,
    glm::vec2 top
  ) {
    if (depth == 0) {
      vertices.push_back({{top}  , {1.f, 0.f, 0.f}});
      vertices.push_back({{right}, {0.f, 1.f, 0.f}});
      vertices.push_back({{left} , {0.f, 0.f, 1.f}});
    } else {
      auto leftTop = 0.5f * (left + top);
      auto rightTop = 0.5f * (right + top);
      auto leftRight = 0.5f * (left + right);
      sierpinski(vertices, depth - 1, left, leftRight, leftTop);
      sierpinski(vertices, depth - 1, leftRight, right, rightTop);
      sierpinski(vertices, depth - 1, leftTop, rightTop, top);
    }
  }

  FirstApp::FirstApp() {
    loadModels();
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
      drawFrame();
    }

    vkDeviceWaitIdle(lveDevice.device());
  }

  void FirstApp::loadModels() {
    /* std::vector<LveModel::Vertex> vertices; */
    /* sierpinski(vertices, 5, {0.f, -0.5f}, {0.5f, 0.5f}, {-0.5f, 0.5f}); */

    std::vector<LveModel::Vertex> vertices {
      {{0.f, -0.5f} , {1.f, 0.f, 0.f}},
      {{0.5f, 0.5f} , {0.f, 1.f, 0.f}},
      {{-0.5f, 0.5f}, {0.f, 0.f, 1.f}},
    };
    lveModel = std::make_unique<LveModel>(lveDevice, vertices);
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

  void FirstApp::createCommandBuffer() {
    commandBuffers.resize(lveSwapChain.imageCount());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = lveDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
      throw std::runtime_error("failed to allocate command buffers");

    for (int i = 0; i < commandBuffers.size(); i++) {
      VkCommandBufferBeginInfo beginInfo{};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

      if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin recording command buffer");

      VkRenderPassBeginInfo renderPassInfo{};
      renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      renderPassInfo.renderPass = lveSwapChain.getRenderPass();
      renderPassInfo.framebuffer = lveSwapChain.getFrameBuffer(i);

      renderPassInfo.renderArea.offset = {0, 0};
      renderPassInfo.renderArea.extent = lveSwapChain.getSwapChainExtent();

      std::array<VkClearValue, 2> clearValues{};
      clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.f};
      clearValues[1].depthStencil = {1.f, 0};
      renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
      renderPassInfo.pClearValues = clearValues.data();

      vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

      lvePipeline->bind(commandBuffers[i]);
      lveModel->bind(commandBuffers[i]);
      lveModel->draw(commandBuffers[i]);

      vkCmdEndRenderPass(commandBuffers[i]);
      if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer");
    }
  }

  void FirstApp::drawFrame() {
    uint32_t imageIndex;
    auto result = lveSwapChain.acquireNextImage(&imageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
      throw std::runtime_error("failed to acquire swap chain image");

    result = lveSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
      throw std::runtime_error("failed to present swap chain image");
  }
}

