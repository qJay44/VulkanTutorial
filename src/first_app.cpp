#include "first_app.hpp"
#include "utils/utils.hpp"

namespace lve {

struct SimplePushConstantData {
  vec2 offset;
  alignas(16) vec3 color;
};
static_assert(sizeof(SimplePushConstantData) % 16 == 0);

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
  std::vector<LveModel::Vertex> vertices{
    {{ 0.0f, -0.5f}, {1.f, 0.f, 0.f,}},
    {{ 0.5f,  0.5f}, {0.f, 1.f, 0.f,}},
    {{-0.5f,  0.5f}, {0.f, 0.f, 1.f,}},
  };

  lveModel = std::make_unique<LveModel>(lveDevice, vertices);
}

void FirstApp::createPipelineLayout() {
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
    error("[FirstApp::createPipelineLayout] Failed to create pipeline layout");
}

void FirstApp::createPipeline() {
  assert(lveSwapChain);
  assert(pipelineLayout);

  PipelineConfigInfo pipelineConfigInfo{};
  LvePipeline::defaultPipelineConfigInfo(pipelineConfigInfo);

  pipelineConfigInfo.renderPass = lveSwapChain->getRenderPass();
  pipelineConfigInfo.pipelineLayout = pipelineLayout;

  lvePipeline = std::make_unique<LvePipeline>(
    lveDevice,
    pipelineConfigInfo,
    "simple_shader.vert",
    "simple_shader.frag"
  );
}

void FirstApp::createCommandBuffers() {
  commandBuffers.resize(lveSwapChain->imageCount());

  VkCommandBufferAllocateInfo allocinfo{};
  allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocinfo.commandPool = lveDevice.getCommandPool();
  allocinfo.commandBufferCount = static_cast<u32>(commandBuffers.size());

  if (vkAllocateCommandBuffers(lveDevice.device(), &allocinfo, commandBuffers.data()) != VK_SUCCESS)
    error("[FirstApp::createCommandBuffers] Failed to allocate command buffers");
}

void FirstApp::freeCommandBuffers() {
  vkFreeCommandBuffers(
      lveDevice.device(),
      lveDevice.getCommandPool(),
      static_cast<u32>(commandBuffers.size()),
      commandBuffers.data());

  commandBuffers.clear();
}

void FirstApp::drawFrame() {
  u32 imageIndex;
  auto result = lveSwapChain->acquireNextImage(&imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    error("[FirstApp::drawFrame] Failed to acquire swap chain image");

  recordCommandBuffer(imageIndex);
  result = lveSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()) {
    lveWindow.resetWindowResizedFlag();
    recreateSwapChain();
    return;
  }

  if (result != VK_SUCCESS)
    error("[FirstApp::drawFrame] Failed to present swap chain image");
}

void FirstApp::recreateSwapChain() {
  auto extent = lveWindow.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = lveWindow.getExtent();
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(lveDevice.device());

  if (lveSwapChain) {
    lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent, std::move(lveSwapChain));
  } else {
    lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);
    if (lveSwapChain->imageCount() != commandBuffers.size()) {
      freeCommandBuffers();
      createCommandBuffers();
    }
  }

  createPipeline(); // If render pass is compatible do not call it
}

void FirstApp::recordCommandBuffer(int imageIndex) {
  static int frame = 0;
  frame = (frame + 1) % 10000;

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
    error("[FirstApp::createCommandBuffers] Failed to begin recording command buffer");

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = lveSwapChain->getRenderPass();
  renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(imageIndex);

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {{0.01f, 0.01f, 0.01f, 1.f}};
  clearValues[1].depthStencil = {1.f, 0};
  renderPassInfo.clearValueCount = static_cast<u32>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  auto swapChainExtent = lveSwapChain->getSwapChainExtent();
  VkViewport viewport{};
  viewport.x = 0.f;
  viewport.y = 0.f;
  viewport.width = static_cast<float>(swapChainExtent.width);
  viewport.height = static_cast<float>(swapChainExtent.height);
  viewport.minDepth = 0.f;
  viewport.maxDepth = 0.f;
  VkRect2D scissor{{0, 0}, swapChainExtent};
  vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
  vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

  lvePipeline->bind(commandBuffers[imageIndex]);
  lveModel->bind(commandBuffers[imageIndex]);

  for (int j = 0; j < 4; j++) {
    SimplePushConstantData push{};
    push.offset = {-0.5f + frame * 0.0002f, -0.4f + j * 0.25f};
    push.color = {0.f, 0.f, 0.2f + 0.2f * j};

    vkCmdPushConstants(
        commandBuffers[imageIndex],
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(SimplePushConstantData),
        &push
    );

    lveModel->draw(commandBuffers[imageIndex]);
  }

  vkCmdEndRenderPass(commandBuffers[imageIndex]);
  if (vkEndCommandBuffer(commandBuffers[imageIndex]))
    error("[FirstApp::createCommandBuffers] Failed to end recording command buffer");
}

} // namespace lve

