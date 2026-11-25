#include "VulkanModel.hpp"

#include "VulkanCore.hpp"

namespace lve {

#define UNIFORM_BUFFER_SIZE sizeof(glm::mat4)

void VulkanModel::init(VulkanCore* core) {
  m_core = core;
}

void VulkanModel::createDescriptorSets(VulkanGraphicsPipeline2& pipeline) {
	u32 numSubmeshes = m_meshes.size();
	pipeline.allocateDescriptorSets(numSubmeshes, m_descriptorSets);

	ModelDesc md;

	updateModelDesc(md);

	pipeline.updateDescriptorSets(md, m_descriptorSets);
}

void VulkanModel::recordCommandBuffer(VkCommandBuffer cmdBuf, VulkanGraphicsPipeline2& pipeline, u32 imageIndex) {
	u32 instanceCount = 1;
	u32 firstInstance = 0;
	u32 baseVertex = 0;

	for (u32 submeshIndex = 0; submeshIndex < m_meshes.size(); submeshIndex++) {
		vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
								pipeline.getPipelineLayout(),
								0,  // firstSet
								1,  // descriptorSetCount
								&m_descriptorSets[imageIndex][submeshIndex],
								0,	// dynamicOffsetCount
								NULL);	// pDynamicOffsets

		vkCmdDraw(cmdBuf, m_meshes[submeshIndex].numIndices, instanceCount, baseVertex, firstInstance);
	}
}

void VulkanModel::addTexture(VulkanTexture* tex) {
  u32 numSubmeshes = m_meshes.size();

	for (u32 submeshIndex = 0; submeshIndex < numSubmeshes; submeshIndex++) {
		m_meshes[submeshIndex].materialIndex = m_materials.size();
    m_materials.push_back({.diffuse = tex});
  }
}

void VulkanModel::addMesh(std::vector<Vertex>& vertices, std::vector<u32>& indices) {
  m_meshes.push_back({
    .numIndices = (u32)indices.size(),
    .numVertices = (u32)vertices.size(),
  });

	m_vb = m_core->createVertexBuffer(vertices.data(), sizeof(vertices[0]) * vertices.size());
	m_ib = m_core->createVertexBuffer(indices.data(), sizeof(indices[0]) * indices.size());

	m_core->createUniformBuffers(m_uniformBuffers, UNIFORM_BUFFER_SIZE * m_meshes.size());

	m_vertexSize = sizeof(Vertex);
}

void VulkanModel::update(u32 imageIndex, const mat4& transformation) {
	std::vector<mat4> transformations(m_meshes.size());

	for (u32 submeshIndex = 0; submeshIndex < transformations.size(); submeshIndex++) {
		mat4 meshTransform = m_meshes[submeshIndex].trans;
		meshTransform = transpose(meshTransform);
		transformations[submeshIndex] = transformation * meshTransform;
	}

	m_uniformBuffers[imageIndex].update(m_core->getDevice(), transformations.data(), sizeof(mat4));
}

const VulkanBufferAndMemory* VulkanModel::getVB() const { return &m_vb; }
const VulkanBufferAndMemory* VulkanModel::getIB() const { return &m_ib; }

void VulkanModel::destroy() {
	m_vb.destroy(m_core->getDevice());
	m_ib.destroy(m_core->getDevice());

	for (VulkanBufferAndMemory& ubo : m_uniformBuffers)
		ubo.destroy(m_core->getDevice());
}

void VulkanModel::updateModelDesc(ModelDesc& md) {
	md.m_vb = m_vb.m_buffer;
	md.m_ib = m_ib.m_buffer;

	md.m_uniforms.resize(m_core->getNumImages());

	for (u32 imageIndex = 0; imageIndex < m_core->getNumImages(); imageIndex++)
		md.m_uniforms[imageIndex] = m_uniformBuffers[imageIndex].m_buffer;

	md.m_materials.resize(m_meshes.size());
	md.m_ranges.resize(m_meshes.size());

	u32 numSubmeshes = m_meshes.size();

	for (u32 submeshIndex = 0; submeshIndex < numSubmeshes; submeshIndex++) {
		int materialIndex = m_meshes[submeshIndex].materialIndex;

		if (materialIndex >= 0 && m_materials[materialIndex].diffuse) {
			VulkanTexture* pDiffuse = m_materials[materialIndex].diffuse;
			md.m_materials[submeshIndex].m_sampler = pDiffuse->m_sampler;
			md.m_materials[submeshIndex].m_imageView = pDiffuse->m_imageView;
		} else {
			error("No diffuse texture in material [{}]", materialIndex);
		}

		size_t offset = m_meshes[submeshIndex].baseVertex  * m_vertexSize;
		size_t range  = m_meshes[submeshIndex].numVertices * m_vertexSize;
		md.m_ranges[submeshIndex].m_vbRange = { .m_offset = offset, .m_range = range };

		offset = m_meshes[submeshIndex].baseIndex  * sizeof(u32);
		range  = m_meshes[submeshIndex].numIndices * sizeof(u32);
		md.m_ranges[submeshIndex].m_ibRange = { .m_offset = offset, .m_range = range };

		offset = submeshIndex * UNIFORM_BUFFER_SIZE;
		range = UNIFORM_BUFFER_SIZE;
		md.m_ranges[submeshIndex].m_uniformRange = { .m_offset = offset, .m_range = range };
	}
}

} // namespace lve

