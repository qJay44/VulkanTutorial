#pragma once

namespace lve {

class VulkanCore {
public:
  VulkanCore();
  ~VulkanCore();

  void init(const std::string& appName, GLFWwindow* window);

private:
  VkInstance instance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
  VkSurfaceKHR surface;

private:
  void createInstance(const std::string& appName);
  void createDebugMessageCallback();
  void createSurface(GLFWwindow* window);

  void destroySurface();
  void destroyDebugMessageCallback();
  void destroyInstance();
};

} // namespace lve

