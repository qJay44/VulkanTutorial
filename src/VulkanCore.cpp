#include "VulkanCore.hpp"

namespace lve {

VulkanCore::VulkanCore() {
}

VulkanCore::~VulkanCore() {
  vkDestroyInstance(instance, nullptr);
  dprint("[VulkanCore::~VulkanCore] Vulkan instance is destroyed");
}

void VulkanCore::init(const std::string& appName) {
  createInstance(appName);
}

void VulkanCore::createInstance(const std::string& appName) {
  std::vector<const char*> layers = {
    "VK_LAYER_KHRONOS_validation",
  };

  std::vector<const char*> extensions = {
    VK_KHR_SURFACE_EXTENSION_NAME,

    #ifdef _WIN32
    "VK_KHR_win32_surface"
    #endif // _WIN32
    #ifdef __APPLE__
    "VK_KHR_macos_surface"
    #endif // __APPLE__
    #ifdef __linux__
    // "VK_KHR_xcb_surface"
    "VK_KHR_wayland_surface"
    #endif // __linux__

    // NOTE: Don't have that
    // VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    // VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
  };

  VkApplicationInfo appInfo = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pNext = nullptr,
    .pApplicationName = appName.c_str(),
    .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
    .pEngineName = "ogldev vulkan tutorial",
    .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
    .apiVersion = VK_API_VERSION_1_0,
  };

  VkInstanceCreateInfo createInfo = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .pApplicationInfo = &appInfo,
    .enabledLayerCount = static_cast<u32>(layers.size()),
    .ppEnabledLayerNames = layers.data(),
    .enabledExtensionCount = static_cast<u32>(extensions.size()),
    .ppEnabledExtensionNames = extensions.data()
  };

  VkResult res = vkCreateInstance(&createInfo, nullptr, &instance);
  CHECK_VK_RESULT(res, "vkCreateInstance");
  dprint("[VulkanCore::createInstance] Created instance");
}

} // namespace lve

