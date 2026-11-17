#pragma once

namespace lve {

struct PhysicalDevice {
  VkPhysicalDevice m_physDevice;
  VkPhysicalDeviceProperties m_devProps;
  std::vector<VkQueueFamilyProperties> m_qFamilyProps;
  std::vector<VkBool32> m_qSupportsPresent;
  std::vector<VkSurfaceFormatKHR> m_surfaceFormats;
  VkSurfaceCapabilitiesKHR m_surfaceCaps;
  VkPhysicalDeviceMemoryProperties m_memProps;
  std::vector<VkPresentModeKHR> m_presentModes;
};

class VulkanPhysicalDevices {
public:
  VulkanPhysicalDevices();
  ~VulkanPhysicalDevices();

  void init(const VkInstance& instance, const VkSurfaceKHR& surface);

  u32 selectDevice(VkQueueFlags requiredQueueType, bool supportsPresent);

  const PhysicalDevice& selected() const;

private:
  std::vector<PhysicalDevice> m_devices;
  int m_devIndex = -1;
};

} // namespace lve

