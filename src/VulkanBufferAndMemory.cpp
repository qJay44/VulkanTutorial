#include "VulkanBufferAndMemory.hpp"

namespace lve {

VulkanBufferAndMemory VulkanBufferAndMemory::createBuffer(
  VkDevice device,
  const VkPhysicalDeviceMemoryProperties& physDeviceMemProps,
  VkDeviceSize size,
  VkBufferUsageFlags usage,
  VkMemoryPropertyFlags properties
) {
  VkBufferCreateInfo bufCreateInfo{};
  bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufCreateInfo.size = size;
  bufCreateInfo.usage = usage;
  bufCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VulkanBufferAndMemory buf;

  // Step 1: create a buffer
  VkResult res = vkCreateBuffer(device, &bufCreateInfo, nullptr, &buf.m_buffer);
  CHECK_VK_RESULT(res, "vkCreateBuffer");

  // Step 2: get the buffer memory requirements
  VkMemoryRequirements memReqs{};
  vkGetBufferMemoryRequirements(device, buf.m_buffer, &memReqs);
  buf.m_allocationSize = memReqs.size;

  // Step 3: get the memory type index
  u32 memTypeIndex = utils::GetMemoryTypeIndex(physDeviceMemProps, memReqs.memoryTypeBits, properties);

  // Step 4: allocate memory
  VkMemoryAllocateInfo memAllocInfo{
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .pNext = nullptr,
    .allocationSize = memReqs.size,
    .memoryTypeIndex = memTypeIndex,
  };
  res = vkAllocateMemory(device, &memAllocInfo, nullptr, &buf.m_mem);
  CHECK_VK_RESULT(res, "vkAllocateMemory");

  // Step 5: bind memory
  res = vkBindBufferMemory(device, buf.m_buffer, buf.m_mem, 0);
  CHECK_VK_RESULT(res, "vkBindBufferMemory");

  return buf;
}

void VulkanBufferAndMemory::update(VkDevice device, const void* data, size_t size) {
  void* mem = nullptr;

  VkResult res = vkMapMemory(device, m_mem, 0, size, 0, &mem);
  CHECK_VK_RESULT(res, "vkMapMemory");

  memcpy(mem, data, size);
  vkUnmapMemory(device, m_mem);
}

void VulkanBufferAndMemory::destroy(VkDevice device) {
  if (m_mem) vkFreeMemory(device, m_mem, nullptr);
  if (m_buffer) vkDestroyBuffer(device, m_buffer, nullptr);
}

} // namespace lve

