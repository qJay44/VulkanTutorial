#include "VulkanApp.hpp"

#include "utils/utils.hpp"
#include "wrappers.hpp"

namespace lve {

namespace {

void GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);

  (void)scancode;
  (void)mods;
}

void GLFW_CursorPosCallback(GLFWwindow* window, double x, double y) {
  auto* app = (VulkanApp*)glfwGetWindowUserPointer(window);
  app->mouseMoveCallback({x, y});
}

struct UniformData {
  mat4 WVP{};
};

} // namespace

VulkanApp::VulkanApp(const char* appName) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  int initWidth = 1600;
  int initHeight = 900;

  if (m_window)
    error("[VulkanApp::init] Window already exists");

  m_window = glfwCreateWindow(initWidth, initHeight, "MyProgram", nullptr, nullptr);
  if (!m_window)
    error("[VulkanApp::init] Didn't create window");

  glfwSetWindowUserPointer(m_window, this);
  glfwSetKeyCallback(m_window, GLFW_KeyCallback);
  glfwSetCursorPosCallback(m_window, GLFW_CursorPosCallback);
  glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPos(m_window, initWidth / 2.f, initHeight / 2.f);

  m_core.init(appName, m_window);
  m_device = m_core.getDevice();
  m_numImages = m_core.getNumImages();
  m_pQueue = m_core.getQueue();

  createSimpleRenderPass();
  createFramebuffers();
  createShaders();
  createUniformBuffers();
  createMesh();
  createPipeline(m_window);

  allocateCommandBuffers();
  recordCommandBuffers(GetWindowExtent(m_window));

  m_camera.pos.z = -2.f;
  m_camera.yaw = glm::radians(90.f);
}

VulkanApp::~VulkanApp() {
  // vkDeviceWaitIdle(m_device);

  freeCommandBuffers();

  destroyPipeline();
  destroyMesh();
  destroyUniformBuffers();
  destroyShaders();
  destroyFramebuffers();
  destroySimpleRenderPass();

  glfwDestroyWindow(m_window);
}

void VulkanApp::mouseMoveCallback(vec2 mousePos) {
  constexpr float pi = glm::pi<float>();
  constexpr float pi_2 = pi * 0.5f;

  auto extent = GetWindowExtent(m_window);

  vec2 winSize{extent.width, extent.height};
  vec2 winCenter = winSize * 0.5f;
  vec2 distFromCenter = mousePos - winCenter;

  vec2 delta = m_camera.sensitivity * distFromCenter / winCenter;
  m_camera.yaw += -delta.x;
  m_camera.pitch += delta.y;

  m_camera.yaw = glm::mod(m_camera.yaw, 2.f * pi);
  m_camera.pitch = glm::clamp(m_camera.pitch, -pi_2 + 0.1f, pi_2 - 0.1f);
}

