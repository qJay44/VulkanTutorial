#include "VulkanApp.hpp"

#include "utils/utils.hpp"
#include "wrappers.hpp"

namespace lve {

void VulkanApp::init(const char* appName, GLFWwindow* window) {
  m_core.init(appName, window);
  m_numImages = m_core.getNumImages();
  m_pQueue = m_core.getQueue();
  m_renderPass = m_core.createSimpleRenderPass();
  m_framebuffers = m_core.createFramebuffers(m_renderPass);

  createCommandBuffers();
  recordCommandBuffers(GetWindowExtent(window));
  createShaders();
}

void VulkanApp::render() {
  u32 imageIndex = m_pQueue->acquireNextImage();
  m_pQueue->submitAsync(m_cmdBufs[imageIndex]);
  m_pQueue->present(imageIndex);
}

void VulkanApp::destroy() {
  m_core.freeCommandBuffers(static_cast<u32>(m_cmdBufs.size()), m_cmdBufs.data());
  vkDestroyRenderPass(m_core.getDevice(), m_renderPass, nullptr);
  vkDestroyShaderModule(m_core.getDevice(), m_vs.shaderModule, nullptr);
  vkDestroyShaderModule(m_core.getDevice(), m_fs.shaderModule, nullptr);
  delete this;
}

void VulkanApp::createCommandBuffers() {
  m_cmdBufs.resize(m_numImages);
  m_core.createCommandBuffers(m_numImages, m_cmdBufs.data());
}

void VulkanApp::recordCommandBuffers(VkExtent2D windowExtent) {
  VkClearColorValue clearColor{{1.f, 0.f, 0.f, 0.f}};
  VkClearValue clearValue{};
  clearValue.color = clearColor;

  VkRenderPassBeginInfo renderPassBeginInfo{
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .pNext = nullptr,
    .renderPass = m_renderPass,
    .framebuffer = 0, // Assigned in the loop
    .renderArea = {
      .offset = {
        .x = 0,
        .y = 0,
      },
      .extent = windowExtent,
    },
    .clearValueCount = 1,
    .pClearValues = &clearValue,
  };

  for (size_t i = 0; i < m_cmdBufs.size(); i++) {
    wrappers::beginCommandBuffer(m_cmdBufs[i], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

    renderPassBeginInfo.framebuffer = m_framebuffers[i];

    vkCmdBeginRenderPass(m_cmdBufs[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(m_cmdBufs[i]);

    VkResult res = vkEndCommandBuffer(m_cmdBufs[i]);
    CHECK_VK_RESULT(res, "vkEndCommandBuffer");
  }

  dprint("Recorded command buffers");
}

void VulkanApp::createShaders() {
  m_vs.createShaderModuleFromText(m_core.getDevice(), "src/shaders/test.vert");
  m_fs.createShaderModuleFromText(m_core.getDevice(), "src/shaders/test.frag");
}

} // namespace lve

