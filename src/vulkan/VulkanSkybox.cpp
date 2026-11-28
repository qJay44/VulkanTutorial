#include "VulkanSkybox.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace lve {

#define UNIFORM_BUFFER_SIZE sizeof(mat4)

void VulkanSkybox::init(VulkanCore* core, const fspath& path) {
  m_core = core;
  m_numImages = core->getNumImages();

  core->createUniformBuffers(m_uniformBuffers, UNIFORM_BUFFER_SIZE);

  m_texCubemap = new VulkanTexture(core);
  m_texCubemap->loadEctCubemap(path);

  m_vs.createShaderModuleFromText(core->getDevice(), "src/shaders/skybox.vert");
  m_fs.createShaderModuleFromText(core->getDevice(), "src/shaders/skybox.frag");

  VulkanGraphicsPipeline2::PipelineDescriptor pd{
    .device = core->getDevice(),
    .window = core->getWindow(),
    .vs = m_vs.shaderModule,
    .fs = m_fs.shaderModule,
    .numImages = m_numImages,
    .colorFormat = core->getSwapchainSurfaceFormat().format,
    .depthFormat = core->getDepthFormat(),
    .depthCompareOp = VK_COMPARE_OP_EQUAL,
    .isUniform = true,
    .isTexCubemap = true,
  };

  m_pipeline = new VulkanGraphicsPipeline2(pd);
  createDescriptorSets();
}

void VulkanSkybox::recordCommandBuffer(VkCommandBuffer cmdBuf, u32 imageIndex) {
  m_pipeline->bind(cmdBuf);

  vkCmdBindDescriptorSets(
    cmdBuf,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    m_pipeline->getPipelineLayout(),
    0,
    1,
    m_descriptorSets[imageIndex].data(),
    0,
    nullptr
  );

  u32 numVertices = 36;
  u32 instanceCount = 1;
  u32 baseVertex = 0;
  u32 firstInstance = 0;

  vkCmdDraw(cmdBuf, numVertices, instanceCount, baseVertex, firstInstance);
}

void VulkanSkybox::update(u32 imageIndex, const mat4& transform) {
  m_uniformBuffers[imageIndex].update(m_core->getDevice(), glm::value_ptr(transform), sizeof(mat4));
}

void VulkanSkybox::destroy() {
  auto& device = m_core->getDevice();

  for (VulkanBufferAndMemory& bam : m_uniformBuffers)
    bam.destroy(device);

  m_vs.destroy(device);
  m_fs.destroy(device);

  m_texCubemap->destroy(device);
  delete m_texCubemap;
  delete m_pipeline;
}

void VulkanSkybox::createDescriptorSets() {
  u32 numSubmeshes = 1;
  m_pipeline->allocateDescriptorSets(numSubmeshes, m_descriptorSets);

  u32 numBindings = 2; // Uniform, Cubemap
  std::vector<VkWriteDescriptorSet> writeDescriptorSets(m_numImages * numBindings);

  VkDescriptorImageInfo imageInfo{
    .sampler = m_texCubemap->m_sampler,
    .imageView = m_texCubemap->m_imageView,
    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
  };

  std::vector<VkDescriptorBufferInfo> bufferInfo_Uniforms(m_numImages);
  u32 wdsIdx = 0;

  for (u32 imageIndex = 0; imageIndex < m_numImages; imageIndex++) {
    bufferInfo_Uniforms[imageIndex].buffer = m_uniformBuffers[imageIndex].m_buffer;
    bufferInfo_Uniforms[imageIndex].offset = 0;
    bufferInfo_Uniforms[imageIndex].range = VK_WHOLE_SIZE;
  }

  for (u32 imageIndex = 0; imageIndex < m_numImages; imageIndex++) {
    VkDescriptorSet dstSet = m_descriptorSets[imageIndex][0];

    writeDescriptorSets[wdsIdx++] = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = nullptr,
      .dstSet = dstSet,
      .dstBinding = VulkanGraphicsPipeline2::Binding_Uniform,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .pImageInfo = nullptr,
      .pBufferInfo = &bufferInfo_Uniforms[imageIndex],
      .pTexelBufferView = nullptr,
    };

    writeDescriptorSets[wdsIdx++] = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = nullptr,
      .dstSet = dstSet,
      .dstBinding = VulkanGraphicsPipeline2::Binding_TextureCubemap,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImageInfo = &imageInfo,
      .pBufferInfo = nullptr,
      .pTexelBufferView = nullptr,
    };
  }

  vkUpdateDescriptorSets(m_core->getDevice(), static_cast<u32>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

} // namespace lve

