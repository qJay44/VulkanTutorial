#pragma once

#include <string>
#include <vector>
#include "lve_device.hpp"

namespace lve {
  struct PipelineConfigInfo {};

  class LvePipeline {
    public:
      LvePipeline(
          LveDevice &device,
          const std::string &vertFilepath,
          const std::string &fragFilepath,
          const PipelineConfigInfo &configInfo
      );
      ~LvePipeline() {}

      LvePipeline(const LvePipeline&) = delete;
      void operator=(const LvePipeline&) = delete;

      static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t widht, uint32_t height);

    private:
      static std::vector<char> readFile(const std::string &filepath);
      LveDevice &lveDevice;
      VkPipeline graphicsPipeline;
      VkShaderModule vertShaderModule;
      VkShaderModule fragShaderModule;

      void createGraphicsPipeline(
          const std::string &vertFilepath,
          const std::string &fragFilepath,
          const PipelineConfigInfo &configInfo
      );

      void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);
  };
}

