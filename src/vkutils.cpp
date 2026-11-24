namespace lve::utils {

#define CHECK_VK_RESULT(res, msg) \
  if (res != VK_SUCCESS) \
    error("\n[CHECK_VK_RESULT] Error in [{}:{}]: \n[\n\n{}\n\n], code {}\n", __FILE__, __LINE__,  msg, (s32)res);

VkExtent2D GetWindowExtent(GLFWwindow* window) {
  int width, height;
  VkExtent2D windowExtent{};
  glfwGetWindowSize(window, &width, &height);

  windowExtent.width = static_cast<u32>(width);
  windowExtent.height = static_cast<u32>(height);

  return windowExtent;
}

u32 GetMemoryTypeIndex(
  const VkPhysicalDeviceMemoryProperties& physDeviceMemProps,
  u32 memTypeBitsMask,
  VkMemoryPropertyFlags reqMemPropFlags
) {
  for (u32 i = 0; i < physDeviceMemProps.memoryTypeCount; i++) {
    const VkMemoryType& memType = physDeviceMemProps.memoryTypes[i];
    u32 currBitMask = 1 << i;
    bool isCurrMemTypeSupported = memTypeBitsMask & currBitMask;
    bool hasRequiredProps = (memType.propertyFlags & reqMemPropFlags) == reqMemPropFlags;

    if (isCurrMemTypeSupported && hasRequiredProps)
      return i;
  }

  error("[GetMemoryTypeIndex] Can't find memory type for type [{:#x}], requested mem props: [{:#x}]", memTypeBitsMask, reqMemPropFlags);
  __builtin_unreachable();
}

bool HasStencilComponent(VkFormat format) {
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat FindSupportedFormat(
  VkPhysicalDevice physDevice,
  const std::vector<VkFormat>& candidates,
  VkImageTiling tiling,
  VkFormatFeatureFlags features
) {
  for (const VkFormat& format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physDevice, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features))
      return format;
    else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features))
      return format;
  }

  error("[FindSupportedFormat] Failed to find supported format");
  __builtin_unreachable();
}

u32 GetBytesPerTexFormat(VkFormat format) {
  switch (format) {
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_SNORM:
    case VK_FORMAT_R8G8B8A8_SRGB : return 4;
    default:
      error("[GetBytesPerTexFormat] Unexpected format [{}]", (s32)format);
  }

  __builtin_unreachable();
}

} // namespace lve::utils

