#include "first_app.hpp"

#include "simple_render_system.hpp"

namespace lve {

namespace {

// temporary helper function, creates a 1x1x1 cube centered at offset
std::unique_ptr<LveModel> createCubeModel(LveDevice& device, glm::vec3 offset) {
  std::vector<LveModel::Vertex> vertices{
      // front face (blue)
      {{-0.5f, -0.5f,  0.5f}, {0.1f, 0.1f, 0.8f}},
      {{ 0.5f,  0.5f,  0.5f}, {0.1f, 0.1f, 0.8f}},
      {{-0.5f,  0.5f,  0.5f}, {0.1f, 0.1f, 0.8f}},
      {{-0.5f, -0.5f,  0.5f}, {0.1f, 0.1f, 0.8f}},
      {{ 0.5f, -0.5f,  0.5f}, {0.1f, 0.1f, 0.8f}},
      {{ 0.5f,  0.5f,  0.5f}, {0.1f, 0.1f, 0.8f}},

      // tail face (green)
      {{-0.5f, -0.5f, -0.5f}, {0.1f, 0.8f, 0.1f}},
      {{ 0.5f,  0.5f, -0.5f}, {0.1f, 0.8f, 0.1f}},
      {{-0.5f,  0.5f, -0.5f}, {0.1f, 0.8f, 0.1f}},
      {{-0.5f, -0.5f, -0.5f}, {0.1f, 0.8f, 0.1f}},
      {{ 0.5f, -0.5f, -0.5f}, {0.1f, 0.8f, 0.1f}},
      {{ 0.5f,  0.5f, -0.5f}, {0.1f, 0.8f, 0.1f}},

      // left face (white)
      {{-0.5f, -0.5f, -0.5f}, {0.9f, 0.9f, 0.9f}},
      {{-0.5f,  0.5f,  0.5f}, {0.9f, 0.9f, 0.9f}},
      {{-0.5f, -0.5f,  0.5f}, {0.9f, 0.9f, 0.9f}},
      {{-0.5f, -0.5f, -0.5f}, {0.9f, 0.9f, 0.9f}},
      {{-0.5f,  0.5f, -0.5f}, {0.9f, 0.9f, 0.9f}},
      {{-0.5f,  0.5f,  0.5f}, {0.9f, 0.9f, 0.9f}},

      // right face (yellow)
      {{ 0.5f, -0.5f, -0.5f}, {0.8f, 0.8f, 0.1f}},
      {{ 0.5f,  0.5f,  0.5f}, {0.8f, 0.8f, 0.1f}},
      {{ 0.5f, -0.5f,  0.5f}, {0.8f, 0.8f, 0.1f}},
      {{ 0.5f, -0.5f, -0.5f}, {0.8f, 0.8f, 0.1f}},
      {{ 0.5f,  0.5f, -0.5f}, {0.8f, 0.8f, 0.1f}},
      {{ 0.5f,  0.5f,  0.5f}, {0.8f, 0.8f, 0.1f}},

      // top face (orange, remember y axis points down)
      {{-0.5f, -0.5f, -0.5f}, {0.9f, 0.6f, 0.1f}},
      {{ 0.5f, -0.5f,  0.5f}, {0.9f, 0.6f, 0.1f}},
      {{-0.5f, -0.5f,  0.5f}, {0.9f, 0.6f, 0.1f}},
      {{-0.5f, -0.5f, -0.5f}, {0.9f, 0.6f, 0.1f}},
      {{ 0.5f, -0.5f, -0.5f}, {0.9f, 0.6f, 0.1f}},
      {{ 0.5f, -0.5f,  0.5f}, {0.9f, 0.6f, 0.1f}},

      // bottom face (red)
      {{-0.5f,  0.5f, -0.5f}, {0.8f, 0.1f, 0.1f}},
      {{ 0.5f,  0.5f,  0.5f}, {0.8f, 0.1f, 0.1f}},
      {{-0.5f,  0.5f,  0.5f}, {0.8f, 0.1f, 0.1f}},
      {{-0.5f,  0.5f, -0.5f}, {0.8f, 0.1f, 0.1f}},
      {{ 0.5f,  0.5f, -0.5f}, {0.8f, 0.1f, 0.1f}},
      {{ 0.5f,  0.5f,  0.5f}, {0.8f, 0.1f, 0.1f}},
      {{ 0.5f,  0.5f,  0.5f}, {0.1f, 0.1f, 0.8f}},
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

  while (!lveWindow.shouldClose()) {
    glfwPollEvents();

    if (auto commandBuffer = lveRenderer.beginFrame()) {
      // ----- Update system ----------------------------------------------------------------------------------------------------------- //


      // ----- Render system ----------------------------------------------------------------------------------------------------------- //

      // begin offscreen shadow pass
      // render shadow casting objects
      // end offscreen shadow pass

      lveRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
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
  cube.transform.translation = {0.f, 0.f, 0.5f};
  cube.transform.scale = vec3(0.5f);

  gameObjects.push_back(std::move(cube));
}

} // namespace lve

