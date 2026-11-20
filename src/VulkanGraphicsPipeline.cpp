#include "VulkanGraphicsPipeline.hpp"

namespace lve {

void VulkanGraphicsPipeline::init(
  VkDevice device,
  GLFWwindow* window,
  VkRenderPass renderPass,
  VkShaderModule vs,
  VkShaderModule fs
) {
  m_device = device;

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

VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {
  destroy();
}

void VulkanGraphicsPipeline::bind(VkCommandBuffer cmdBuf) {
  vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
}

void VulkanGraphicsPipeline::destroy() {
  // NOTE: Assuming [m_device] is assigned only during init()
  if (m_device) {
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
    vkDestroyPipeline(m_device, m_pipeline, nullptr);
    dprint("Destroyed graphics pipeline");

    m_device = nullptr;
  }
}

} // namespace lve

