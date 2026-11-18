#include "VulkanApp.hpp"

#include "wrappers.hpp"

namespace lve {

VulkanApp::~VulkanApp() {
  m_core.freeCommandBuffers(static_cast<u32>(m_cmdBufs.size()), m_cmdBufs.data());
}

void VulkanApp::init(const char* appName, GLFWwindow* window) {
  m_core.init(appName, window);
  m_numImages = m_core.getNumImages();
  createCommandBuffers();
  recordCommandBuffers();
}

void VulkanApp::render() {

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
    wrappers::beginCommandBuffer(m_cmdBufs[i], 0);
    vkCmdClearColorImage(m_cmdBufs[i], m_core.getImage(i), VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &imageRange);

    VkResult res = vkEndCommandBuffer(m_cmdBufs[i]);
    CHECK_VK_RESULT(res, "vkEndCommandBuffer");
  }

  dprint("Recorded command buffers");
}

} // namespace lve

