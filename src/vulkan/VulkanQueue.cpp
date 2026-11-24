#include "VulkanQueue.hpp"

namespace lve {

void VulkanQueue::init(VkDevice device, VkSwapchainKHR swapchain, u32 queueFamily, u32 queueIndex) {
  m_device = device;
  m_swapchain = swapchain;

  vkGetDeviceQueue(device, queueFamily, queueIndex, &m_queue);
  dprint("Queue acquired");

  createSemaphores();
}

void VulkanQueue::destroy() {
  vkDestroySemaphore(m_device, m_presentCompleteSem, nullptr);
  vkDestroySemaphore(m_device, m_renderCompleteSem, nullptr);
}

u32 VulkanQueue::acquireNextImage() {
  u32 imageIndex = 0;
  VkResult res = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_presentCompleteSem, nullptr, &imageIndex);
  CHECK_VK_RESULT(res, "vkAcquireNextImageKHR");

  return imageIndex;
}

void VulkanQueue::submitSync(VkCommandBuffer cmdBuf) {
  VkSubmitInfo submitInfo{
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext = nullptr,
    .waitSemaphoreCount = 0,
    .pWaitSemaphores = VK_NULL_HANDLE,
    .pWaitDstStageMask = VK_NULL_HANDLE,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmdBuf,
    .signalSemaphoreCount = 0,
    .pSignalSemaphores = VK_NULL_HANDLE,
  };

  VkResult res = vkQueueSubmit(m_queue, 1, &submitInfo, nullptr);
  CHECK_VK_RESULT(res, "vkQueueSubmit");
}

void VulkanQueue::submitAsync(VkCommandBuffer cmdBuf) {
  VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkSubmitInfo submitInfo{
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext = nullptr,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &m_presentCompleteSem,
    .pWaitDstStageMask = &waitFlags,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmdBuf,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = &m_renderCompleteSem,
  };

  VkResult res = vkQueueSubmit(m_queue, 1, &submitInfo, nullptr);
  CHECK_VK_RESULT(res, "vkQueueSubmit");
}

void VulkanQueue::present(u32 imageIndex) {
  VkPresentInfoKHR presentInfo{
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .pNext = nullptr,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &m_renderCompleteSem,
    .swapchainCount = 1,
    .pSwapchains = &m_swapchain,
    .pImageIndices = &imageIndex,
    .pResults = nullptr,
  };

  VkResult res = vkQueuePresentKHR(m_queue, &presentInfo);
  CHECK_VK_RESULT(res, "vkQueuePresentKHR");

  waitIdle(); // NOTE: Authours workaround
}

void VulkanQueue::waitIdle() {
  vkQueueWaitIdle(m_queue);
}

void VulkanQueue::createSemaphores() {
  m_renderCompleteSem = wrappers::createSemaphore(m_device);
  m_presentCompleteSem = wrappers::createSemaphore(m_device);
}

} // namespace lve