void VulkanApp::render() {
  VkExtent2D windowExtent = GetWindowExtent(m_window);
  float aspect = (float)windowExtent.width / windowExtent.height;
  m_camera.setPerspectiveProjection(glm::radians(60.f), aspect, 0.1f, 100.f);

  float prevTime = glfwGetTime() + 1e-6f;

  while (!glfwWindowShouldClose(m_window)) {
    glfwPollEvents();
    glfwSetCursorPos(m_window, windowExtent.width * 0.5f, windowExtent.height * 0.5f);

    float currTime = glfwGetTime();
    float dt = currTime - prevTime;
    prevTime = currTime;

    updateCameraInput(dt);
    m_camera.update();

    u32 imageIndex = m_pQueue->acquireNextImage();

    updateUniformBuffers(imageIndex);

    m_pQueue->submitAsync(m_cmdBufs[imageIndex]);
    m_pQueue->present(imageIndex);
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

  VkExtent2D windowExtent = GetWindowExtent(m_window);
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

void VulkanApp::createMesh() {
  loadVertexBuffer();
  loadTexture();
}

void VulkanApp::loadVertexBuffer() {
  struct Vertex {
    vec3 pos;
    vec2 tex;
  };

  std::vector<Vertex> vertices = {
    {{-1.0f, -1.0f,  0.0f}, {0.f, 0.f}},
    {{ 1.0f, -1.0f,  0.0f}, {1.f, 0.f}},
    {{ 1.0f,  1.0f,  0.0f}, {1.f, 1.f}},
    {{ 1.0f,  1.0f,  0.0f}, {1.f, 1.f}},
    {{-1.0f,  1.0f,  0.0f}, {0.f, 1.f}},
    {{-1.0f, -1.0f,  0.0f}, {0.f, 0.f}},
  };

  m_mesh.m_vertexBufferSize = sizeof(vertices[0]) * vertices.size();
  m_mesh.m_vb = m_core.createVertexBuffer(vertices.data(), m_mesh.m_vertexBufferSize);
  dprint("Created vertex buffer");
}

void VulkanApp::loadTexture() {
  m_mesh.m_tex = new VulkanTexture();
  m_mesh.m_tex->create(&m_core, "res/tex/debug/uvChecker.jpg");
}

void VulkanApp::createUniformBuffers() {
  m_core.createUniformBuffers(m_uniformBuffers, sizeof(UniformData));
}

void VulkanApp::createPipeline(GLFWwindow* window) {
  m_pipeline.init(
    m_device,
    window,
    m_renderPass,
    m_vs.shaderModule,
    m_fs.shaderModule,
    &m_mesh,
    m_numImages,
    m_uniformBuffers,
    sizeof(UniformData)
  );
}

void VulkanApp::allocateCommandBuffers() {
  m_cmdBufs.resize(m_numImages);
  m_core.allocateCommandBuffers(m_numImages, m_cmdBufs.data());
}

void VulkanApp::recordCommandBuffers(VkExtent2D windowExtent) {
  VkClearColorValue clearColor{{0.01f, 0.01f, 0.01f, 0.f}};
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

    m_pipeline.bind(m_cmdBufs[i], i);

    u32 vertexCount = 6;
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

void VulkanApp::freeCommandBuffers() {
  m_core.freeCommandBuffers(static_cast<u32>(m_cmdBufs.size()), m_cmdBufs.data());
  dprint("Freed command buffers");
}

void VulkanApp::updateCameraInput(float dt) {
  vec3 forward = m_camera.orientation;
  vec3 up = m_camera.up;
  vec3 right = glm::normalize(glm::cross(up, forward));

  vec3 totalMove{};
  float mult = 1.f;

  if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) totalMove += forward;
  if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) totalMove += -right;
  if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) totalMove += -forward;
  if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) totalMove += right;
  if (glfwGetKey(m_window, GLFW_KEY_SPACE)        == GLFW_PRESS) totalMove += -up;
  if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) totalMove += up;
  if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) mult = m_camera.speedMult;

  m_camera.pos += totalMove * m_camera.speed * mult * dt;
}

void VulkanApp::updateUniformBuffers(u32 imageIndex) {
  static float foo = 0.f;
  mat4 rotate = glm::rotate(mat4(1.f), glm::radians(foo), glm::normalize(vec3(0.f, 0.f, 1.f)));
  mat4 world{1.f};
  world = rotate;

  UniformData ubo{
    .WVP = (m_camera.proj * m_camera.view) * world,
  };

  m_uniformBuffers[imageIndex].update(m_device, &ubo, sizeof(UniformData));

  foo += 0.001f;
}

void VulkanApp::destroyPipeline() { m_pipeline.destroy(); }

void VulkanApp::destroyUniformBuffers() {
  for (auto& ubo : m_uniformBuffers)
    ubo.destroy(m_device);
}
void VulkanApp::destroyMesh() {
  m_mesh.destroy(m_device);
  dprint("Destroyed mesh");
}

void VulkanApp::destroyShaders() {
  vkDestroyShaderModule(m_device, m_vs.shaderModule, nullptr);
  vkDestroyShaderModule(m_device, m_fs.shaderModule, nullptr);
  dprint("Destroyed shaders");
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

