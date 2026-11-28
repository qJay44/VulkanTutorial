#pragma once

namespace lve::wrappers {

void beginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags usageFlags);
void endCommandBuffer(VkCommandBuffer commandBuffer);
VkSemaphore createSemaphore(VkDevice device);

void imageMemBarrier(
  VkCommandBuffer cmdBuf,
  VkImage image,
  VkFormat format,
  u32 layerCount,
  VkImageLayout oldLayout,
  VkImageLayout newLayout
);

VkImageView createImageView(
  VkDevice device,
  VkImage image,
  VkFormat format,
  VkImageAspectFlags aspectFlags,
  bool isCubemap
);

VkSampler createTextureSampler(VkDevice device, VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressMode);

} // namespace lve::wrappers

