#include "first_app.hpp"

#include "simple_render_system.hpp"

namespace lve {

namespace {

// temporary helper function, creates a 1x1x1 cube centered at offset
std::unique_ptr<LveModel> createCubeModel(LveDevice& device, glm::vec3 offset) {
  std::vector<LveModel::Vertex> vertices{
    // left face (white)
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

      // right face (yellow)
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

      // top face (orange, remember y axis points down)
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

      // bottom face (red)
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

      // nose face (blue)
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

      // tail face (green)
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
  };

  for (auto& v : vertices)
    v.position += offset;

  return std::make_unique<LveModel>(device, vertices);
}

} // namespace

FirstApp::FirstApp() {
  loadGameObjects();
}

FirstApp::~FirstApp() {}

void FirstApp::run() {
  SimpleRenderSystem simpleRenderSystem(lveDevice, lveRenderer.getSwapChainRenderPass());
  LveCamera camera{};
  // camera.setViewDirection(vec3(0.f), glm::normalize(vec3(0.5f, 0.f, 1.f)));
  // camera.setViewTarget(vec3(-1.f, -2.f, 2.f), vec3(0.f, 0.f, 2.5f));

  size_t i = 0;
  while (!lveWindow.shouldClose()) {
    glfwPollEvents();

    float a = glm::pi<float>() * fabs(sin(i++ * 0.0005f)) * 0.1f;
    float aspect = lveRenderer.getAspectRatio();
    // camera.setOrthographicProjection(-aspect, aspect, 1, -1, -1, 1);
    camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);
    camera.setViewYXZ(vec3(-0.f, -0.f, 0.f), vec3(a, 0, 0));

    if (auto commandBuffer = lveRenderer.beginFrame()) {
      // ----- Update system ----------------------------------------------------------------------------------------------------------- //


      // ----- Render system ----------------------------------------------------------------------------------------------------------- //

      // begin offscreen shadow pass
      // render shadow casting objects
      // end offscreen shadow pass

      lveRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
      lveRenderer.endSwapChainRenderPass(commandBuffer);
      lveRenderer.endFrame();

      // --------------------------------------------------------------------------------------------------------------------------------- //
    }
  }

  vkDeviceWaitIdle(lveDevice.device());
}

void FirstApp::loadGameObjects() {
  std::shared_ptr<LveModel> lveModel = createCubeModel(lveDevice, {0.f, 0.f, 0.f});

  auto cube = LveGameObject::createGameObject();
  cube.model = lveModel;
  cube.transform.translation = {0.f, 0.f, 2.5f};
  cube.transform.scale = vec3(0.5f);

  gameObjects.push_back(std::move(cube));
}

} // namespace lve

