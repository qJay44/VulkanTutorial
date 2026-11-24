#pragma once

#include "VulkanBufferAndMemory.hpp"
#include "VulkanPhysicalDevices.hpp"
#include "VulkanQueue.hpp"
#include "VulkanTexture.hpp"

namespace lve {

class VulkanCore {
public:
  VulkanCore() = default;
  ~VulkanCore();

  void init(const std::string& appName, GLFWwindow* window);

  void allocateCommandBuffers(u32 count, VkCommandBuffer* cmdBufs);
  VulkanBufferAndMemory createVertexBuffer(const void* vertices, size_t size);
  void createUniformBuffers(std::vector<VulkanBufferAndMemory>& ubos, size_t size);
  VulkanBufferAndMemory createUniformBuffer(size_t size);

  VkDevice& getDevice();
  u32 getNumImages() const;
  const VkSwapchainKHR& getSwapchain() const;
  const VkSurfaceFormatKHR& getSwapchainSurfaceFormat() const;
  const VkImage& getImage(u32 index) const;
  const VkImageView& getImageView(u32 index) const;
  const VkImageView& getDepthImageView(u32 index) const;
  VulkanQueue* getQueue();
  u32 getQueueFamily() const;
  const std::vector<VkFramebuffer>& getFramebuffers() const;
  VkFormat getDepthFormat() const;

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
  VkCommandBuffer m_copyCmdBuf;
  std::vector<VulkanTexture> m_depthImages;

  friend VulkanTexture;

private:
  void createInstance(const std::string& appName);
  void createDebugMessageCallback();
  void createSurface(GLFWwindow* window);
  void createLogicalDevice();
  void createSwapchain(VkExtent2D windowExtent);
  void createCommandBufferPool();
  void createQueue();
  void createDepthResources();

  void destroyDepthResources();
  void destroyQueue();
  void destroyCommandBufferPool();
  void destroySwapchain();
  void destroyLogicalDevice();
  void destroySurface();
  void destroyDebugMessageCallback();
  void destroyInstance();

  void submitCopyCommand();
  void copyBufferToBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
  void copyBufferToImage(VkBuffer src, VkImage dst, VkExtent3D imgExtent);
};

} // namespace lve

