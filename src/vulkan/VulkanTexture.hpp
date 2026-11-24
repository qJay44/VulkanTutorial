#pragma once

namespace lve {

struct VulkanTexture {
  VulkanTexture(class VulkanCore* core);

  VulkanCore* m_core = nullptr;
  VkImage m_image         = VK_NULL_HANDLE;
  VkImageView m_imageView = VK_NULL_HANDLE;
  VkDeviceMemory m_mem    = VK_NULL_HANDLE;
  VkSampler m_sampler     = VK_NULL_HANDLE;

  void load(const fspath& path);
  void load(u32 bufferSize, void* data);
  void destroy();
};

} // namespace lve

