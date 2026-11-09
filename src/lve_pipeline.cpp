#include "lve_pipeline.hpp"
#include "utils/utils.hpp"
#include <vulkan/vulkan_core.h>

namespace lve {

LvePipeline::LvePipeline(
  LveDevice &device,
  const PipelineConfigInfo &configInfo,
  const fspath& vertPath,
  const fspath& fragPath
) : lveDevice(device)

{
  createGraphicsPipeline(configInfo, vertPath, fragPath);
}

LvePipeline::~LvePipeline() {

}

PipelineConfigInfo LvePipeline::defaultPipelineConfigInfo(u32 width, u32 height) {
 return PipelineConfigInfo{};
}

void LvePipeline::createGraphicsPipeline(const PipelineConfigInfo& configInfo, fspath vertPath, fspath fragPath) {
  vertPath = "Build/spv" / vertPath; vertPath += ".spv";
  fragPath = "Build/spv" / fragPath; fragPath += ".spv";

  auto vertCode = readFile(vertPath);
  auto fragCode = readFile(fragPath);

  std::println("Vert code size: {}", vertCode.size());
  std::println("Frag code size: {}", fragCode.size());
}

void LvePipeline::createShaderModule(const std::string& code, VkShaderModule* shaderModule) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const u32*>(code.data());

  if (vkCreateShaderModule(lveDevice.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
    error("[LvePipeline::createShaderModule] Failed to create shader module");
}

} // namespace lve

