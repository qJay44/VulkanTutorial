#pragma once

#include "VulkanBufferAndMemory.hpp"
#include "VulkanGraphicsPipeline2.hpp"
#include "model_desc.hpp"
#include "VulkanTexture.hpp"
#include "../model/CoreModel.hpp"

namespace lve {

class VulkanCore;

class VulkanModel : public CoreModel<VulkanTexture> {
public:
	VulkanModel() {}

	void init(VulkanCore* core);
	void createDescriptorSets(VulkanGraphicsPipeline2& Pipeline);
	void recordCommandBuffer(VkCommandBuffer cmdBuf, VulkanGraphicsPipeline2& pipeline, u32 imageIndex);
  void addTexture(VulkanTexture* tex);
	void addMesh(std::vector<Vertex>& vertices, std::vector<u32>& indices);
	void update(u32 imageIndex, const mat4& transformation);

	const VulkanBufferAndMemory* getVB() const;
	const VulkanBufferAndMemory* getIB() const;

	void destroy();

private:
	VulkanCore* m_core = NULL;

	VulkanBufferAndMemory m_vb;
	VulkanBufferAndMemory m_ib;
	std::vector<VulkanBufferAndMemory> m_uniformBuffers;
	std::vector<std::vector<VkDescriptorSet>> m_descriptorSets;
	size_t m_vertexSize = 0;	// sizeof(Vertex) OR sizeof(SkinnedVertex)

private:
	void updateModelDesc(ModelDesc& md);
};

} // namespace lve

