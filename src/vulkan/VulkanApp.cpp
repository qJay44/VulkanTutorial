#include "VulkanApp.hpp"

#include "utils/utils.hpp"

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

  createShaders();
  createPipeline(m_window);
  createModel();

  allocateCommandBuffers();
  recordCommandBuffers();

  m_camera.pos.z = -2.f;
  m_camera.yaw = glm::radians(90.f);
}

VulkanApp::~VulkanApp() {
  // vkDeviceWaitIdle(m_device);

  freeCommandBuffers();

  destroyPipeline();
  destroyModel();
  destroyShaders();

  glfwDestroyWindow(m_window);
}

void VulkanApp::mouseMoveCallback(vec2 mousePos) {
  constexpr float pi = glm::pi<float>();
  constexpr float pi_2 = pi * 0.5f;

  auto extent = utils::GetWindowExtent(m_window);

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
  VkExtent2D windowExtent = utils::GetWindowExtent(m_window);
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

void VulkanApp::createShaders() {
  m_vs.createShaderModuleFromText(m_device, "src/shaders/test.vert");
  m_fs.createShaderModuleFromText(m_device, "src/shaders/test.frag");
}

void VulkanApp::createModel() {
  m_texDebug = new VulkanTexture(&m_core);
  m_model.init(&m_core);
  loadVertexBuffer();
  loadTexture();
  m_model.createDescriptorSets(*m_pipeline);
}

void VulkanApp::loadVertexBuffer() {
  std::vector<VulkanModel::Vertex> vertices = {
    {{-1.0f, -1.0f,  0.0f}, {0.f, 0.f}, {}, {}, {}},
    {{ 1.0f, -1.0f,  0.0f}, {1.f, 0.f}, {}, {}, {}},
    {{ 1.0f,  1.0f,  0.0f}, {1.f, 1.f}, {}, {}, {}},
    {{-1.0f,  1.0f,  0.0f}, {0.f, 1.f}, {}, {}, {}},

    {{-1.0f, -1.0f,  5.0f}, {0.f, 0.f}, {}, {}, {}},
    {{ 1.0f, -1.0f,  5.0f}, {1.f, 0.f}, {}, {}, {}},
    {{ 1.0f,  1.0f,  5.0f}, {1.f, 1.f}, {}, {}, {}},
    {{-1.0f,  1.0f,  5.0f}, {0.f, 1.f}, {}, {}, {}},
  };

  std::vector<u32> indices = {
    0, 1, 2,
    2, 3, 0,

    4, 5, 6,
    6, 7, 4
  };

  m_model.addMesh(vertices, indices);
  dprint("Created vertex buffer");
}

void VulkanApp::loadTexture() {
  m_texDebug->load("res/tex/debug/uvChecker.jpg");
  m_model.addTexture(m_texDebug);
}

void VulkanApp::createPipeline(GLFWwindow* window) {
  m_pipeline = new VulkanGraphicsPipeline2(
    m_device,
    window,
    m_vs.shaderModule,
    m_fs.shaderModule,
    m_numImages,
    m_core.getSwapchainSurfaceFormat().format,
    m_core.getDepthFormat()
  );
}

void VulkanApp::allocateCommandBuffers() {
  m_cmdBufs.resize(m_numImages);
  m_core.allocateCommandBuffers(m_numImages, m_cmdBufs.data());
}

void VulkanApp::recordCommandBuffers() {
  for (size_t i = 0; i < m_cmdBufs.size(); i++) {
    wrappers::beginCommandBuffer(m_cmdBufs[i], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

    wrappers::imageMemBarrier(
      m_cmdBufs[i],
      m_core.getImage(i),
      m_core.getSwapchainSurfaceFormat().format,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    );

    beginRendering(m_cmdBufs[i], i);

    m_pipeline->bind(m_cmdBufs[i]);

    m_model.recordCommandBuffer(m_cmdBufs[i], *m_pipeline, i);

    endRendering(m_cmdBufs[i]);

    wrappers::imageMemBarrier(
      m_cmdBufs[i],
      m_core.getImage(i),
      m_core.getSwapchainSurfaceFormat().format,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    );

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

  mat4 wvp = (m_camera.proj * m_camera.view) * world;
  m_model.update(imageIndex, wvp);

  foo += 0.001f;
}

void VulkanApp::beginRendering(VkCommandBuffer cmdBuf, u32 imageIndex) {
  VkExtent2D windowExtent = utils::GetWindowExtent(m_window);

  VkClearValue clearColor{
    .color = {{0.01f, 0.01f, 0.01f, 1.f}},
  };

  VkClearValue clearDepth{
    .depthStencil = {.depth = 1.f, .stencil = 0},
  };

  VkRenderingAttachmentInfoKHR colorAttachment{
    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
    .pNext = nullptr,
    .imageView = m_core.getImageView(imageIndex),
    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    .resolveMode = VK_RESOLVE_MODE_NONE,
    .resolveImageView = VK_NULL_HANDLE,
    .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .clearValue = clearColor,
  };

  VkRenderingAttachmentInfoKHR depthAttachment{
    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
    .pNext = nullptr,
    .imageView = m_core.getDepthImageView(imageIndex),
    .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    .resolveMode = VK_RESOLVE_MODE_NONE,
    .resolveImageView = VK_NULL_HANDLE,
    .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .clearValue = clearDepth,
  };

  VkRenderingInfoKHR renderingInfo{
    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
    .pNext = nullptr,
    .flags = 0,
    .renderArea = {{0, 0}, windowExtent},
    .layerCount = 1,
    .viewMask = 0,
    .colorAttachmentCount = 1,
    .pColorAttachments = &colorAttachment,
    .pDepthAttachment = &depthAttachment,
    .pStencilAttachment = nullptr,
  };

  vkCmdBeginRendering(cmdBuf, &renderingInfo);
}

void VulkanApp::endRendering(VkCommandBuffer cmdBuf) {
  vkCmdEndRendering(cmdBuf);
}

void VulkanApp::destroyPipeline() { delete m_pipeline; }

void VulkanApp::destroyModel() {
  m_model.destroy();
  m_texDebug->destroy(m_device);
  dprint("Destroyed model");
}

void VulkanApp::destroyShaders() {
  vkDestroyShaderModule(m_device, m_vs.shaderModule, nullptr);
  vkDestroyShaderModule(m_device, m_fs.shaderModule, nullptr);
  dprint("Destroyed shaders");
}

} // namespace lve

