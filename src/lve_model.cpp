#include "lve_model.hpp"

#include "lve_utils.hpp"
#include "glm/gtx/hash.hpp"
#include "tiny_obj_loader.h"

namespace {

struct VertexHash {
  size_t operator()(lve::LveModel::Vertex const& vertex) const {
    size_t seed = 0;
    lve::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
    return seed;
  }
};

} // namespace

namespace lve {

std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescriptions() {
  std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
  bindingDescriptions[0].binding = 0;
  bindingDescriptions[0].stride = sizeof(Vertex);
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescriptions() {
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions{
    {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) },
    {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)    },
    {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)   },
    {3, 0, VK_FORMAT_R32G32_SFLOAT   , offsetof(Vertex, uv)       },
  };

  return attributeDescriptions;
}

bool LveModel::Vertex::operator==(const Vertex& rhs) const {
  return
    position == rhs.position &&
    color    == rhs.color    &&
    normal   == rhs.normal   &&
    uv       == rhs.uv;
}

void LveModel::Data::loadFromOBJ(const fspath& path) {
  const auto& pathStr = path.string();

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, pathStr.c_str()))
    error("[LveModel::Data::loadFromOBJ] Can't load object [{}]\n\nWarning: [{}]\n\nError: [{}]", pathStr, warn, err);

  vertices.clear();
  indices.clear();

  std::unordered_map<Vertex, u32, VertexHash> uniqueVertices;

  for (const auto& shape : shapes) {
    for (const auto& index : shape.mesh.indices) {
      Vertex vertex{};

      if (index.vertex_index >= 0) {
        vertex.position = {
          attrib.vertices[3 * index.vertex_index + 0],
          attrib.vertices[3 * index.vertex_index + 1],
          attrib.vertices[3 * index.vertex_index + 2],
        };

        vertex.color = {
          attrib.colors[3 * index.vertex_index + 0],
          attrib.colors[3 * index.vertex_index + 1],
          attrib.colors[3 * index.vertex_index + 2],
        };
      }

      if (index.normal_index >= 0) {
        vertex.normal = {
          attrib.normals[3 * index.normal_index + 0],
          attrib.normals[3 * index.normal_index + 1],
          attrib.normals[3 * index.normal_index + 2],
        };
      }

      if (index.texcoord_index >= 0) {
        vertex.uv = {
          attrib.texcoords[2 * index.texcoord_index + 0],
          attrib.texcoords[2 * index.texcoord_index + 1],
        };
      }

      auto [it, inserted] = uniqueVertices.try_emplace(vertex, vertices.size());

      if (inserted)
        vertices.push_back(vertex);

      indices.push_back(it->second);
    }
  }

  dprint("\n[LveModel::Data::loadFromOBJ] Model [{}]\nVertices: [{}]\nIndices: [{}]\n", pathStr, vertices.size(), indices.size());
}

std::unique_ptr<LveModel> LveModel::createModelFromOBJ(LveDevice& device, const fspath& path) {
  Data data{};
  data.loadFromOBJ(path);

  return std::make_unique<LveModel>(device, data);
}

LveModel::LveModel(LveDevice& device, const Data& data) : lveDevice(device) {
  createVertexBuffers(data.vertices);
  createIndexBuffer(data.indices);
}

LveModel::~LveModel() {
  vkDestroyBuffer(lveDevice.device(), vertexBuffer, nullptr);
  vkFreeMemory(lveDevice.device(), vertexBufferMemory, nullptr);

  if (hasIndexBuffer) {
    vkDestroyBuffer(lveDevice.device(), indexBuffer, nullptr);
    vkFreeMemory(lveDevice.device(), indexBufferMemory, nullptr);
  }
}

void LveModel::bind(VkCommandBuffer commandBuffer) {
  VkBuffer buffers[] = {vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

  if (hasIndexBuffer)
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32); // NOTE: constant type
}

void LveModel::draw(VkCommandBuffer commandBuffer) {
  if (hasIndexBuffer) {
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
  } else {
    vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
  }
}

void LveModel::createVertexBuffers(const std::vector<Vertex>& vertices) {
  vertexCount = static_cast<u32>(vertices.size());
  assert(vertexCount > 2);
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  lveDevice.createBuffer(
    bufferSize,
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    stagingBuffer,
    stagingBufferMemory
  );

  void* data;
  vkMapMemory(lveDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(lveDevice.device(), stagingBufferMemory);

  lveDevice.createBuffer(
    bufferSize,
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    vertexBuffer,
    vertexBufferMemory
  );

  lveDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

  vkDestroyBuffer(lveDevice.device(), stagingBuffer, nullptr);
  vkFreeMemory(lveDevice.device(), stagingBufferMemory, nullptr);
}

void LveModel::createIndexBuffer(const std::vector<u32>& indices) {
  indexCount = static_cast<u32>(indices.size());

  if (!(hasIndexBuffer = indexCount > 0))
    return;

  VkDeviceSize bufferSize = sizeof(u32) * indexCount;
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  lveDevice.createBuffer(
    bufferSize,
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    stagingBuffer,
    stagingBufferMemory
  );

  void* data;
  vkMapMemory(lveDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(lveDevice.device(), stagingBufferMemory);

  lveDevice.createBuffer(
    bufferSize,
    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    indexBuffer,
    indexBufferMemory
  );

  lveDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

  vkDestroyBuffer(lveDevice.device(), stagingBuffer, nullptr);
  vkFreeMemory(lveDevice.device(), stagingBufferMemory, nullptr);
}

} // namespace lve

