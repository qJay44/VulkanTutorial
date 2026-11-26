#include "ImGuiRenderer.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "../VulkanApp.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace lve {

namespace {

void CheckVkResult(VkResult res) {
  CHECK_VK_RESULT(res, "Vulkan ImGui error")
}

void fixGammaStyle() {
  ImGuiStyle &style = ImGui::GetStyle();
  // Go through every colour and convert it to linear
  // This is because ImGui uses linear colours but we are using sRGB
  // This is a simple approximation of the conversion
  for (int i = 0; i < ImGuiCol_COUNT; i++) {
    /*float linear = (srgb <= 0.04045f) ? srgb / 12.92f : pow((srgb + 0.055f)
     * / 1.055f, 2.4f);*/

    ImVec4 &col = style.Colors[i];
    col.x = col.x <= 0.04045f ? col.x / 12.92f
                              : pow((col.x + 0.055f) / 1.055f, 2.4f);
    col.y = col.y <= 0.04045f ? col.y / 12.92f
                              : pow((col.y + 0.055f) / 1.055f, 2.4f);
    col.z = col.z <= 0.04045f ? col.z / 12.92f
                              : pow((col.z + 0.055f) / 1.055f, 2.4f);
  }
}

void TextVec3(const std::string& text, vec3 v) {
  ImGui::Text((text + ": [%.2f, %.2f, %.2f]").c_str(), v.x, v.y, v.z);
}

} // namespace

bool ImGuiRenderer::isMouseControlledByImGui() {
  return ImGui::GetIO().WantCaptureMouse;
}

void ImGuiRenderer::init(VulkanCore* core, VulkanApp* app) {
  m_core = core;
  m_app = app;
  m_framebufferSize = m_core->getFramebufferSize();
  createDescriptorPool();
  initImGui();
}

void ImGuiRenderer::update() {
  ImGuiIO& io = ImGui::GetIO();

  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::Begin("Settings", nullptr);
  ImGui::Text("Application average %.2f ms/frame (%.1f FPS)", 1000.f / io.Framerate, io.Framerate);

  if (ImGui::CollapsingHeader("Camera")) {
    auto& cam = m_app->m_camera;

    TextVec3("Orientation", cam.orientation);
    TextVec3("Up", cam.up);

    ImGui::Text("Application average %.2f ms/frame (%.1f FPS)", 1000.f / io.Framerate, io.Framerate);
    ImGui::DragFloat3("Positon", glm::value_ptr(cam.pos));
    ImGui::DragFloat2("Sensitivity", glm::value_ptr(cam.sensitivity));
    ImGui::SliderAngle("Yaw", &cam.yaw);
    ImGui::SliderAngle("Pitch", &cam.pitch);
    ImGui::DragFloat("Speed", &cam.speed);
    ImGui::DragFloat("Speed multiplier", &cam.speedMult);
  }

  ImGui::End();
  ImGui::Render();
}

VkCommandBuffer ImGuiRenderer::prepareCommandBuffer(u32 imageIndex) {
  auto& cmdBuf = m_cmdBufs[imageIndex];

  wrappers::beginCommandBuffer(cmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  m_core->beginDynamicRendering(cmdBuf, imageIndex, nullptr, nullptr);

  ImDrawData* drawData = ImGui::GetDrawData();
  ImGui_ImplVulkan_RenderDrawData(drawData, cmdBuf);

  m_core->endDynamicRendering(cmdBuf);

  wrappers::imageMemBarrier(
    cmdBuf,
    m_core->getImage(imageIndex),
    m_core->getSwapchainSurfaceFormat().format,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
  );

  wrappers::endCommandBuffer(cmdBuf);

  return cmdBuf;
}

void ImGuiRenderer::destroy() {
  m_core->freeCommandBuffers(static_cast<u32>(m_cmdBufs.size()), m_cmdBufs.data());

  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  // The author calls it before the ImGui_ImplVulkan_Shutdown,
  // which in my case raises validation error
  // (I guess that function still needs the descriptor pool)
  vkDestroyDescriptorPool(m_core->m_device, m_descriptorPool, nullptr);

  dprint("Destroyed ImGui");
}

void ImGuiRenderer::createDescriptorPool() {
  VkDescriptorPoolSize poolSizes[] = {
    {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
    {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
    {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
    {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000},
  };

  VkDescriptorPoolCreateInfo poolCreateInfo{
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .pNext = nullptr,
    .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
    .maxSets = 1000 * IM_ARRAYSIZE(poolSizes),
    .poolSizeCount = static_cast<u32>(IM_ARRAYSIZE(poolSizes)),
    .pPoolSizes = poolSizes,
  };

  VkResult res = vkCreateDescriptorPool(m_core->getDevice(), &poolCreateInfo, nullptr, &m_descriptorPool);
  CHECK_VK_RESULT(res, "vkCreateDescriptorPool");
}

void ImGuiRenderer::initImGui() {
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
  io.DisplaySize.x = (float)m_framebufferSize.width;
  io.DisplaySize.y = (float)m_framebufferSize.height;

  ImGui::GetStyle().FontScaleMain = 1.2f;
  fixGammaStyle();
  // ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForVulkan(m_core->getWindow(), true);

  VkFormat colorFormat = m_core->getSwapchainSurfaceFormat().format;

  ImGui_ImplVulkan_InitInfo initInfo{
    .ApiVersion = m_core->m_instanceVersion.bits,
    .Instance = m_core->m_instance,
    .PhysicalDevice = m_core->m_physDevices.selected().m_physDevice,
    .Device = m_core->m_device,
    .QueueFamily = m_core->m_queueFamily,
    .Queue = m_core->m_queue.getHandle(),
    .DescriptorPool = m_descriptorPool,
    .DescriptorPoolSize{},
    .MinImageCount = m_core->m_physDevices.selected().m_surfaceCaps.minImageCount,
    .ImageCount = m_core->getNumImages(),
    .PipelineCache = nullptr,
    .PipelineInfoMain = {
      .RenderPass = nullptr,
      .Subpass = 0,
      .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
      .PipelineRenderingCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = nullptr,
        .viewMask = 0,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &colorFormat,
        .depthAttachmentFormat = m_core->getDepthFormat(),
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
      },
    },
    .UseDynamicRendering = true,
    .Allocator = nullptr,
    .CheckVkResultFn = CheckVkResult,
    .MinAllocationSize{},
    .CustomShaderVertCreateInfo{},
    .CustomShaderFragCreateInfo{},
  };

  ImGui_ImplVulkan_Init(&initInfo);
  m_cmdBufs.resize(m_core->getNumImages());
  m_core->allocateCommandBuffers(m_core->getNumImages(), m_cmdBufs.data());
}

} // namespace lve

