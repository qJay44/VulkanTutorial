#pragma once

namespace lve::utils {

#define CHECK_VK_RESULT(res, msg) \
  if (res != VK_SUCCESS) \
    error("\n[CHECK_VK_RESULT] Error in [{}:{}]: \n[\n\n{}\n\n], code {}\n", __FILE__, __LINE__,  msg, (s32)res);

VkExtent2D GetWindowExtent(GLFWwindow* window);

u32 GetMemoryTypeIndex(
  const VkPhysicalDeviceMemoryProperties& physDeviceMemProps,
  u32 memTypeBitsMask,
  VkMemoryPropertyFlags reqMemPropFlags
);

bool HasStencilComponent(VkFormat format);

VkFormat FindSupportedFormat(
  VkPhysicalDevice physDevice,
  const std::vector<VkFormat>& candidates,
  VkImageTiling tiling,
  VkFormatFeatureFlags features
);

u32 GetBytesPerTexFormat(VkFormat format);

} // namespace lve::utils

