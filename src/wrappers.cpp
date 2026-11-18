#include "wrappers.hpp"

namespace lve::wrappers {

void beginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags usageFlags) {
  VkCommandBufferBeginInfo beginInfo{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .pNext = nullptr,
    .flags = usageFlags,
    .pInheritanceInfo = nullptr,
  };

  VkResult res = vkBeginCommandBuffer(commandBuffer, &beginInfo);
  CHECK_VK_RESULT(res, "vkBeginCommandBuffer");
}

VkSemaphore createSemaphore(VkDevice device) {
  VkSemaphoreCreateInfo createInfo{
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
  };

  VkSemaphore sem;
  VkResult res = vkCreateSemaphore(device, &createInfo, nullptr, &sem);
  CHECK_VK_RESULT(res, "vkCreateSemaphore");

  return sem;
}

} // namespace lve::wrappers

