#pragma once

namespace lve::wrappers {

void beginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags usageFlags);
VkSemaphore createSemaphore(VkDevice device);

} // namespace lve::wrappers

