#pragma once

#include "VulkanBufferAndMemory.hpp"
#include "VulkanTexture.hpp"

namespace lve {

struct VulkanMesh {
  VulkanBufferAndMemory m_vb;
  size_t m_vertexBufferSize = 0;

  VulkanTexture* m_tex = nullptr;

  void destroy(VkDevice device) {
    m_vb.destroy(device);

    if (m_tex) {
      m_tex->destroy();
      delete m_tex; m_tex = nullptr;
    }
  }
};

} // namespace lve

