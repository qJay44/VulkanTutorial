#include "VulkanCore.hpp"

#include "utils/clrp.hpp"

namespace lve {

namespace {

#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

constexpr std::string GetDebugSeverityStr(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
  constexpr auto clrpVerbose = clrp::clrp_t{.fg = clrp::FG::WHITE};
  constexpr auto clrpInfo    = clrp::clrp_t{.fg = clrp::FG::BLUE};
  constexpr auto clrpWarning = clrp::clrp_t{.fg = clrp::FG::YELLOW};
  constexpr auto clrpError   = clrp::clrp_t{.fg = clrp::FG::RED};

  switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: return clrp::format("Verbose", clrpVerbose);
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    return clrp::format("Info",    clrpInfo);
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: return clrp::format("Warning", clrpWarning);
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   return clrp::format("Error",   clrpError);
    default:
      error("Invalid security code: [{}]", (s32)severity);
  }

  __builtin_unreachable();
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
  std::string message = std::format(
    "Validation layer error.\n\tSeverity: [{}]\n\tType: [{:#x}]\nMessage:\n{}",
    GetDebugSeverityStr(messageSeverity), (int)messageType, pCallbackData->pMessage
  );

  message += "\tObjects:\n";
  for (u32 i = 0; i < pCallbackData->objectCount; i++) {
    message += std::format("\t[{}]\n", pCallbackData->pObjects[i].objectHandle);
  }
  message += "\n";

  puts(message.c_str());

  (void)pUserData;

  return VK_FALSE;
}

} // namespace

VulkanCore::VulkanCore() {
}

VulkanCore::~VulkanCore() {
  destroySurface();
  destroyDebugMessageCallback();
  destroyInstance();
}

void VulkanCore::init(const std::string& appName, GLFWwindow* window) {
  m_window = window;

  createInstance(appName);
  createDebugMessageCallback();
  createSurface(window);

  m_physDevices.init(m_instance, m_surface);
  m_queueFamily = m_physDevices.selectDevice(VK_QUEUE_GRAPHICS_BIT, true);
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
    // "VK_KHR_xcb_surface",
    "VK_KHR_wayland_surface",
    #endif // __linux__

    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
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
#ifdef NDEBUG
    .enabledLayerCount = static_cast<u32>(layers.size()),
    .ppEnabledLayerNames = layers.data(),
#else
    .enabledLayerCount = 0,
    .ppEnabledLayerNames = nullptr,
#endif
    .enabledExtensionCount = static_cast<u32>(extensions.size()),
    .ppEnabledExtensionNames = extensions.data()
  };

  VkResult res = vkCreateInstance(&createInfo, nullptr, &m_instance);
  CHECK_VK_RESULT(res, "vkCreateInstance");
  dprint("Created instance");
}

void VulkanCore::createDebugMessageCallback() {
  if (!enableValidationLayers)
    return;

  VkDebugUtilsMessengerCreateInfoEXT createInfo{
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .pNext = nullptr,
    .flags = 0,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,

    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,

    .pfnUserCallback = debugCallback,
    .pUserData = nullptr,
  };

  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
  VkResult res = func ? func(m_instance, &createInfo, nullptr, &m_debugMessenger) : VK_ERROR_EXTENSION_NOT_PRESENT;
  CHECK_VK_RESULT(res, "[VulkanCore::createDebugMessageCallback] Failed to set up debug messenger");
  dprint("Debug utils created");
}

void VulkanCore::createSurface(GLFWwindow* window) {
  VkResult res = glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface);
  CHECK_VK_RESULT(res, "[VulkanCore::createSurface] Can't create GLFW window surface");
  dprint("GLFW window surface created");
}

void VulkanCore::destroySurface() {
  auto func = (PFN_vkDestroySurfaceKHR)vkGetInstanceProcAddr(m_instance, "vkDestroySurfaceKHR");
  if (!func)
    error("[VulkanCore::destroySurface] Can't find address of the function");

  func(m_instance, m_surface, nullptr); // Or vkDestroySurfaceKHR(instance, surface, nullptr);
  dprint("GLFW window surface destroyed");
}

void VulkanCore::destroyInstance() {
  vkDestroyInstance(m_instance, nullptr);
  dprint("Vulkan instance destroyed");
}

void VulkanCore::destroyDebugMessageCallback() {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
  if (!func)
    error("[VulkanCore::destroyDebugMessageCallback] Can't find address of the function");

  func(m_instance, m_debugMessenger, nullptr);
  dprint("Vulkan instance destroyed");
}

} // namespace lve

