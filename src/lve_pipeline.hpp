#pragma once

#include "lve_device.hpp"
#include "utils/types.hpp"

namespace lve {

struct PipelineConfigInfo {

};

class LvePipeline {
public:
  LvePipeline(
    LveDevice& device,
    const PipelineConfigInfo& configInfo,
    const fspath& vertPath,
    const fspath& fragPath
  );

  ~LvePipeline();

  LvePipeline(const LvePipeline&) = delete;
  LvePipeline& operator=(const LvePipeline&) = delete;

  static PipelineConfigInfo defaultPipelineConfigInfo(u32 width, u32 height);

private:
  LveDevice& lveDevice;
  VkPipeline graphicsPipeline;
  VkShaderModule vertShaderModule;
  VkShaderModule fragShaderModule;

private:
  void createGraphicsPipeline(const PipelineConfigInfo& configInfo, fspath vertPath, fspath fragPath);
  void createShaderModule(const std::string& code, VkShaderModule* shaderModule);
};

}; // namespace lve

