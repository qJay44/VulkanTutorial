#pragma once

namespace lve {

class VulkanQueue {
public:
  VulkanQueue() = default;
  ~VulkanQueue() = default;

  void init(VkDevice device, VkSwapchainKHR swapchain, u32 queueFamily, u32 queueIndex, u32 numImages);
  void destroy();

  u32 acquireNextImage();
  VkQueue getHandle();

  void submitSync(VkCommandBuffer cmdBuf);
  void submitAsync(VkCommandBuffer cmdBufs);
  void submitAsync(VkCommandBuffer* cmdBufs, u32 numCmdBufs);
  void present(u32 imageIndex);
  void waitIdle();

private:
  VkDevice m_device = VK_NULL_HANDLE;
  VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
  VkQueue m_queue = VK_NULL_HANDLE;
  std::vector<VkSemaphore> m_imageAvailableSems;
  std::vector<VkSemaphore> m_renderFinishedSems;
  std::vector<VkFence> m_inFlightFences;
  std::vector<VkFence> m_imagesInFlight; // "reference" to [m_inFlightFences] fences, don't call destroy on this fences
  u32 m_numImages = 0;
  u32 m_frameIndex = 0;

private:
  void createSyncObjects();
};

} // namespace lve

