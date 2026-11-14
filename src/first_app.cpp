#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "simple_render_system.hpp"

namespace lve {

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

  loadGameObjects();
}

FirstApp::~FirstApp() {}

void FirstApp::run() {
  using namespace std::chrono;

  SimpleRenderSystem simpleRenderSystem(lveDevice, lveRenderer.getSwapChainRenderPass());

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
      // ----- Update system ----------------------------------------------------------------------------------------------------------- //


      // ----- Render system ----------------------------------------------------------------------------------------------------------- //

      // begin offscreen shadow pass
      // render shadow casting objects
      // end offscreen shadow pass

      lveRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, lveCamera);
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
  flatVase.transform.translation = {-0.5f, 0.5f, 2.5f};
  flatVase.transform.scale = vec3(3.f, 1.5f, 3.f);
  gameObjects.push_back(std::move(flatVase));

  lveModel = LveModel::createModelFromOBJ(lveDevice, "res/obj/smooth_vase.obj");
  auto smoothVase = LveGameObject::createGameObject();
  smoothVase.model = lveModel;
  smoothVase.transform.translation = {0.5f, 0.5f, 2.5f};
  smoothVase.transform.scale = vec3(3.f, 1.5f, 3.f);
  gameObjects.push_back(std::move(smoothVase));
}

} // namespace lve

