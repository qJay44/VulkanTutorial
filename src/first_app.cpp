#include "first_app.hpp"
#include "simple_render_system.hpp"

namespace lve {

FirstApp::FirstApp() {
  loadGameObjects();
}

FirstApp::~FirstApp() {}

void FirstApp::run() {
  SimpleRenderSystem simpleRenderSystem(lveDevice, lveRenderer.getSwapChainRenderPass());

  while (!lveWindow.shouldClose()) {
    glfwPollEvents();

    if (auto commandBuffer = lveRenderer.beginFrame()) {
      // begin offscreen shadow pass
      // render shadow casting objects
      // end offscreen shadow pass

      lveRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
      lveRenderer.endSwapChainRenderPass(commandBuffer);
      lveRenderer.endFrame();
    }
  }

  vkDeviceWaitIdle(lveDevice.device());
}

void FirstApp::loadGameObjects() {
  std::vector<LveModel::Vertex> vertices{
    {{ 0.0f, -0.5f}, {1.f, 0.f, 0.f,}},
    {{ 0.5f,  0.5f}, {0.f, 1.f, 0.f,}},
    {{-0.5f,  0.5f}, {0.f, 0.f, 1.f,}},
  };

  auto lveModel = std::make_shared<LveModel>(lveDevice, vertices);

  int cnt = 12;
  float step = 1.f / cnt;
  for (int i = 0; i < cnt; i++) {
    auto triangle = LveGameObject::createGameObject();
    int c = i % 3;
    triangle.model = lveModel;
    triangle.color = vec3{0.8f};
    triangle.color[c] *= 0.f;
    triangle.transform2d.translation = vec2{0.f};
    triangle.transform2d.rotation = 0.25f * glm::two_pi<float>() * i * step;
    triangle.transform2d.scale = vec2{1.f} * (i * step);


    gameObjects.push_back(std::move(triangle));
  }
}

} // namespace lve

