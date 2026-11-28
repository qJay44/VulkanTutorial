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
  void createTexture2D(const fspath& path, VulkanTexture* tex);
  void createTextureFromData(const void* pixels, VkFormat format, VkExtent3D imageExtent, VulkanTexture* tex, bool isCubemap);
  void createTextureCubemap(const fspath& path, VulkanTexture* tex);

  VkDevice& getDevice();
  u32 getNumImages() const;
  GLFWwindow* getWindow() const;
  const VkSwapchainKHR& getSwapchain() const;
  const VkSurfaceFormatKHR& getSwapchainSurfaceFormat() const;
  const VkImage& getImage(u32 index) const;
  const VkImageView& getImageView(u32 index) const;
  const VkImageView& getDepthImageView(u32 index) const;
  VulkanQueue* getQueue();
  u32 getQueueFamily() const;
  VkExtent2D getFramebufferSize() const;
  VkFormat getDepthFormat() const;

  void beginDynamicRendering(VkCommandBuffer cmdBuf, u32 imageIndex, VkClearValue* clearColor, VkClearValue* depthDepth);
  void endDynamicRendering(VkCommandBuffer cmdBuf);

  void freeCommandBuffers(u32 count, const VkCommandBuffer* cmdBufs);

private:
  struct {
    u32 bits;
    u8 major{};
    u8 minor{};
    u8 patch{};
  } m_instanceVersion;

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

  friend class ImGuiRenderer;

private:
  void populateInstanceVersion();

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
  void copyBufferToImage(VkBuffer src, VkImage dst, VkExtent3D imgExtent, VkDeviceSize layerSize, u32 layerCount);

  void transitionImageLayout(
    VulkanTexture* tex,
    VkFormat format,
    u32 layerCount,
    VkImageLayout oldLayout,
    VkImageLayout newLayout
  );

  void createTextureImageFromData(
    VulkanTexture* tex,
    const void* pixels,
    VkExtent3D extent,
    VkFormat format,
    bool isCubemap
  );

  void createImage(
    VulkanTexture* tex,
    VkExtent3D extent,
    VkFormat format,
    VkImageUsageFlagBits usage,
    VkMemoryPropertyFlagBits propertyFlags,
    bool isCubemap
  );

  void updateTextureImage(
    VulkanTexture* tex,
    VkExtent3D extent,
    VkFormat format,
    const void* pixels,
    bool isCubemap
  );
};

} // namespace lve

