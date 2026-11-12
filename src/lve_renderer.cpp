#include "lve_renderer.hpp"

namespace lve {

LveRenderer::LveRenderer(LveWindow& lveWindow, LveDevice& lveDevice) : lveWindow(lveWindow), lveDevice(lveDevice) {
  recreateSwapChain();
  createCommandBuffers();
}

LveRenderer::~LveRenderer() {
  freeCommandBuffers();
}

VkRenderPass LveRenderer::getSwapChainRenderPass() const {
  return lveSwapChain->getRenderPass();
}

float LveRenderer::getAspectRatio() const {
  return lveSwapChain->extentAspectRatio();
}

bool LveRenderer::isFrameInProgress() const {
  return isFrameStarted;
}

VkCommandBuffer LveRenderer::getCurrentCommandBuffer() const {
  assert(isFrameStarted);
  return commandBuffers[currFrameIndex];
}

int LveRenderer::getFrameIndex() const {
  assert(isFrameStarted);
  return currFrameIndex;
}

VkCommandBuffer LveRenderer::beginFrame() {
  assert(!isFrameStarted);

  auto result = lveSwapChain->acquireNextImage(&currImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return nullptr;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    error("[LveRenderer::beginFrame] Failed to acquire swap chain image");

  isFrameStarted = true;

  auto commandBuffer = getCurrentCommandBuffer();

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    error("[LveRenderer::beginFrame] Failed to begin recording command buffer");

  return commandBuffer;
}

void LveRenderer::endFrame() {
  assert(isFrameStarted);

  auto commandBuffer = getCurrentCommandBuffer();

  if (vkEndCommandBuffer(commandBuffer))
    error("[LveRenderer::endFrame] Failed to end recording command buffer");

  auto result = lveSwapChain->submitCommandBuffers(&commandBuffer, &currImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()) {
    lveWindow.resetWindowResizedFlag();
    recreateSwapChain();
  }

  if (result != VK_SUCCESS)
    error("[LveRenderer::endFrame] Failed to present swap chain image");

  isFrameStarted = false;
  currFrameIndex = (currFrameIndex + 1) % LveSwapChain::MAX_FRAMES_IN_FLIGHT;
}

void LveRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted);
  assert(commandBuffer == getCurrentCommandBuffer());

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    error("[FirstApp::createCommandBuffers] Failed to begin recording command buffer");

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = lveSwapChain->getRenderPass();
  renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(currImageIndex);

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {{0.01f, 0.01f, 0.01f, 1.f}};
  clearValues[1].depthStencil = {1.f, 0};
  renderPassInfo.clearValueCount = static_cast<u32>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  auto swapChainExtent = lveSwapChain->getSwapChainExtent();
  VkViewport viewport{};
  viewport.x = 0.f;
  viewport.y = 0.f;
  viewport.width = static_cast<float>(swapChainExtent.width);
  viewport.height = static_cast<float>(swapChainExtent.height);
  viewport.minDepth = 0.f;
  viewport.maxDepth = 1.f;
  VkRect2D scissor{{0, 0}, swapChainExtent};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void LveRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted);
  assert(commandBuffer == getCurrentCommandBuffer());

  vkCmdEndRenderPass(commandBuffer);
}

void LveRenderer::createCommandBuffers() {
  commandBuffers.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocinfo{};
  allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocinfo.commandPool = lveDevice.getCommandPool();
  allocinfo.commandBufferCount = static_cast<u32>(commandBuffers.size());

  if (vkAllocateCommandBuffers(lveDevice.device(), &allocinfo, commandBuffers.data()) != VK_SUCCESS)
    error("[LveRenderer::createCommandBuffers] Failed to allocate command buffers");
}

void LveRenderer::freeCommandBuffers() {
  vkFreeCommandBuffers(
      lveDevice.device(),
      lveDevice.getCommandPool(),
      static_cast<u32>(commandBuffers.size()),
      commandBuffers.data());

  commandBuffers.clear();
}

void LveRenderer::recreateSwapChain() {
  auto extent = lveWindow.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = lveWindow.getExtent();
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(lveDevice.device());

  if (lveSwapChain) {
    std::shared_ptr<LveSwapChain> oldSwapChain = std::move(lveSwapChain);
    lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent, oldSwapChain);

    if (!oldSwapChain->compareSwapFormats(*lveSwapChain.get()))
      error("[LveRenderer::recreateSwapChain] Swap chain image (or depth) format has changed");
  } else {
    lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);
  }
}

} // namespace lve

