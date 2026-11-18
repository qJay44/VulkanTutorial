#pragma once

#include "VulkanPhysicalDevices.hpp"
#include "VulkanQueue.hpp"

namespace lve {

class VulkanCore {
public:
  VulkanCore() = default;
  ~VulkanCore();

  void init(const std::string& appName, GLFWwindow* window);

  VkRenderPass createSimpleRenderPass();
  std::vector<VkFramebuffer> createFramebuffers(VkRenderPass renderPass);
  void createCommandBuffers(u32 count, VkCommandBuffer* cmdBufs);

  VkDevice& getDevice();
  u32 getNumImages() const;
  const VkImage& getImage(u32 index) const;
  VulkanQueue* getQueue();
  u32 getQueueFamily() const;
  const std::vector<VkFramebuffer>& getFramebuffers() const;

  void freeCommandBuffers(u32 count, const VkCommandBuffer* cmdBufs);

private:
  VkInstance m_instance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
  GLFWwindow* m_window = nullptr;
  VkSurfaceKHR m_surface = VK_NULL_HANDLE;
  VulkanPhysicalDevices m_physDevices;
  u32 m_queueFamily = 0;
  VkDevice m_device;
  VkSurfaceFormatKHR m_swapchainSurfaceFormat;
  VkSwapchainKHR m_swapchain;
  std::vector<VkImage> m_images;
  std::vector<VkImageView> m_imageViews;
  VkCommandPool m_cmdBufPool;
  VulkanQueue m_queue;
  std::vector<VkFramebuffer> m_framebuffers;

private:
  void createInstance(const std::string& appName);
  void createDebugMessageCallback();
  void createSurface(GLFWwindow* window);
  void createLogicalDevice();
  void createSwapchain(VkExtent2D windowExtent);
  void createCommandBufferPool();
  void createQueue();

  void destroyFramebuffers();
  void destroyQueue();
  void destroyCommandBufferPool();
  void destroySwapchain();
  void destroyLogicalDevice();
  void destroySurface();
  void destroyDebugMessageCallback();
  void destroyInstance();
};

} // namespace lve

