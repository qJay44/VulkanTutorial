#include "VulkanApp.hpp"

#include "utils/utils.hpp"
#include "wrappers.hpp"

namespace lve {

void VulkanApp::init(const char* appName, GLFWwindow* window) {
  m_core.init(appName, window);
  m_device = m_core.getDevice();
  m_numImages = m_core.getNumImages();
  m_pQueue = m_core.getQueue();
  m_renderPass = m_core.createSimpleRenderPass();
  m_framebuffers = m_core.createFramebuffers(m_renderPass);

  createShaders();
  createPipeline(window);

  createCommandBuffers();
  recordCommandBuffers(GetWindowExtent(window));
}

void VulkanApp::render() {
  u32 imageIndex = m_pQueue->acquireNextImage();
  m_pQueue->submitAsync(m_cmdBufs[imageIndex]);
  m_pQueue->present(imageIndex);
}

void VulkanApp::destroy() {
  vkDeviceWaitIdle(m_device);

  m_core.freeCommandBuffers(static_cast<u32>(m_cmdBufs.size()), m_cmdBufs.data());
  vkDestroyShaderModule(m_device, m_vs.shaderModule, nullptr);
  vkDestroyShaderModule(m_device, m_fs.shaderModule, nullptr);
  vkDestroyRenderPass(m_device, m_renderPass, nullptr);
  delete this; // Also destroyes pipeline
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

    m_pipeline.bind(m_cmdBufs[i]);

    u32 vertexCount = 3;
    u32 instanceCount = 1;
    u32 firstVertex = 0;
    u32 firstInstance = 0;

    vkCmdDraw(m_cmdBufs[i], vertexCount, instanceCount, firstVertex, firstInstance);

    vkCmdEndRenderPass(m_cmdBufs[i]);

    VkResult res = vkEndCommandBuffer(m_cmdBufs[i]);
    CHECK_VK_RESULT(res, "vkEndCommandBuffer");
  }

  dprint("Recorded command buffers");
}

void VulkanApp::createShaders() {
  m_vs.createShaderModuleFromText(m_device, "src/shaders/test.vert");
  m_fs.createShaderModuleFromText(m_device, "src/shaders/test.frag");
}

void VulkanApp::createPipeline(GLFWwindow* window) {
  m_pipeline.init(m_device, window, m_renderPass, m_vs.shaderModule, m_fs.shaderModule);
}

} // namespace lve

