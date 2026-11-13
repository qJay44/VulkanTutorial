#pragma once

#include "lve_camera.hpp"
#include "lve_game_object.hpp"
#include "lve_renderer.hpp"
#include "lve_window.hpp"
#include "lve_device.hpp"

namespace lve {

class FirstApp {
public:
  FirstApp();
  ~FirstApp();

  FirstApp(const FirstApp&) = delete;
  FirstApp& operator=(const FirstApp&) = delete;

  void onMouseMove(GLFWwindow* window, double x, double y);

  void run();

private:
  LveWindow lveWindow{1600, 900, "MyProgram"};
  LveDevice lveDevice{lveWindow};
  LveRenderer lveRenderer{lveWindow, lveDevice};
  LveCamera lveCamera{};

  std::vector<LveGameObject> gameObjects;
  LveGameObject viewerObject = LveGameObject::createGameObject();

private:
  void loadGameObjects();
};

} // namespace lve

