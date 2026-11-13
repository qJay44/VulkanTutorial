#pragma once

#include "lve_device.hpp"

namespace lve {

class LveModel {
public:
  struct Vertex {
    vec3 position;
    vec3 color{};
    vec3 normal{};
    vec2 uv{};

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

    bool operator==(const Vertex& rhs) const;
  };

  struct Data {
    std::vector<Vertex> vertices{};
    std::vector<u32> indices{};

    void loadFromOBJ(const fspath& path);
  };

  static std::unique_ptr<LveModel> createModelFromOBJ(LveDevice& device, const fspath& path);

  LveModel(LveDevice& device, const Data& data);
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

  // TODO: Implement std::variant?
  bool hasIndexBuffer = false;
  VkBuffer indexBuffer;
  VkDeviceMemory indexBufferMemory;
  u32 indexCount;

private:
  void createVertexBuffers(const std::vector<Vertex>& vertices);
  void createIndexBuffer(const std::vector<u32>& indices);
};

} // namespace lve

