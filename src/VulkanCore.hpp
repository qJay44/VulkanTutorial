#pragma once

namespace lve {

class VulkanCore {
public:
  VulkanCore();
  ~VulkanCore();

  void init(const std::string& appName);

private:
  VkInstance instance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

private:
  void createInstance(const std::string& appName);
  void createDebugMessageCallback();

  void destroyDebugMessageCallback();
  void destroyInstance();
};

} // namespace lve

