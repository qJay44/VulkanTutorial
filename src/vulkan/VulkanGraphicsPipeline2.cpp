#include "VulkanGraphicsPipeline2.hpp"

namespace lve {

namespace binding {

enum Binding {
  VB = 0,
  IB = 1,
  Texture = 2,
  Uniform = 3,
  BindingCount = 4
};

} // namespace binding

VulkanGraphicsPipeline2::VulkanGraphicsPipeline2(
  VkDevice device,
  GLFWwindow* window,
  VkShaderModule vs,
  VkShaderModule fs,
  u32 numImages,
  VkFormat colorFormat,
  VkFormat depthFormat
) : m_device(device), m_numImages(numImages)
{
  bool isVB = true;
  bool isIB = true;
  bool isTex = true;
  bool isUniform = true;

  createDescriptorSetLayout(isVB, isIB, isTex, isUniform);
  initCommon(window, vs, fs, colorFormat, depthFormat);
}

void VulkanGraphicsPipeline2::initCommon(
  GLFWwindow* window,
  VkShaderModule vs,
  VkShaderModule fs,
  VkFormat colorFormat,
  VkFormat depthFormat
) {
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

  VkExtent2D windowExtent = utils::GetWindowExtent(window);

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

  VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo{};
  depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencilStateInfo.depthTestEnable = VK_TRUE;
  depthStencilStateInfo.depthWriteEnable = VK_TRUE;
  depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
  depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
  depthStencilStateInfo.stencilTestEnable = VK_FALSE;
  depthStencilStateInfo.front = {};
  depthStencilStateInfo.back = {};
  depthStencilStateInfo.minDepthBounds = 0.f;
  depthStencilStateInfo.maxDepthBounds = 1.f;

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
  layoutInfo.setLayoutCount = 1;
  layoutInfo.pSetLayouts = &m_descriptorSetLayout;

  VkResult res = vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &m_pipelineLayout);
  CHECK_VK_RESULT(res, "vkCreatePipelineLayout");

  VkPipelineRenderingCreateInfo renderingInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
    .pNext = nullptr,
    .viewMask = 0,
    .colorAttachmentCount = 1,
    .pColorAttachmentFormats = &colorFormat,
    .depthAttachmentFormat = depthFormat,
    .stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
  };

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.pNext = &renderingInfo;
  pipelineInfo.stageCount = shaderStageInfos.size();
  pipelineInfo.pStages = shaderStageInfos.data();
  pipelineInfo.pVertexInputState   = &vertexInputStateInfo;
  pipelineInfo.pInputAssemblyState = &inputAssemblyStateInfo;
  pipelineInfo.pViewportState      = &viewportStateInfo;
  pipelineInfo.pRasterizationState = &rasterStateInfo;
  pipelineInfo.pMultisampleState   = &multisampleStateInfo;
  pipelineInfo.pDepthStencilState  = &depthStencilStateInfo;
  pipelineInfo.pColorBlendState    = &blendStateInfo;
  pipelineInfo.layout = m_pipelineLayout;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.basePipelineIndex = -1;

  res = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);
  CHECK_VK_RESULT(res, "vkCreateGraphicsPipelines");
  dprint("Created graphics pipeline");
}

VulkanGraphicsPipeline2::~VulkanGraphicsPipeline2() {
  vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
  vkDestroyPipeline(m_device, m_pipeline, nullptr);
  vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
  vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
  dprint("Destroyed graphics pipeline");
}

void VulkanGraphicsPipeline2::bind(VkCommandBuffer cmdBuf) {
  vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
}

void VulkanGraphicsPipeline2::allocateDescriptorSets(u32 numSubmeshes, std::vector<std::vector<VkDescriptorSet>>& descriptorSets) {
  createDescriptorPool(numSubmeshes * m_numImages);
  allocateDescriptorSetsInternal(numSubmeshes, descriptorSets);
}

