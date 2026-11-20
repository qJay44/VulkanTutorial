#include "VulkanApp.hpp"

#include "utils/utils.hpp"
#include "wrappers.hpp"

namespace lve {

VulkanApp::~VulkanApp() {
  destroy();
}

void VulkanApp::init(const char* appName, GLFWwindow* window) {
  m_pWindow = window;

  m_core.init(appName, window);
  m_device = m_core.getDevice();
  m_numImages = m_core.getNumImages();
  m_pQueue = m_core.getQueue();

  createSimpleRenderPass();
  createFramebuffers();
  createShaders();
  createPipeline(window);

  allocateCommandBuffers();
  recordCommandBuffers(GetWindowExtent(window));
}

void VulkanApp::render() {
  u32 imageIndex = m_pQueue->acquireNextImage();
  m_pQueue->submitAsync(m_cmdBufs[imageIndex]);
  m_pQueue->present(imageIndex);
}

void VulkanApp::destroy() {
  // NOTE: Assuming [m_pWindow] is assigned only during init()
  if (m_pWindow) {
    vkDeviceWaitIdle(m_device);

    freeCommandBuffers();

    destroyPipeline();
    destroyVertexBuffer();
    destroyShaders();
    destroyFramebuffers();
    destroySimpleRenderPass();

    m_pWindow = nullptr;
  }
}

void VulkanApp::createSimpleRenderPass() {
  VkAttachmentDescription attachDesc{
    .flags = 0,
    .format = m_core.getSwapchainSurfaceFormat().format,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  };

  VkAttachmentReference attachRef{
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };

  VkSubpassDescription subpassDesc{
    .flags = 0,
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .inputAttachmentCount = 0,
    .pInputAttachments = nullptr,
    .colorAttachmentCount = 1,
    .pColorAttachments = &attachRef,
    .pResolveAttachments = nullptr,
    .pDepthStencilAttachment = nullptr,
    .preserveAttachmentCount = 0,
    .pPreserveAttachments = nullptr,
  };

  VkRenderPassCreateInfo renderPassCreateInfo{
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .attachmentCount = 1,
    .pAttachments = &attachDesc,
    .subpassCount = 1,
    .pSubpasses = &subpassDesc,
    .dependencyCount = 0,
    .pDependencies = nullptr,
  };

  VkResult res = vkCreateRenderPass(m_device, &renderPassCreateInfo, nullptr, &m_renderPass);
  CHECK_VK_RESULT(res, "vkCreateRenderPass");
  dprint("Created a simple render pass");
}

void VulkanApp::createFramebuffers() {
  m_framebuffers.resize(m_core.getNumImages());

  VkExtent2D windowExtent = GetWindowExtent(m_pWindow);
  VkResult res;

  VkFramebufferCreateInfo fbCreateInfo{
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .renderPass = m_renderPass,
    .attachmentCount = 1,
    .pAttachments = nullptr, // Assigned in the loop
    .width = windowExtent.width,
    .height = windowExtent.height,
    .layers = 1,
  };

  for (u32 i = 0; i < m_core.getNumImages(); i++) {
    fbCreateInfo.pAttachments = &m_core.getImageView(i);

    res = vkCreateFramebuffer(m_device, &fbCreateInfo, nullptr, &m_framebuffers[i]);
    CHECK_VK_RESULT(res, "vkCreateFramebuffer");
  }

  dprint("Created framebuffers");
}

void VulkanApp::createShaders() {
  m_vs.createShaderModuleFromText(m_device, "src/shaders/test.vert");
  m_fs.createShaderModuleFromText(m_device, "src/shaders/test.frag");
}

void VulkanApp::createVertexBuffer() {
  struct Vertex {
    vec3 pos;
    vec2 tex;
  };

  std::vector<Vertex> vertices = {
    {{-1.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
    {{ 1.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},
    {{ 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f}},
  };

  m_mesh.m_vertexBufferSize = sizeof(vertices[0]) * vertices.size();
  m_mesh.m_vb = m_core.createVertexBuffer(vertices.data(), m_mesh.m_vertexBufferSize);
}

void VulkanApp::createPipeline(GLFWwindow* window) {
  m_pipeline.init(m_device, window, m_renderPass, m_vs.shaderModule, m_fs.shaderModule);
}

void VulkanApp::allocateCommandBuffers() {
  m_cmdBufs.resize(m_numImages);
  m_core.allocateCommandBuffers(m_numImages, m_cmdBufs.data());
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

void VulkanApp::destroyPipeline() {
  m_pipeline.destroy();
}

void VulkanApp::freeCommandBuffers() {
  m_core.freeCommandBuffers(static_cast<u32>(m_cmdBufs.size()), m_cmdBufs.data());
  dprint("Freed command buffers");
}

void VulkanApp::destroyShaders() {
  vkDestroyShaderModule(m_device, m_vs.shaderModule, nullptr);
  vkDestroyShaderModule(m_device, m_fs.shaderModule, nullptr);
  dprint("Destroyed shaders");
}

void VulkanApp::destroyVertexBuffer() {
  m_mesh.destroy(m_device);
  dprint("Destroyed vertex buffer");
}

void VulkanApp::destroyFramebuffers() {
  for (auto& fb : m_framebuffers)
    vkDestroyFramebuffer(m_device, fb, nullptr);
  dprint("Destroyed framebuffers");
}
void VulkanApp::destroySimpleRenderPass() {
  vkDestroyRenderPass(m_device, m_renderPass, nullptr);
  dprint("Destroyed simple render pass");
}

} // namespace lve

