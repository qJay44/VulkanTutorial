#pragma once

#include "lve_window.hpp"
#include "lve_pipeline.hpp"
#include "lve_device.hpp"

namespace lve {

class FirstApp {
public:
  static constexpr int WINDOW_WIDTH  = 800;
  static constexpr int WINDOW_HEIGHT = 600;
  static constexpr std::string WINDOW_NAME = "MyProgram";

  void run();
private:
  LveWindow lveWindow{WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME};
  LveDevice lveDevice{lveWindow};
  LvePipeline lvePipeline{lveDevice, LvePipeline::defaultPipelineConfigInfo(WINDOW_WIDTH, WINDOW_HEIGHT), "simple_shader.vert", "simple_shader.frag"};
};

} // namespace lve

