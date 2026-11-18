#pragma once

namespace lve {

class VulkanQueue {
public:
  VulkanQueue() = default;
  ~VulkanQueue() = default;

  void init(VkDevice device, VkSwapchainKHR swapchain, u32 queueFamily, u32 queueIndex);
  void destroy();

  u32 acquireNextImage();

  void submitSync(VkCommandBuffer cmdBuf);
  void submitAsync(VkCommandBuffer cmdBuf);
  void present(u32 imageIndex);
  void waitIdle();

private:
  VkDevice m_device = VK_NULL_HANDLE;
  VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
  VkQueue m_queue = VK_NULL_HANDLE;
  VkSemaphore m_renderCompleteSem;
  VkSemaphore m_presentCompleteSem;

private:
  void createSemaphores();
};

} // namespace lve

