#include "VulkanGraphicsPipeline.hpp"

namespace lve {

VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {
  destroy();
}

void VulkanGraphicsPipeline::init(
  VkDevice device,
  GLFWwindow* window,
  VkRenderPass renderPass,
  VkShaderModule vs,
  VkShaderModule fs,
  const VulkanMesh* mesh,
  u32 numImages,
  const std::vector<VulkanBufferAndMemory>& uniformBuffers,
  size_t uniformDataSize
) {
  m_device = device;

  if (mesh)
    createDescriptorSets(mesh, numImages, uniformBuffers, uniformDataSize);

  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStageInfos{};
  shaderStageInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStageInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStageInfos[0].module = vs;
  shaderStageInfos[0].pName = "main";

  shaderStageInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStageInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaderStageInfos[1].module = fs;
  shaderStageInfos[1].pName = "main";

  VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
  vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo{};
  inputAssemblyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssemblyStateInfo.primitiveRestartEnable = VK_FALSE;

  VkExtent2D windowExtent = GetWindowExtent(window);

  VkViewport viewport{
    .x = 0.f,
    .y = 0.f,
    .width = static_cast<float>(windowExtent.width),
    .height = static_cast<float>(windowExtent.height),
    .minDepth = 0.f,
    .maxDepth = 1.f,
  };

  VkRect2D scissor{
    .offset{},
    .extent = windowExtent,
  };

  VkPipelineViewportStateCreateInfo viewportStateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .viewportCount = 1,
    .pViewports = &viewport,
    .scissorCount = 1,
    .pScissors = &scissor,
  };

  VkPipelineRasterizationStateCreateInfo rasterStateInfo{};
  rasterStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
  rasterStateInfo.cullMode = VK_CULL_MODE_NONE;
  rasterStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterStateInfo.lineWidth = 1.f;

  VkPipelineMultisampleStateCreateInfo multisampleStateInfo{};
  multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampleStateInfo.sampleShadingEnable = VK_FALSE;
  multisampleStateInfo.minSampleShading = 1.f;

  VkPipelineColorBlendAttachmentState blendAttachState{};
  blendAttachState.blendEnable = VK_FALSE;
  blendAttachState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo blendStateInfo{};
  blendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blendStateInfo.logicOpEnable = VK_FALSE;
  blendStateInfo.logicOp = VK_LOGIC_OP_COPY;
  blendStateInfo.attachmentCount = 1;
  blendStateInfo.pAttachments = &blendAttachState;

  VkPipelineLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layoutInfo.setLayoutCount = 0;
  layoutInfo.pSetLayouts = nullptr;

  if (mesh && mesh->m_vb.m_buffer) {
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &m_descriptorSetLayout;
  }

  VkResult res = vkCreatePipelineLayout(device, &layoutInfo, nullptr, &m_pipelineLayout);
  CHECK_VK_RESULT(res, "vkCreatePipelineLayout");

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = shaderStageInfos.size();
  pipelineInfo.pStages = shaderStageInfos.data();
  pipelineInfo.pVertexInputState   = &vertexInputStateInfo;
  pipelineInfo.pInputAssemblyState = &inputAssemblyStateInfo;
  pipelineInfo.pViewportState      = &viewportStateInfo;
  pipelineInfo.pRasterizationState = &rasterStateInfo;
  pipelineInfo.pMultisampleState   = &multisampleStateInfo;
  pipelineInfo.pColorBlendState    = &blendStateInfo;
  pipelineInfo.layout = m_pipelineLayout;
  pipelineInfo.renderPass = renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.basePipelineIndex = -1;

  res = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);
  CHECK_VK_RESULT(res, "vkCreateGraphicsPipelines");
  dprint("Created graphics pipeline");
}

void VulkanGraphicsPipeline::bind(VkCommandBuffer cmdBuf, u32 imageIndex) {
  vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

  if (!m_descriptorSets.empty()) {
    vkCmdBindDescriptorSets(
      cmdBuf,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      m_pipelineLayout,
      0, // First set
      1, // descriptorSetCount
      &m_descriptorSets[imageIndex],
      0,      // dynamicOffsetCount
      nullptr // pDynamicOffsets
    );
  }
}

void VulkanGraphicsPipeline::destroy() {
  // NOTE: Assuming [m_device] is assigned only during init()
  if (m_device) {
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
    vkDestroyPipeline(m_device, m_pipeline, nullptr);
    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
    dprint("Destroyed graphics pipeline");

    m_device = nullptr;
  }
}

