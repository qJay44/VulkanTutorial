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

} // namespace lve::wrappers

