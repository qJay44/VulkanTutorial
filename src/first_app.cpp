#include "first_app.hpp"
#include "utils/utils.hpp"

namespace lve {

struct SimplePushConstantData {
  mat2 transform{1.f};
  vec2 offset;
  alignas(16) vec3 color;
};
static_assert(sizeof(SimplePushConstantData) % 16 == 0);

FirstApp::FirstApp() {
  loadGameObjects();
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

void FirstApp::loadGameObjects() {
  std::vector<LveModel::Vertex> vertices{
    {{ 0.0f, -0.5f}, {1.f, 0.f, 0.f,}},
    {{ 0.5f,  0.5f}, {0.f, 1.f, 0.f,}},
    {{-0.5f,  0.5f}, {0.f, 0.f, 1.f,}},
  };

  auto lveModel = std::make_shared<LveModel>(lveDevice, vertices);

  int cnt = 12;
  float step = 1.f / cnt;
  for (int i = 0; i < cnt; i++) {
    auto triangle = LveGameObject::createGameObject();
    int c = i % 3;
    triangle.model = lveModel;
    triangle.color = vec3{0.8f};
    triangle.color[c] *= 0.f;
    triangle.transform2d.translation = vec2{0.f};
    triangle.transform2d.rotation = 0.25f * glm::two_pi<float>() * i * step;
    triangle.transform2d.scale = vec2{1.f} * (i * step);


    gameObjects.push_back(std::move(triangle));
  }
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

void FirstApp::recordCommandBuffer(int imageIndex) {
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

  renderGameObjects(commandBuffers[imageIndex]);

  vkCmdEndRenderPass(commandBuffers[imageIndex]);
  if (vkEndCommandBuffer(commandBuffers[imageIndex]))
    error("[FirstApp::createCommandBuffers] Failed to end recording command buffer");
}

void FirstApp::renderGameObjects(VkCommandBuffer commandBuffer) {
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

} // namespace lve

