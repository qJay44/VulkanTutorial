#pragma once

namespace lve {

class VulkanCore {
public:
  VulkanCore();
  ~VulkanCore();

  void init(const std::string& appName);

private:
  VkInstance instance = nullptr;

private:
  void createInstance(const std::string& appName);
};

} // namespace lve

