#include "VulkanApp.hpp"

#include "wrappers.hpp"

namespace lve {

void VulkanApp::init(const char* appName, GLFWwindow* window) {
  m_core.init(appName, window);
  m_numImages = m_core.getNumImages();
  m_pQueue = m_core.getQueue();

  createCommandBuffers();
  recordCommandBuffers();
}

void VulkanApp::render() {
  u32 imageIndex = m_pQueue->acquireNextImage();
  m_pQueue->submitAsync(m_cmdBufs[imageIndex]);
  m_pQueue->present(imageIndex);
}

void VulkanApp::destroy() {
  m_core.freeCommandBuffers(static_cast<u32>(m_cmdBufs.size()), m_cmdBufs.data());
  delete this;
}

void VulkanApp::createCommandBuffers() {
  m_cmdBufs.resize(m_numImages);
  m_core.createCommandBuffers(m_numImages, m_cmdBufs.data());
}

void VulkanApp::recordCommandBuffers() {
  VkClearColorValue clearColor{{1.f, 0.f, 0.f, 0.f}};

  VkImageSubresourceRange imageRange{
    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    .baseMipLevel = 0,
    .levelCount = 1,
    .baseArrayLayer = 0,
    .layerCount = 1,
  };

  for (size_t i = 0; i < m_cmdBufs.size(); i++) {
    VkImageMemoryBarrier presentToClearBarrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .pNext = nullptr,
      .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
      .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = m_core.getImage(static_cast<u32>(i)),
      .subresourceRange = imageRange,
    };

    VkImageMemoryBarrier clearToPresentBarrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .pNext = nullptr,
      .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = m_core.getImage(static_cast<u32>(i)),
      .subresourceRange = imageRange,
    };

    wrappers::beginCommandBuffer(m_cmdBufs[i], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

    vkCmdPipelineBarrier(m_cmdBufs[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, // dependency flags
        0, nullptr, // memory barriers
        0, nullptr, // buffer memory barriers
        1, &presentToClearBarrier); // image memory barriers

    vkCmdClearColorImage(
        m_cmdBufs[i],
        m_core.getImage(i),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        &clearColor,
        1,
        &imageRange);

    vkCmdPipelineBarrier(m_cmdBufs[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &clearToPresentBarrier);

    VkResult res = vkEndCommandBuffer(m_cmdBufs[i]);
    CHECK_VK_RESULT(res, "vkEndCommandBuffer");
  }

  dprint("Recorded command buffers");
}

} // namespace lve

