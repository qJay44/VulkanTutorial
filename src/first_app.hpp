#pragma once

#include "lve_model.hpp"
#include "lve_swap_chain.hpp"
#include "lve_window.hpp"
#include "lve_pipeline.hpp"
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
  std::unique_ptr<LveSwapChain> lveSwapChain;
  std::unique_ptr<LvePipeline> lvePipeline;
  VkPipelineLayout pipelineLayout;
  std::vector<VkCommandBuffer> commandBuffers;
  std::unique_ptr<LveModel> lveModel;

private:
  void loadModels();
  void createPipelineLayout();
  void createPipeline();
  void createCommandBuffers();
  void freeCommandBuffers();
  void drawFrame();
  void recreateSwapChain();
  void recordCommandBuffer(int imageIndex);
};

} // namespace lve

