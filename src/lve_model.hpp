#pragma once

#include "lve_device.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>

namespace lve {
class LveModel {
  public:
    struct Vertex {
      glm::vec2 position;

      static std::vector<VkVertexInputBindingDescription> getBindingDescriptipons();
      static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptipons();
    };

    LveModel(LveDevice& device, const std::vector<Vertex>& vertices);
    ~LveModel();

    LveModel(const LveModel&) = delete;
    LveModel &operator=(const LveModel&) = delete;

    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);

  private:
    LveDevice& lveDevice;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    uint32_t vertexCount;

    void createVertexBuffers(const std::vector<Vertex>& vertices);
};
}  // namespace lve

