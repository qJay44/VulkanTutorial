#pragma once

namespace lve {

struct VulkanTexture {
  VulkanTexture(class VulkanCore* core); // NOTE: I don't like this

  VulkanCore* m_core = nullptr;
  VkImage m_image = VK_NULL_HANDLE;
  VkImageView m_imageView = VK_NULL_HANDLE;
  VkDeviceMemory m_mem = VK_NULL_HANDLE;
  VkSampler m_sampler = VK_NULL_HANDLE;

  void load(const fspath& path);
  void destroy();

  void transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
  void createTextureImageData(const void* pixels, VkExtent3D extent, VkFormat format);
  void createImage(VkExtent3D extent, VkFormat format, VkImageUsageFlagBits usage, VkMemoryPropertyFlagBits propertyFlags);
  void updateTextureImage(VkExtent3D extent, VkFormat format, const void* pixels);
};

} // namespace lve

