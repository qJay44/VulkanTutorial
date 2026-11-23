#include "VulkanTexture.hpp"

#include "VulkanCore.hpp"
#include "VulkanBufferAndMemory.hpp"
#include "stb/stb_image.h"
#include "wrappers.hpp"

namespace lve {

namespace {

u32 GetBytesPerTexFormat(VkFormat format) {
  switch (format) {
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_SNORM:
    case VK_FORMAT_R8G8B8A8_SRGB : return 4;
    default:
      error("[GetBytesPerTexFormat] Unexpected format [{}]", (s32)format);
  }

  __builtin_unreachable();
}

} // namespace

void VulkanTexture::create(VulkanCore* core, const fspath& path) {
  m_core = core;

  int imageWidth = 0;
  int imageHeight = 0;
  int imageChannels = 0;

  // Step 1: load the image pixels
  stbi_uc* pixels = stbi_load(path.string().c_str(), &imageWidth, &imageHeight, &imageChannels, STBI_rgb_alpha);
  if (!pixels)
    error("[VulkanTexture::createTexture] Can't load [{}]", path.string());

  VkExtent3D imageExtent = {
    .width = static_cast<u32>(imageWidth),
    .height = static_cast<u32>(imageHeight),
    .depth = 1,
  };

  // Step 2: create the image object and populate it with pixels
  VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
  createTextureImageData(pixels, imageExtent, format);

  // Step 3: release the image pixels
  stbi_image_free(pixels);

  // Step 4: create the image view
  VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
  m_imageView = wrappers::createImageView(m_core->m_device, m_image, format, aspectFlags);

  VkFilter magFilter = VK_FILTER_LINEAR;
  VkFilter minFilter = VK_FILTER_LINEAR;
  VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

  // Step 5: create the texture sampler
  m_sampler = wrappers::createTextureSampler(m_core->m_device, magFilter, minFilter, addressMode);

  dprint("Created texture from [{}]", path.string());
}

void VulkanTexture::destroy() {
  vkDestroySampler(m_core->m_device, m_sampler, nullptr);
  vkDestroyImageView(m_core->m_device, m_imageView, nullptr);
  vkDestroyImage(m_core->m_device, m_image, nullptr);
  vkFreeMemory(m_core->m_device, m_mem, nullptr);
}

void VulkanTexture::transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
  wrappers::beginCommandBuffer(m_core->m_copyCmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  wrappers::imageMemBarrier(m_core->m_copyCmdBuf, m_image, format, oldLayout, newLayout);
  m_core->submitCopyCommand();
}

void VulkanTexture::createTextureImageData(const void* pixels, VkExtent3D extent, VkFormat format) {
  auto usage = (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                      VK_IMAGE_USAGE_SAMPLED_BIT);
  VkMemoryPropertyFlagBits propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  createImage(extent, format, usage, propertyFlags);
  updateTextureImage(extent, format, pixels);
}

void VulkanTexture::createImage(
  VkExtent3D extent,
  VkFormat format,
  VkImageUsageFlagBits usage,
  VkMemoryPropertyFlagBits propertyFlags
) {
  const auto& device = m_core->m_device;
  const auto& physDeviceMemProps = m_core->m_physDevices.selected().m_memProps;

  VkImageCreateInfo imgInfo{
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = format,
    .extent = extent,
    .mipLevels = 1,
    .arrayLayers = 1,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .usage = usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0,
    .pQueueFamilyIndices = nullptr,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  // Step 1: create the image object
  VkResult res = vkCreateImage(device, &imgInfo, nullptr, &m_image);
  CHECK_VK_RESULT(res, "vkCreateImage");

  // Step 2: get the buffer memory requirements
  VkMemoryRequirements memReqs{};
  vkGetImageMemoryRequirements(device, m_image, &memReqs);

  // Step 3: get the memory type index
  u32 memTypeIndex = GetMemoryTypeIndex(physDeviceMemProps, memReqs.memoryTypeBits, propertyFlags);

  // Step 4: allocate memory
  VkMemoryAllocateInfo memAllocInfo{
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .pNext = nullptr,
    .allocationSize = memReqs.size,
    .memoryTypeIndex = memTypeIndex,
  };
  res = vkAllocateMemory(device, &memAllocInfo, nullptr, &m_mem);
  CHECK_VK_RESULT(res, "vkAllocateMemory");

  // Step 5: bind memory
  res = vkBindImageMemory(device, m_image, m_mem, 0);
  CHECK_VK_RESULT(res, "vkBindImageMemory");
}

void VulkanTexture::updateTextureImage(
  VkExtent3D extent,
  VkFormat format,
  const void* pixels
) {
  const auto& device = m_core->m_device;
  const auto& physDeviceMemProps = m_core->m_physDevices.selected().m_memProps;

  u32 bytesPerPixel = GetBytesPerTexFormat(format);
  u32 layerCount = 1;
  VkDeviceSize layerSize = extent.width * extent.height * bytesPerPixel;
  VkDeviceSize imageSize = layerSize * layerCount;

  VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

  auto stagingTex = VulkanBufferAndMemory::createBuffer(device, physDeviceMemProps, imageSize, usage, properties);
  stagingTex.update(device, pixels, imageSize);

  transitionImageLayout(format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  m_core->copyBufferToImage(stagingTex.m_buffer, m_image, extent);
  transitionImageLayout(format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  stagingTex.destroy(device);
}

} // namespace lve

