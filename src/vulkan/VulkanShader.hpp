#pragma once

#include <glslang/Include/glslang_c_shader_types.h>

namespace lve {

struct VulkanShader {
  VulkanShader() = default;

  VkShaderModule shaderModule = nullptr;

  void createShaderModuleFromBinary(VkDevice& device, const fspath& path);
  void createShaderModuleFromText(VkDevice& device, const fspath& path);

  void destroy(VkDevice device);
};

} // namespace lve

