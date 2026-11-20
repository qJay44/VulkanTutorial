#include "VulkanBufferAndMemory.hpp"

namespace lve {

namespace {

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

} // namespace

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
  u32 memTypeIndex = GetMemoryTypeIndex(physDeviceMemProps, memReqs.memoryTypeBits, properties);

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

void VulkanBufferAndMemory::destroy(VkDevice device) {
  if (m_mem) vkFreeMemory(device, m_mem, nullptr);
  if (m_buffer) vkDestroyBuffer(device, m_buffer, nullptr);
}

} // namespace lve

