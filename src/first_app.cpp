#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "simple_render_system.hpp"

namespace lve {

struct GlobalUbo {
  mat4 projectionView{1.f};
  vec4 lightAmbientColor{1.f, 1.f, 1.f, 0.02f}; // w - intensity
  vec3 lightPos{-1.f};
  alignas(16) vec4 lightColor{1.f}; // w - light intensity
};

FirstApp::FirstApp() {
  lveWindow.setCursorMoveCallback([&](double x, double y) {
    auto extent = lveWindow.getExtent();
    dvec2 mousePos{x, y};
    dvec2 sensitivity{2.f};

    dvec2 winSize{extent.width, extent.height};
    dvec2 winCenter = winSize * 0.5;
    dvec2 distFromCenter = mousePos - winCenter;

    dvec2 delta = sensitivity * distFromCenter / winCenter;
    viewerObject.transform.rotation.y += delta.x;
    viewerObject.transform.rotation.x -= delta.y;

    viewerObject.transform.rotation.y = glm::mod(viewerObject.transform.rotation.y, glm::two_pi<float>());
    viewerObject.transform.rotation.x = glm::clamp(viewerObject.transform.rotation.x, -1.5f, 1.5f);
  });

  lveCamera.setViewTarget(vec3(-1.f, -2.f, 2.f), vec3(0.f, 0.f, 2.5f));
  viewerObject.transform.translation.z = -2.5f;

  globalPool = LveDescriptorPool::Builder(lveDevice)
    .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
    .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
    .build();

  loadGameObjects();
}

FirstApp::~FirstApp() {}

void FirstApp::run() {
  using namespace std::chrono;

  std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (auto& buf : uboBuffers) {
    buf = std::make_unique<LveBuffer>(
      lveDevice,
      sizeof(GlobalUbo),
      1,
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    );
    buf->map();
  }

  auto globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
    .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
    .build();

  std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (size_t i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    LveDescriptorWriter(*globalSetLayout, *globalPool)
      .writeBuffer(0, &bufferInfo)
      .build(globalDescriptorSets[i]);
  }

  SimpleRenderSystem simpleRenderSystem(lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
  KeyboardMovementController cameraController{};
  auto currTime = high_resolution_clock::now();

  while (!lveWindow.shouldClose()) {
    glfwPollEvents();

    auto extent = lveWindow.getExtent();
    glfwSetCursorPos(lveWindow.getGFLWwindow(), extent.width * 0.5f, extent.height * 0.5f);

    auto newTime = high_resolution_clock::now();
    float frameTime = duration<float, seconds::period>(newTime - currTime).count();
    currTime = newTime;

    cameraController.moveInPlainXZ(lveWindow.getGFLWwindow(), frameTime, viewerObject);
    lveCamera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

    float aspect = lveRenderer.getAspectRatio();
    lveCamera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

    if (auto commandBuffer = lveRenderer.beginFrame()) {
      int frameIndex = lveRenderer.getFrameIndex();
      FrameInfo frameInfo{
        .frameIndex = frameIndex,
        .frameTime = frameTime,
        .commandBuffer = commandBuffer,
        .camera = lveCamera,
        .globalDescriptorSet = globalDescriptorSets[frameIndex],
        .gameObjects = gameObjects
      };

      // ----- Update ------------------------------------------------------------------------------------------------------------------ //

      GlobalUbo ubo{};
      ubo.projectionView = lveCamera.getProjectioinMatrix() * lveCamera.getViewMatrix();
      uboBuffers[frameIndex]->writeToBuffer(&ubo);
      uboBuffers[frameIndex]->flush();

      // ----- Render ------------------------------------------------------------------------------------------------------------------ //

      // begin offscreen shadow pass
      // render shadow casting objects
      // end offscreen shadow pass

      lveRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(frameInfo);
      lveRenderer.endSwapChainRenderPass(commandBuffer);
      lveRenderer.endFrame();

      // --------------------------------------------------------------------------------------------------------------------------------- //
    }
  }

  vkDeviceWaitIdle(lveDevice.device());
}

void FirstApp::loadGameObjects() {
  std::shared_ptr<LveModel> lveModel = LveModel::createModelFromOBJ(lveDevice, "res/obj/flat_vase.obj");
  auto flatVase = LveGameObject::createGameObject();
  flatVase.model = lveModel;
  flatVase.transform.translation = {-0.5f, 0.5f, 0.f};
  flatVase.transform.scale = vec3(3.f, 1.5f, 3.f);
  gameObjects.emplace(flatVase.getId(), std::move(flatVase));

  lveModel = LveModel::createModelFromOBJ(lveDevice, "res/obj/smooth_vase.obj");
  auto smoothVase = LveGameObject::createGameObject();
  smoothVase.model = lveModel;
  smoothVase.transform.translation = {0.5f, 0.5f, 0.f};
  smoothVase.transform.scale = vec3(3.f, 1.5f, 3.f);
  gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

  lveModel = LveModel::createModelFromOBJ(lveDevice, "res/obj/quad.obj");
  auto floor = LveGameObject::createGameObject();
  floor.model = lveModel;
  floor.transform.translation = {0.f, 0.5f, 0.f};
  floor.transform.scale = vec3(3.f, 1.f, 3.f);
  gameObjects.emplace(floor.getId(), std::move(floor));
}

} // namespace lve

