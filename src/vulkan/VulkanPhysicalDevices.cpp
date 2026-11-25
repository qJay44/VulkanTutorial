#include "VulkanPhysicalDevices.hpp"

namespace lve {

namespace {

void PrintImageUsageFlags(const VkImageUsageFlags& flags) {
  if (flags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)             dprint("Image usage transfer src is supported");
  if (flags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)             dprint("Image usage transfer dest is supported");
  if (flags & VK_IMAGE_USAGE_SAMPLED_BIT)                  dprint("Image usage sampled is supported");
  if (flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)         dprint("Image usage color attachment is supported");
  if (flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) dprint("Image usage depth stencil attachment is supported");
  if (flags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)     dprint("Image usage transient attachment is supported");
  if (flags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)         dprint("Image usage input attachment is supported");
}

std::string StringifyMemoryProperty(VkMemoryPropertyFlags flags) {
  std::string propsStr = " [";

  if (flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)     propsStr += "DEVICE LOCAL, ";
  if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)     propsStr += "HOST VISIBLE, ";
  if (flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)    propsStr += "HOST COHERENT, ";
  if (flags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)      propsStr += "HOST CACHED, ";
  if (flags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) propsStr += "LAZILY ALLOCATED, ";
  if (flags & VK_MEMORY_PROPERTY_PROTECTED_BIT)        propsStr += "PROTECTED, ";

  propsStr.pop_back();
  propsStr.back() = ']';

  return propsStr;
}

VkFormat FindDepthFormat(VkPhysicalDevice physDevice) {
  std::vector<VkFormat> candidates = {
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D24_UNORM_S8_UINT,
  };

  VkFormat depthFormat = utils::FindSupportedFormat(physDevice, candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

  return depthFormat;
}

} // namespace

VulkanPhysicalDevices::VulkanPhysicalDevices() {
}

VulkanPhysicalDevices::~VulkanPhysicalDevices() {
}

bool PhysicalDevice::isExtensionSupported(std::string_view ext) const {
  bool ret = std::ranges::find_if(m_extensions, [&ext](const VkExtensionProperties& e) {
    return e.extensionName == ext;
  }) != m_extensions.end();

  dprint("Device extension [{}] [{}] supported", ext, ret ? "is" : "is not");

  return ret;
}

void VulkanPhysicalDevices::init(const VkInstance& instance, const VkSurfaceKHR& surface) {
  u32 numDevices = 0;
  VkResult res = vkEnumeratePhysicalDevices(instance, &numDevices, nullptr);
  CHECK_VK_RESULT(res, "vkEnumeratePhysicalDevices (1)");

  dprint("Num physical devices: [{}]", numDevices);

  m_devices.resize(numDevices);
  std::vector<VkPhysicalDevice> devicesHandles(numDevices);

  res = vkEnumeratePhysicalDevices(instance, &numDevices, devicesHandles.data());
  CHECK_VK_RESULT(res, "vkEnumeratePhysicalDevices (2)");

  for (u32 i = 0; i < numDevices; i++) {
    PhysicalDevice& currDevice = m_devices[i];

    VkPhysicalDevice physDevice = devicesHandles[i];
    currDevice.m_physDevice = physDevice;

    vkGetPhysicalDeviceProperties(physDevice, &currDevice.m_devProps);
    dprint("Device name: [{}]", currDevice.m_devProps.deviceName);

    populateApiVersion(currDevice);
    populateExtensions(currDevice);

    u32 numQFamilies = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &numQFamilies, nullptr);
    dprint("\tNum of family queues: [{}]", numQFamilies);

    currDevice.m_qFamilyProps.resize(numQFamilies);
    currDevice.m_qSupportsPresent.resize(numQFamilies);

    vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &numQFamilies, currDevice.m_qFamilyProps.data());

    for (u32 q = 0; q < numQFamilies; q++) {
      const VkQueueFamilyProperties& QFamilyProp = currDevice.m_qFamilyProps[q];

      std::string msg{};

      msg += std::format("\tFamily: [{}], Num queues: [{}]", q, QFamilyProp.queueCount);
      VkQueueFlags flags = QFamilyProp.queueFlags;
      msg += std::format("\tGFX: [{}], Compute: [{}], Transfer: [{}], Sparse binding: [{}]",
        flags & VK_QUEUE_GRAPHICS_BIT       ? "Yes" : "No",
        flags & VK_QUEUE_COMPUTE_BIT        ? "Yes" : "No",
        flags & VK_QUEUE_TRANSFER_BIT       ? "Yes" : "No",
        flags & VK_QUEUE_SPARSE_BINDING_BIT ? "Yes" : "No"
      );

      dprint(msg);

      res = vkGetPhysicalDeviceSurfaceSupportKHR(physDevice, q, surface, &currDevice.m_qSupportsPresent[q]);
      CHECK_VK_RESULT(res, "vkGetPhysicalDeviceSurfaceSupportKHR");
    }

