#pragma once

#include "lve_game_object.hpp"
#include "lve_renderer.hpp"
#include "lve_window.hpp"
#include "lve_device.hpp"

namespace lve {

class FirstApp {
public:
  static constexpr int WINDOW_WIDTH  = 800;
  static constexpr int WINDOW_HEIGHT = 600;
  static constexpr std::string WINDOW_NAME = "MyProgram";

  FirstApp();
  ~FirstApp();

  FirstApp(const FirstApp&) = delete;
  FirstApp& operator=(const FirstApp&) = delete;

  void run();

private:
  LveWindow lveWindow{WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME};
  LveDevice lveDevice{lveWindow};
  LveRenderer lveRenderer{lveWindow, lveDevice};

  std::vector<LveGameObject> gameObjectsTriangles;
  std::vector<LveGameObject> gameObjectsPhysicsObjects;
  std::vector<LveGameObject> gameObjectsVectorField;

private:
  void loadGameObjects();
};

} // namespace lve

