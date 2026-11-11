#pragma once

#include "lve_device.hpp"

namespace lve {

class LveModel {
public:
  struct Vertex {
    vec2 position;
    vec3 color;

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
  };

  LveModel(LveDevice& device, const std::vector<Vertex>& vertices);
  ~LveModel();

  LveModel(const LveModel&) = delete;
  LveModel& operator=(const LveModel&) = delete;

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

private:
  LveDevice& lveDevice;
  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  u32 vertexCount;

private:
  void createVertexBuffers(const std::vector<Vertex>& vertices);
};

} // namespace lve

