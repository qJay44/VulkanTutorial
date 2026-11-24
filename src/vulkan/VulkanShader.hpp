#pragma once

#include <glslang/Include/glslang_c_shader_types.h>

namespace lve {

struct VulkanShader {
  VulkanShader() = default;

  // NOTE: Destroy this from outside!
  VkShaderModule shaderModule = nullptr;
  // Add live update, like in OpenGL?

  void createShaderModuleFromBinary(VkDevice& device, const fspath& path);
  void createShaderModuleFromText(VkDevice& device, const fspath& path);
};

} // namespace lve

