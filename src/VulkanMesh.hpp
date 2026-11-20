#pragma once

#include "VulkanBufferAndMemory.hpp"

namespace lve {

struct VulkanMesh {
  VulkanBufferAndMemory m_vb;
  size_t m_vertexBufferSize = 0;

  void destroy(VkDevice device) {
    m_vb.destroy(device);
  }
};

} // namespace lve

