#pragma once

#include "../lve_frame_info.hpp"
#include "../lve_pipeline.hpp"
#include "../lve_device.hpp"

namespace lve {

class PointLightSystem {
public:
  PointLightSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
  ~PointLightSystem();

  PointLightSystem(const PointLightSystem&) = delete;
  PointLightSystem& operator=(const PointLightSystem&) = delete;

  void render(FrameInfo& frameInfo);

private:
  LveDevice& lveDevice;

  std::unique_ptr<LvePipeline> lvePipeline;
  VkPipelineLayout pipelineLayout;

private:
  void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
  void createPipeline(VkRenderPass renderPass);
};

} // namespace lve

