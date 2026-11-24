#pragma once

namespace lve {

struct VulkanBufferAndMemory {
  VkBuffer m_buffer = nullptr;
  VkDeviceMemory m_mem = nullptr;
  VkDeviceSize m_allocationSize = 0;

  static VulkanBufferAndMemory createBuffer(
    VkDevice device,
    const VkPhysicalDeviceMemoryProperties& physDeviceMemProps,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties
  );

  void update(VkDevice device, const void* data, size_t size);

  void destroy(VkDevice device);
};

} // namespace lve

