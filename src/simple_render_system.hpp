#pragma once

#include "lve_frame_info.hpp"
#include "lve_game_object.hpp"
#include "lve_pipeline.hpp"
#include "lve_device.hpp"

namespace lve {

class SimpleRenderSystem {
public:
  SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass);
  ~SimpleRenderSystem();

  SimpleRenderSystem(const SimpleRenderSystem&) = delete;
  SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

  void renderGameObjects(FrameInfo& frameInfo, std::vector<LveGameObject>& gameObjects);

private:
  LveDevice& lveDevice;

  std::unique_ptr<LvePipeline> lvePipeline;
  VkPipelineLayout pipelineLayout;

private:
  void createPipelineLayout();
  void createPipeline(VkRenderPass renderPass);
};

} // namespace lve

