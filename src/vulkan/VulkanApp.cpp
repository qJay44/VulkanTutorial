#include "VulkanApp.hpp"

#include "utils/utils.hpp"

namespace lve {

namespace {

void GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  auto* app = (VulkanApp*)glfwGetWindowUserPointer(window);
  app->keyCallback(window, key, scancode, action, mods);
}

void GLFW_CursorPosCallback(GLFWwindow* window, double x, double y) {
  auto* app = (VulkanApp*)glfwGetWindowUserPointer(window);
  app->mouseMoveCallback({x, y});
}

} // namespace

void VulkanApp::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  bool pressed = action == GLFW_PRESS;
  VkExtent2D windowExtent = utils::GetWindowExtent(window);

  switch (key) {
    case GLFW_KEY_ESCAPE:
      if (pressed)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
      break;
    case GLFW_KEY_E:
      if (pressed)
        m_showGui = !m_showGui;
      break;
    case GLFW_KEY_R:
      if (pressed) {
        m_cursorEnabled = !m_cursorEnabled;

        if (!m_cursorEnabled) {
          glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
          glfwSetCursorPos(window, windowExtent.width * 0.5, windowExtent.height * 0.5);
          glfwSetCursor(window, nullptr);
        } else {
          glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
      }
      break;
  }

  (void)scancode;
  (void)mods;
}

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
  glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL + !m_cursorEnabled * 2);

  m_core.init(appName, m_window);
  m_device = m_core.getDevice();
  m_numImages = m_core.getNumImages();
  m_pQueue = m_core.getQueue();

  createShaders();
  createPipeline(m_window);
  createModel();

  allocateCommandBuffers();
  recordCommandBuffersGeneral();

  m_camera.pos.z = -2.f;
  m_camera.yaw = glm::radians(90.f);

  m_gui.init(&m_core, this);
}

VulkanApp::~VulkanApp() {
  // vkDeviceWaitIdle(m_device);

  freeCommandBuffers();

  destroyPipeline();
  destroyModel();
  destroyShaders();

  m_gui.destroy();

  glfwDestroyWindow(m_window);
}

void VulkanApp::mouseMoveCallback(vec2 mousePos) {
  if (m_gui.isMouseControlledByImGui() || m_cursorEnabled)
    return;

  auto extent = utils::GetWindowExtent(m_window);
  m_camera.mouseMoveCallback(mousePos, {extent.width , extent.height});

  glfwSetCursorPos(m_window, extent.width * 0.5, extent.height * 0.5);
}

void VulkanApp::render() {
  VkExtent2D windowExtent = utils::GetWindowExtent(m_window);
  float aspect = (float)windowExtent.width / windowExtent.height;
  m_camera.setPerspectiveProjection(glm::radians(60.f), aspect, 0.1f, 100.f);

  float prevTime = glfwGetTime() + 1e-6f;

  while (!glfwWindowShouldClose(m_window)) {
    glfwPollEvents();

    float currTime = glfwGetTime();
    float dt = currTime - prevTime;
    prevTime = currTime;

    updateCameraInput(dt);
    m_camera.update();

    u32 imageIndex = m_pQueue->acquireNextImage();

    updateUniformBuffers(imageIndex);

    if (m_showGui) {
      m_gui.update();
      auto guiCmdBuf = m_gui.prepareCommandBuffer(imageIndex);
      VkCommandBuffer cmdBufs[] = {m_cmdBufs.withGui[imageIndex], guiCmdBuf};

      m_pQueue->submitAsync(&cmdBufs[0], 2, imageIndex);
    } else {
      m_pQueue->submitAsync(m_cmdBufs.withoutGui[imageIndex], imageIndex);
    }

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
  m_cmdBufs.withGui.resize(m_numImages);
  m_core.allocateCommandBuffers(m_numImages, m_cmdBufs.withGui.data());

  m_cmdBufs.withoutGui.resize(m_numImages);
  m_core.allocateCommandBuffers(m_numImages, m_cmdBufs.withoutGui.data());
}

void VulkanApp::recordCommandBuffersGeneral() {
  recordCommandBuffers(false, m_cmdBufs.withGui);
  recordCommandBuffers(true, m_cmdBufs.withoutGui);
}

void VulkanApp::recordCommandBuffers(bool withSecondBarrier, std::vector<VkCommandBuffer>& cmdBufs) {
  for (size_t i = 0; i < cmdBufs.size(); i++) {
    auto& cmdBuf = cmdBufs[i];

    wrappers::beginCommandBuffer(cmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

    wrappers::imageMemBarrier(
      cmdBuf,
      m_core.getImage(i),
      m_core.getSwapchainSurfaceFormat().format,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    );

    beginRendering(cmdBuf, i);

    m_pipeline->bind(cmdBuf);

    m_model.recordCommandBuffer(cmdBuf, *m_pipeline, i);

    endRendering(cmdBuf);

    if (withSecondBarrier) {
      wrappers::imageMemBarrier(
        cmdBuf,
        m_core.getImage(i),
        m_core.getSwapchainSurfaceFormat().format,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
      );
    }

    wrappers::endCommandBuffer(cmdBuf);
  }

  dprint("Recorded command buffers, with second barrier: [{}]", withSecondBarrier ? "Yes" : "No");
}

void VulkanApp::freeCommandBuffers() {
  m_core.freeCommandBuffers(static_cast<u32>(m_cmdBufs.withGui.size()), m_cmdBufs.withGui.data());
  m_core.freeCommandBuffers(static_cast<u32>(m_cmdBufs.withoutGui.size()), m_cmdBufs.withoutGui.data());
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
  VkClearValue clearColor{
    .color = {{0.01f, 0.01f, 0.01f, 1.f}},
  };

  VkClearValue clearDepth{
    .depthStencil = {.depth = 1.f, .stencil = 0},
  };

  m_core.beginDynamicRendering(cmdBuf, imageIndex, &clearColor, &clearDepth);
}

void VulkanApp::endRendering(VkCommandBuffer cmdBuf) {
  m_core.endDynamicRendering(cmdBuf);
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

