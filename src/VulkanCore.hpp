#pragma once

#include "VulkanPhysicalDevices.hpp"

namespace lve {

class VulkanCore {
public:
  VulkanCore() = default;
  ~VulkanCore();

  void init(const std::string& appName, GLFWwindow* window);

  u32 getNumImages() const;
  void createCommandBuffers(u32 count, VkCommandBuffer* cmdBufs);
  void freeCommandBuffers(u32 count, const VkCommandBuffer* cmdBufs);

private:
  VkInstance m_instance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
  GLFWwindow* m_window = nullptr;
  VkSurfaceKHR m_surface = VK_NULL_HANDLE;
  VulkanPhysicalDevices m_physDevices;
  u32 m_queueFamily = 0;
  VkDevice m_device;
  VkSwapchainKHR m_swapchain;
  std::vector<VkImage> m_images;
  std::vector<VkImageView> m_imageViews;
  VkCommandPool m_cmdBufPool;

private:
  void createInstance(const std::string& appName);
  void createDebugMessageCallback();
  void createSurface(GLFWwindow* window);
  void createLogicalDevice();
  void createSwapchain(VkExtent2D windowExtent);
  void createCommandBufferPool();

  void destroyCommandBufferPool();
  void destroySwapchain();
  void destroyLogicalDevice();
  void destroySurface();
  void destroyDebugMessageCallback();
  void destroyInstance();
};

} // namespace lve

