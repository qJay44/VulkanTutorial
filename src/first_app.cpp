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
    recreateSwapChain();
    createCommandBuffers();
  }

  FirstApp::~FirstApp() {
    vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
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

    if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
      throw std::runtime_error("failed to create pipeline layout");
  }

  void FirstApp::recreateSwapChain() {
    auto extent = lveWindow.getExtent();
    while (extent.width == 0 || extent.height == 0) {
      extent = lveWindow.getExtent();
      glfwWaitEvents();
    }

    vkDeviceWaitIdle(lveDevice.device());

    if (lveSwapChain)
      lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);
    else {
      lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent, std::move(lveSwapChain));
      if (lveSwapChain->imageCount() != commandBuffers.size()) {
        freeCommandBuffers();
        createCommandBuffers();
      }
    }

    // if render pass compatible do nothing else
    createPipeline();
  }

  void FirstApp::createPipeline() {
    assert(lveSwapChain && "Cannot create pipeline before swap chain");
    assert(pipelineLayout && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = lveSwapChain->getRenderPass();
    pipelineConfig.pipelineLayout = pipelineLayout;
    lvePipeline = std::make_unique<LvePipeline>(
      lveDevice,
      "simple_shader.vert.spv",
      "simple_shader.frag.spv",
      pipelineConfig
    );
  }

  void FirstApp::createCommandBuffers() {
    commandBuffers.resize(lveSwapChain->imageCount());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = lveDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
      throw std::runtime_error("failed to allocate command buffers");
  }

  void FirstApp::freeCommandBuffers() {
    vkFreeCommandBuffers(
      lveDevice.device(),
      lveDevice.getCommandPool(),
      static_cast<uint32_t>(commandBuffers.size()),
      commandBuffers.data()
    );
    commandBuffers.clear();
  }

  void FirstApp::recordCommandBuffer(int i) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
      throw std::runtime_error("failed to begin recording command buffer");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = lveSwapChain->getRenderPass();
    renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(i);

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.f};
    clearValues[1].depthStencil = {1.f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    VkRect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
    vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

    lvePipeline->bind(commandBuffers[i]);
    lveModel->bind(commandBuffers[i]);
    lveModel->draw(commandBuffers[i]);

    vkCmdEndRenderPass(commandBuffers[i]);
    if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
      throw std::runtime_error("failed to record command buffer");
  }

  void FirstApp::drawFrame() {
    uint32_t imageIndex;
    auto result = lveSwapChain->acquireNextImage(&imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      recreateSwapChain();
      return;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
      throw std::runtime_error("failed to acquire swap chain image");

    recordCommandBuffer(imageIndex);
    result = lveSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()) {
      lveWindow.resetWindowResizedFlag();
      recreateSwapChain();
      return;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
      throw std::runtime_error("failed to present swap chain image");
  }
}

