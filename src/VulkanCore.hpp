#pragma once

#include "VulkanPhysicalDevices.hpp"

namespace lve {

class VulkanCore {
public:
  VulkanCore();
  ~VulkanCore();

  void init(const std::string& appName, GLFWwindow* window);

private:
  VkInstance m_instance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
  GLFWwindow* m_window = nullptr;
  VkSurfaceKHR m_surface = VK_NULL_HANDLE;
  VulkanPhysicalDevices m_physDevices;
  u32 m_queueFamily = 0;

private:
  void createInstance(const std::string& appName);
  void createDebugMessageCallback();
  void createSurface(GLFWwindow* window);

  void destroySurface();
  void destroyDebugMessageCallback();
  void destroyInstance();
};

} // namespace lve

