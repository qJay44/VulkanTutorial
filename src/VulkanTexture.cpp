#include "VulkanTexture.hpp"

#include "VulkanCore.hpp"
#include "stb/stb_image.h"

namespace lve {

VulkanTexture::VulkanTexture(class VulkanCore *core) : m_core(core) {}

void VulkanTexture::load(const fspath& path) {
  assert(m_core);
  m_core->createTexture(path, this);
}

void VulkanTexture::destroy() {
  assert(m_core);

  if (m_sampler   != VK_NULL_HANDLE) vkDestroySampler(m_core->m_device, m_sampler, nullptr);
  if (m_imageView != VK_NULL_HANDLE) vkDestroyImageView(m_core->m_device, m_imageView, nullptr);
  if (m_image     != VK_NULL_HANDLE) vkDestroyImage(m_core->m_device, m_image, nullptr);
  if (m_mem       != VK_NULL_HANDLE) vkFreeMemory(m_core->m_device, m_mem, nullptr);

  m_sampler   = VK_NULL_HANDLE;
  m_imageView = VK_NULL_HANDLE;
  m_image     = VK_NULL_HANDLE;
  m_mem       = VK_NULL_HANDLE;
}

} // namespace lve