void VulkanGraphicsPipeline2::updateDescriptorSets(const ModelDesc& modelDesc, std::vector<std::vector<VkDescriptorSet>>& descriptorSets) {
  u32 numSubmeshes = (u32)descriptorSets[0].size();
  u32 descriptorWriteCount = m_numImages * numSubmeshes * binding::BindingCount;
  u32 wdsIdx = 0;

  std::vector<VkWriteDescriptorSet> writeDescriptorSet(descriptorWriteCount);

  std::vector<VkDescriptorBufferInfo> bufferInfo_VBs(numSubmeshes);
  std::vector<VkDescriptorBufferInfo> bufferInfo_IBs(numSubmeshes);
  std::vector<VkDescriptorImageInfo> imageInfos(numSubmeshes);
  std::vector<std::vector<VkDescriptorBufferInfo>> bufferInfo_Uniforms(m_numImages);

  for (u32 submeshIdx = 0; submeshIdx < numSubmeshes; submeshIdx++) {
    bufferInfo_VBs[submeshIdx].buffer = modelDesc.m_vb;
    bufferInfo_VBs[submeshIdx].offset = modelDesc.m_ranges[submeshIdx].m_vbRange.m_offset;
    bufferInfo_VBs[submeshIdx].range  = modelDesc.m_ranges[submeshIdx].m_vbRange.m_range;

    bufferInfo_IBs[submeshIdx].buffer = modelDesc.m_ib;
    bufferInfo_IBs[submeshIdx].offset = modelDesc.m_ranges[submeshIdx].m_ibRange.m_offset;
    bufferInfo_IBs[submeshIdx].range  = modelDesc.m_ranges[submeshIdx].m_ibRange.m_range;

    imageInfos[submeshIdx].sampler     = modelDesc.m_materials[submeshIdx].m_sampler;
    imageInfos[submeshIdx].imageView   = modelDesc.m_materials[submeshIdx].m_imageView;
    imageInfos[submeshIdx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  }

  for (u32 imageIdx = 0; imageIdx < m_numImages; imageIdx++) {
    auto& curr = bufferInfo_Uniforms[imageIdx];
    curr.reserve(numSubmeshes);

    for (u32 submeshIdx = 0; submeshIdx < numSubmeshes; submeshIdx++) {
      curr.push_back({
        .buffer = modelDesc.m_uniforms[imageIdx],
        .offset = modelDesc.m_ranges[submeshIdx].m_uniformRange.m_offset,
        .range  = modelDesc.m_ranges[submeshIdx].m_uniformRange.m_range,
      });
    }
  }

  for (u32 imageIdx = 0; imageIdx < m_numImages; imageIdx++) {
    for (u32 submeshIdx = 0; submeshIdx < numSubmeshes; submeshIdx++) {
      VkDescriptorSet dstSet = descriptorSets[imageIdx][submeshIdx];

      writeDescriptorSet[wdsIdx++] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = dstSet,
        .dstBinding = binding::VB,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = &bufferInfo_VBs[submeshIdx],
        .pTexelBufferView = nullptr,
      };

      writeDescriptorSet[wdsIdx++] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = dstSet,
        .dstBinding = binding::IB,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = &bufferInfo_IBs[submeshIdx],
        .pTexelBufferView = nullptr,
      };

      writeDescriptorSet[wdsIdx++] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = dstSet,
        .dstBinding = binding::Uniform,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = &bufferInfo_Uniforms[imageIdx][submeshIdx],
        .pTexelBufferView = nullptr,
      };

      writeDescriptorSet[wdsIdx++] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = dstSet,
        .dstBinding = binding::Texture,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &imageInfos[submeshIdx],
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr,
      };
    }
  }

  vkUpdateDescriptorSets(m_device, descriptorWriteCount, writeDescriptorSet.data(), 0, nullptr);
}

VkPipelineLayout VulkanGraphicsPipeline2::getPipelineLayout() const {
  return m_pipelineLayout;
}

void VulkanGraphicsPipeline2::createDescriptorPool(u32 maxSets) {
  // WARNING: All of [.descriptorCount] probably should rely on number of images
  std::array<VkDescriptorPoolSize, 4> poolSizes = {
    VkDescriptorPoolSize{
      .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .descriptorCount = maxSets,
    },
    VkDescriptorPoolSize{
      .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .descriptorCount = maxSets,
    },
    VkDescriptorPoolSize{
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = maxSets,
    },
    VkDescriptorPoolSize{
      .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = maxSets,
    },
  };

  VkDescriptorPoolCreateInfo poolInfo{
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .maxSets = maxSets,
    .poolSizeCount = poolSizes.size(),
    .pPoolSizes = poolSizes.data(),
  };

  VkResult res = vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool);
  CHECK_VK_RESULT(res, "vkCreateDescriptorPool");
  dprint("Created descriptor pool");
}

void VulkanGraphicsPipeline2::allocateDescriptorSetsInternal(u32 numSubmeshes, std::vector<std::vector<VkDescriptorSet>>& descriptorSets) {
  std::vector<VkDescriptorSetLayout> layouts(numSubmeshes, m_descriptorSetLayout);

  VkDescriptorSetAllocateInfo allocInfo{
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .pNext = nullptr,
    .descriptorPool = m_descriptorPool,
    .descriptorSetCount = static_cast<u32>(layouts.size()),
    .pSetLayouts = layouts.data(),
  };

  descriptorSets.resize(m_numImages);

  for (auto& submeshesDescSet : descriptorSets) {
    submeshesDescSet.resize(numSubmeshes);

    VkResult res = vkAllocateDescriptorSets(m_device, &allocInfo, submeshesDescSet.data());
    CHECK_VK_RESULT(res, "vkAllocateDescriptorSets");
  }
}

void VulkanGraphicsPipeline2::createDescriptorSetLayout(bool isVB, bool isIB, bool isTex, bool isUniform) {
  std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
  layoutBindings.reserve(isVB + isIB + isTex + isUniform);

  if (isVB)
    layoutBindings.push_back({
      .binding = binding::VB,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      .pImmutableSamplers = nullptr,
    });

  if (isIB)
    layoutBindings.push_back({
      .binding = binding::IB,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      .pImmutableSamplers = nullptr,
    });

  if (isTex)
    layoutBindings.push_back({
      .binding = binding::Texture,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      .pImmutableSamplers = nullptr,
    });

  if (isUniform)
    layoutBindings.push_back({
      .binding = binding::Uniform,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      .pImmutableSamplers = nullptr,
    });

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

} // namespace lve

