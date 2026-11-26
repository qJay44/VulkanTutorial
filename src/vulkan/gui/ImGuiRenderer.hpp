#pragma once

#include "../VulkanCore.hpp"

namespace lve {

class ImGuiRenderer {
public:
  static bool isMouseControlledByImGui();

  ImGuiRenderer() = default;
  ~ImGuiRenderer() = default;

  void init(VulkanCore* core, class VulkanApp* app);

  void update();
  [[nodiscard]] VkCommandBuffer prepareCommandBuffer(u32 imageIndex);

  void destroy();

private:
  VulkanCore* m_core = nullptr;
  VulkanApp* m_app = nullptr;
  VkExtent2D m_framebufferSize{};
  std::vector<VkCommandBuffer> m_cmdBufs;
  VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

private:
  void createDescriptorPool();
  void initImGui();
};

} // namespace lve

