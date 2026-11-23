#pragma once

namespace lve::wrappers {

void beginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags usageFlags);
VkSemaphore createSemaphore(VkDevice device);

void imageMemBarrier(
  VkCommandBuffer cmdBuf,
  VkImage image,
  VkFormat format,
  VkImageLayout oldLayout,
  VkImageLayout newLayout
);

VkImageView createImageView(
  VkDevice device,
  VkImage image,
  VkFormat format,
  VkImageAspectFlags aspectFlags
);

VkSampler createTextureSampler(VkDevice device, VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressMode);

} // namespace lve::wrappers