    u32 numFormats = 0;
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surface, &numFormats, nullptr);
    CHECK_VK_RESULT(res, "vkGetPhysicalDeviceSurfaceFormatsKHR (1)");
    assert(numFormats);

    currDevice.m_surfaceFormats.resize(numFormats);

    res = vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surface, &numFormats, currDevice.m_surfaceFormats.data());
    CHECK_VK_RESULT(res, "vkGetPhysicalDeviceSurfaceFormatsKHR (2)");

    // Just too much of them, uncomment when needed
    // for (u32 f = 0; f < numFormats; f++) {
    //   const VkSurfaceFormatKHR& surfaceFormat = m_devices[i].m_surfaceFormats[f];
    //   dprint("\tFormat: [{:#010x}], color space: [{:#010x}]", (s32)surfaceFormat.format, (s32)surfaceFormat.colorSpace);
    // }

    res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDevice, surface, &currDevice.m_surfaceCaps);
    CHECK_VK_RESULT(res, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

    PrintImageUsageFlags(currDevice.m_surfaceCaps.supportedUsageFlags);

    u32 numPresentModes = 0;

    res = vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice, surface, &numPresentModes, nullptr);
    CHECK_VK_RESULT(res, "vkGetPhysicalDeviceSurfacePresentModesKHR (1)");
    assert(numPresentModes);

    currDevice.m_presentModes.resize(numPresentModes);

    res = vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice, surface, &numPresentModes, currDevice.m_presentModes.data());
    CHECK_VK_RESULT(res, "vkGetPhysicalDeviceSurfacePresentModesKHR (2)");

    dprint("Num presentation modes: [{}]", numPresentModes);

    vkGetPhysicalDeviceMemoryProperties(physDevice, &currDevice.m_memProps);

    dprint("Num memory types: [{}]", currDevice.m_memProps.memoryTypeCount);

    for (u32 mt = 0; mt < currDevice.m_memProps.memoryTypeCount; mt++) {
      std::string msg = std::format("[{:<4}]: flags [{:<4}] heap [{:<4}]", mt,
        currDevice.m_memProps.memoryTypes[mt].propertyFlags,
        currDevice.m_memProps.memoryTypes[mt].heapIndex);

      msg += StringifyMemoryProperty(currDevice.m_memProps.memoryTypes[mt].propertyFlags);

      dprint(msg);
    }

    dprint("Num heap types: [{}]\n", currDevice.m_memProps.memoryHeapCount);

    vkGetPhysicalDeviceFeatures(currDevice.m_physDevice, &currDevice.m_features);

    currDevice.m_depthFormat = FindDepthFormat(physDevice);
  }
}

u32 VulkanPhysicalDevices::selectDevice(VkQueueFlags requiredQueueType, bool supportsPresent) {
  for (u32 i = 0; i < m_devices.size(); i++) {
    const PhysicalDevice& currDevice = m_devices[i];
    for (u32 j = 0; j < currDevice.m_qFamilyProps.size(); j++) {
      if ((currDevice.m_qFamilyProps[j].queueFlags & requiredQueueType) && currDevice.m_qSupportsPresent[j] == supportsPresent) {
        m_devIndex = i;
        dprint("Using GFX device [{}] and queue family [{}]", m_devIndex, j);
        return j;
      }
    }
  }

  error("[VulkanPhysicalDevices::selectDevice] Required queue type [{}] and supports present [{}] not found", requiredQueueType, supportsPresent);

  return 0;
}

const PhysicalDevice& VulkanPhysicalDevices::selected() const {
  if (m_devIndex < -1)
    error("[VulkanPhysicalDevices::selected] A physical device has not been selected");

  return m_devices[m_devIndex];
}

void VulkanPhysicalDevices::populateApiVersion(PhysicalDevice& physDevice) {
  u32 apiVer = physDevice.m_devProps.apiVersion;

  physDevice.m_apiVersion.variant = VK_API_VERSION_MAJOR(apiVer);
  physDevice.m_apiVersion.major   = VK_API_VERSION_MAJOR(apiVer);
  physDevice.m_apiVersion.minor   = VK_API_VERSION_MINOR(apiVer);
  physDevice.m_apiVersion.patch   = VK_API_VERSION_PATCH(apiVer);

  dprint("\tAPI version: {}.{}.{}.{}",
    physDevice.m_apiVersion.variant,
    physDevice.m_apiVersion.major,
    physDevice.m_apiVersion.minor,
    physDevice.m_apiVersion.patch
  );
}

void VulkanPhysicalDevices::populateExtensions(PhysicalDevice& physDevice) {
  u32 extCount;
  VkResult res = vkEnumerateDeviceExtensionProperties(physDevice.m_physDevice, nullptr, &extCount, nullptr);
  CHECK_VK_RESULT(res, "vkEnumerateDeviceExtensionProperties (1)");

  physDevice.m_extensions.resize(extCount);
  res = vkEnumerateDeviceExtensionProperties(physDevice.m_physDevice, nullptr, &extCount, physDevice.m_extensions.data());
  CHECK_VK_RESULT(res, "vkEnumerateDeviceExtensionProperties (2)");

  dprint("Supported extensions:");
  for (const auto& e : physDevice.m_extensions)
    dprint("\t{}", e.extensionName);
}

} // namespace lve

