#include "VulkanTexture.hpp"

#include "VulkanCore.hpp"
#include "stb/stb_image.h"

namespace lve {

VulkanTexture::VulkanTexture(class VulkanCore *core) : m_core(core) {}

void VulkanTexture::load(const fspath& path) {
  assert(m_core);
  m_core->createTexture2D(path, this);
}

void VulkanTexture::load(u32 bufferSize, void* data) {
  assert(m_core);

  int imgWidth, imgHeight, imgChannels;
  void* pixels = stbi_load_from_memory((const stbi_uc*)data, bufferSize, &imgWidth, &imgHeight, &imgChannels, 0);

  VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
  VkExtent3D imageExtent{
    .width = static_cast<u32>(imgWidth),
    .height = static_cast<u32>(imgHeight),
    .depth = 1
  };

  m_core->createTextureFromData(pixels, format, imageExtent, this, false);
}

void VulkanTexture::loadEctCubemap(const fspath& path) {
  assert(m_core);
  m_core->createTextureCubemap(path, this);
}

void VulkanTexture::destroy(VkDevice device) {
  if (m_sampler   != VK_NULL_HANDLE) vkDestroySampler(device, m_sampler, nullptr);
  if (m_imageView != VK_NULL_HANDLE) vkDestroyImageView(device, m_imageView, nullptr);
  if (m_image     != VK_NULL_HANDLE) vkDestroyImage(device, m_image, nullptr);
  if (m_mem       != VK_NULL_HANDLE) vkFreeMemory(device, m_mem, nullptr);

  m_sampler   = VK_NULL_HANDLE;
  m_imageView = VK_NULL_HANDLE;
  m_image     = VK_NULL_HANDLE;
  m_mem       = VK_NULL_HANDLE;
}

} // namespace lve

