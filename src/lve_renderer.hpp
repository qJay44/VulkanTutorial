#pragma once

#include "lve_swap_chain.hpp"
#include "lve_window.hpp"
#include "lve_device.hpp"

namespace lve {

class LveRenderer {
public:
  LveRenderer(LveWindow& lveWindow, LveDevice& lveDevice);
  ~LveRenderer();

  LveRenderer(const LveRenderer&) = delete;
  LveRenderer& operator=(const LveRenderer&) = delete;

  VkRenderPass getSwapChainRenderPass() const;
  bool isFrameInProgress() const;
  VkCommandBuffer getCurrentCommandBuffer() const;
  int getFrameIndex() const;

  VkCommandBuffer beginFrame();
  void endFrame();

  void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
  void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

private:
  LveWindow& lveWindow;
  LveDevice& lveDevice;
  std::unique_ptr<LveSwapChain> lveSwapChain;
  std::vector<VkCommandBuffer> commandBuffers;

  u32 currImageIndex = 0;
  int currFrameIndex = 0;
  bool isFrameStarted = false;

private:
  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapChain();
};

} // namespace lve

