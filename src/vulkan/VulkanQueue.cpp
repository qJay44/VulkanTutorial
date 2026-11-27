#include "VulkanQueue.hpp"

namespace lve {

void VulkanQueue::init(VkDevice device, VkSwapchainKHR swapchain, u32 queueFamily, u32 queueIndex, u32 numImages) {
  m_device = device;
  m_swapchain = swapchain;
  m_numImages = numImages;

  vkGetDeviceQueue(device, queueFamily, queueIndex, &m_queue);
  dprint("Queue acquired");

  createSyncObjects();
}

void VulkanQueue::destroy() {
  for (VkSemaphore& sem : m_imageAvailableSems)
    vkDestroySemaphore(m_device, sem, nullptr);

  for (VkSemaphore& sem : m_renderFinishedSems)
    vkDestroySemaphore(m_device, sem, nullptr);

  for (VkFence& fence : m_inFlightFences)
    vkDestroyFence(m_device, fence, nullptr);
}

u32 VulkanQueue::acquireNextImage() {
  vkWaitForFences(m_device, 1, &m_inFlightFences[m_frameIndex], VK_TRUE, UINT64_MAX);
  vkResetFences(m_device, 1, &m_inFlightFences[m_frameIndex]);

  u32 imageIndex = 0;
  VkResult res = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_imageAvailableSems[m_frameIndex], VK_NULL_HANDLE, &imageIndex);
  CHECK_VK_RESULT(res, "vkAcquireNextImageKHR");

  if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE &&
      m_imagesInFlight[imageIndex] != m_inFlightFences[m_frameIndex])
    vkWaitForFences(m_device, 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);

  m_imagesInFlight[imageIndex] = m_inFlightFences[m_frameIndex];

  return imageIndex;
}

VkQueue VulkanQueue::getHandle() {
  return m_queue;
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
  submitAsync(&cmdBuf, 1);
}

void VulkanQueue::submitAsync(VkCommandBuffer* cmdBufs, u32 numCmdBufs) {
  VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkSubmitInfo submitInfo{
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext = nullptr,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &m_imageAvailableSems[m_frameIndex],
    .pWaitDstStageMask = &waitFlags,
    .commandBufferCount = numCmdBufs,
    .pCommandBuffers = cmdBufs,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = &m_renderFinishedSems[m_frameIndex],
  };

  VkResult res = vkQueueSubmit(m_queue, 1, &submitInfo, m_inFlightFences[m_frameIndex]);
  CHECK_VK_RESULT(res, "vkQueueSubmit");
}

void VulkanQueue::present(u32 imageIndex) {
  VkPresentInfoKHR presentInfo{
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .pNext = nullptr,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &m_renderFinishedSems[m_frameIndex],
    .swapchainCount = 1,
    .pSwapchains = &m_swapchain,
    .pImageIndices = &imageIndex,
    .pResults = nullptr,
  };

  VkResult res = vkQueuePresentKHR(m_queue, &presentInfo);
  CHECK_VK_RESULT(res, "vkQueuePresentKHR");

  m_frameIndex = (m_frameIndex + 1) % m_numImages;
}

void VulkanQueue::waitIdle() {
  vkQueueWaitIdle(m_queue);
}

void VulkanQueue::createSyncObjects() {
  m_imageAvailableSems.resize(m_numImages);
  for (VkSemaphore& sem : m_imageAvailableSems)
    sem = wrappers::createSemaphore(m_device);

  m_renderFinishedSems.resize(m_numImages);
  for (VkSemaphore& sem : m_renderFinishedSems)
    sem = wrappers::createSemaphore(m_device);

  m_inFlightFences.resize(m_numImages);
  m_imagesInFlight.resize(m_numImages, VK_NULL_HANDLE);

  VkFenceCreateInfo fenceInfo{
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .pNext = nullptr,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT, // Start signaled to avoid stalls
  };

  for (VkFence& fence : m_inFlightFences) {
    VkResult res = vkCreateFence(m_device, &fenceInfo, nullptr, &fence);
    CHECK_VK_RESULT(res, "vkCreateFence");
  }
}

} // namespace lve