void VulkanGraphicsPipeline::createDescriptorSets(
  const VulkanMesh* mesh,
  u32 numImages,
  const std::vector<VulkanBufferAndMemory>& uniformBuffers,
  size_t uniformDataSize
) {
  createDescriptorPool(numImages);
  createDescriptorSetLayout(uniformBuffers);
  allocateDescriptorSets(numImages);
  updateDescriptorSets(mesh, numImages, uniformBuffers, uniformDataSize);
}

void VulkanGraphicsPipeline::createDescriptorPool(u32 numImages) {
  std::array<VkDescriptorPoolSize, 2> poolSizes = {
    VkDescriptorPoolSize{
      .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .descriptorCount = numImages,
    },
    VkDescriptorPoolSize{
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = numImages,
    },
  };

  VkDescriptorPoolCreateInfo poolInfo{
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .maxSets = numImages,
    .poolSizeCount = poolSizes.size(),
    .pPoolSizes = poolSizes.data(),
  };

  VkResult res = vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool);
  CHECK_VK_RESULT(res, "vkCreateDescriptorPool");
  dprint("Created descriptor pool");
}

void VulkanGraphicsPipeline::createDescriptorSetLayout(const std::vector<VulkanBufferAndMemory>& uniformBuffers) {
  std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {
    VkDescriptorSetLayoutBinding{
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      .pImmutableSamplers = nullptr,
    },
  };

  if (!uniformBuffers.empty()) {
    layoutBindings.push_back({
      .binding = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      .pImmutableSamplers = nullptr,
    });
  }

  VkDescriptorSetLayoutCreateInfo layoutInfo{
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0, // Must be zero
    .bindingCount = static_cast<u32>(layoutBindings.size()),
    .pBindings = layoutBindings.data(),
  };

  VkResult res = vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout);
  CHECK_VK_RESULT(res, "vkCreateDescriptorSetLayout");
}

void VulkanGraphicsPipeline::allocateDescriptorSets(u32 numImages) {
  std::vector<VkDescriptorSetLayout> layouts(numImages, m_descriptorSetLayout);

  VkDescriptorSetAllocateInfo allocInfo{
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .pNext = nullptr,
    .descriptorPool = m_descriptorPool,
    .descriptorSetCount = numImages,
    .pSetLayouts = layouts.data(),
  };

  m_descriptorSets.resize(numImages);

  VkResult res = vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data());
  CHECK_VK_RESULT(res, "vkAllocateDescriptorSets");
}

void VulkanGraphicsPipeline::updateDescriptorSets(
  const VulkanMesh* mesh,
  u32 numImages,
  const std::vector<VulkanBufferAndMemory>& uniformBuffers,
  size_t uniformDataSize
) {
  VkDescriptorBufferInfo vertexBufferInfo{
    .buffer = mesh->m_vb.m_buffer,
    .offset = 0,
    .range = mesh->m_vertexBufferSize,
  };

  std::vector<VkDescriptorBufferInfo> uniformBufferInfos(uniformBuffers.size());
  for (size_t i = 0; i < uniformBufferInfos.size(); i++) {
    uniformBufferInfos[i] = {
      .buffer = uniformBuffers[i].m_buffer,
      .offset = 0,
      .range = static_cast<VkDeviceSize>(uniformDataSize),
    };
  }

  VkWriteDescriptorSet defaultWriteDescriptorSet{};
  defaultWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  defaultWriteDescriptorSet.dstArrayElement = 0;
  defaultWriteDescriptorSet.descriptorCount = 1;

  assert(uniformBuffers.size() == numImages);

  // One buffer used for all [numImages]
  u32 vertexWriteDescriptors = numImages;
  u32 uniformWriteDescriptors = numImages * !uniformBuffers.empty();
  u32 totalWrtieDescriptors = vertexWriteDescriptors + uniformWriteDescriptors;
  std::vector<VkWriteDescriptorSet> writeDescriptorSets(totalWrtieDescriptors, defaultWriteDescriptorSet);

  for (u32 i = 0; i < numImages; i++) {
    auto& descVertex = writeDescriptorSets[i];
    descVertex.dstBinding = 0;
    descVertex.dstSet = m_descriptorSets[i];
    descVertex.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descVertex.pBufferInfo = &vertexBufferInfo;

    if (!uniformBuffers.empty()) {
      auto& descUniform = writeDescriptorSets[i + vertexWriteDescriptors];
      descUniform.dstBinding = 1;
      descUniform.dstSet = m_descriptorSets[i];
      descUniform.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descUniform.pBufferInfo = &uniformBufferInfos[i];
    }
  }

  vkUpdateDescriptorSets(m_device, totalWrtieDescriptors, writeDescriptorSets.data(), 0, nullptr);
}

} // namespace lve

