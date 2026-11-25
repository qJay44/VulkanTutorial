#pragma once

#include "VulkanBufferAndMemory.hpp"
#include "VulkanTexture.hpp"
#include "model_desc.hpp"
#include "../model/CoreModel.hpp"

namespace lve {

class VulkanCore;

class VulkanModel : public CoreModel {
public:
	VulkanModel() {}

	void Destroy();

	void Init(VulkanCore* pVulkanCore) { m_pVulkanCore = pVulkanCore; }

	void CreateDescriptorSets(GraphicsPipelineV2& Pipeline);

	void RecordCommandBuffer(VkCommandBuffer CmdBuf, GraphicsPipelineV2& pPipeline, int ImageIndex);

	void Update(int ImageIndex, const glm::mat4& Transformation);

	const VulkanBufferAndMemory* GetVB() const { return &m_vb; }

	const VulkanBufferAndMemory* GetIB() const { return &m_ib; }

protected:

	virtual void AllocBuffers() { /* Nothing to do here */ }

	virtual Texture* AllocTexture2D();

	virtual void InitGeometryPost() { /* Nothing to do here */ }

	virtual void PopulateBuffersSkinned(vector<SkinnedVertex>& Vertices) { assert(0); }

	virtual void PopulateBuffers(vector<Vertex>& Vertices);

private:
	void UpdateModelDesc(ModelDesc& md);

	VulkanCore* m_pVulkanCore = NULL;

	VulkanBufferAndMemory m_vb;
	VulkanBufferAndMemory m_ib;
	std::vector<VulkanBufferAndMemory> m_uniformBuffers;
	std::vector<std::vector<VkDescriptorSet>> m_descriptorSets;
	size_t m_vertexSize = 0;	// sizeof(Vertex) OR sizeof(SkinnedVertex)
};

} // namespace lve

