#include "VulkanCore.hpp"

#include "utils/clrp.hpp"
#include "wrappers.hpp"

namespace lve {

namespace {

#ifdef NDEBUG
  constexpr bool enableValidationLayers = false;
#else
  constexpr bool enableValidationLayers = true;
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
  std::string separator = "\n\n---------------------------------------------------------------------------------------------------------------------------------\n\n";

  std::string message = separator;
  message += std::format(
    "[Debug callback] Severity: [{}] Type: [{:#x}]", GetDebugSeverityStr(messageSeverity), (s32)messageType
  );

  message += std::format("\n\nMessage:\n{}\n", pCallbackData->pMessage);
  message += "\nObjects:\n";
  for (u32 i = 0; i < pCallbackData->objectCount; i++) {
    message += std::format("\t[{}]\n", pCallbackData->pObjects[i].objectHandle);
  }
  message += separator;

  puts(message.c_str());

  (void)pUserData;

  return VK_FALSE;
}

u32 ChooseNumImages(const VkSurfaceCapabilitiesKHR& capabilities) {
  u32 requestedNumImages = capabilities.minImageCount + 1;
  u32 finalNumImages = requestedNumImages;

  if (capabilities.maxImageCount > 0 && requestedNumImages > capabilities.maxImageCount)
    finalNumImages = capabilities.maxImageCount;

  return finalNumImages;
}

VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& presentModes) {
  for (const VkPresentModeKHR& pm : presentModes)
    if (pm == VK_PRESENT_MODE_MAILBOX_KHR)
      return pm;
  return VK_PRESENT_MODE_FIFO_KHR; // Always supported
}

VkSurfaceFormatKHR ChooseSurfaceFormatAndColorSpace(const std::vector<VkSurfaceFormatKHR>& surfaceFormats) {
  for (const VkSurfaceFormatKHR& sf : surfaceFormats)
    if (sf.format == VK_FORMAT_B8G8R8A8_SRGB && sf.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      return sf;
  return surfaceFormats[0];
}

VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, VkExtent2D windowExtent) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    VkExtent2D actualExtent = windowExtent;
    actualExtent.width = std::max(
        capabilities.minImageExtent.width,
        std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(
        capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
}

VkImageView CreateImageView(
  VkDevice device,
  VkImage image,
  VkFormat format,
  VkImageAspectFlags aspectFlags,
  VkImageViewType viewType,
  u32 layerCount,
  u32 mipLevels
) {
  VkImageViewCreateInfo viewCreateInfo{
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .image = image,
    .viewType = viewType,
    .format = format,
    .components = {
      .r = VK_COMPONENT_SWIZZLE_IDENTITY,
      .g = VK_COMPONENT_SWIZZLE_IDENTITY,
      .b = VK_COMPONENT_SWIZZLE_IDENTITY,
      .a = VK_COMPONENT_SWIZZLE_IDENTITY,
    },
    .subresourceRange = {
      .aspectMask = aspectFlags,
      .baseMipLevel = 0,
      .levelCount = mipLevels,
      .baseArrayLayer = 0,
      .layerCount = layerCount,
    },
  };

  VkImageView imageView;
  VkResult res = vkCreateImageView(device, &viewCreateInfo, nullptr, &imageView);
  CHECK_VK_RESULT(res, "vkCreateImageView");

  return imageView;
}

} // namespace

VulkanCore::~VulkanCore() {
  freeCommandBuffers(1, &m_copyCmdBuf);

  destroyQueue();
  destroyCommandBufferPool();
  destroySwapchain();
  destroyLogicalDevice();
  destroySurface();
  destroyDebugMessageCallback();
  destroyInstance();
}

void VulkanCore::init(const std::string& appName, GLFWwindow* window) {
  m_window = window;

  createInstance(appName);
  createDebugMessageCallback();
  createSurface(window);

  // Init physical device here, the correspondng class Creates it and Destroys it,
  // so no separate functions here
  m_physDevices.init(m_instance, m_surface);
  m_queueFamily = m_physDevices.selectDevice(VK_QUEUE_GRAPHICS_BIT, true);

  createLogicalDevice();
  createSwapchain(GetWindowExtent(window));
  createCommandBufferPool();
  createQueue();

  allocateCommandBuffers(1, &m_copyCmdBuf);
}

void VulkanCore::createCommandBufferPool() {
  VkCommandPoolCreateInfo cmdPoolCreateInfo{
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .queueFamilyIndex = m_queueFamily
  };

  VkResult res = vkCreateCommandPool(m_device, &cmdPoolCreateInfo, nullptr, &m_cmdBufPool);
  CHECK_VK_RESULT(res, "vkCreateCommandPool");
  dprint("Created command buffer pool");
}

VkDevice& VulkanCore::getDevice() {
  return m_device;
}

u32 VulkanCore::getNumImages() const {
  return static_cast<u32>(m_images.size());
}

const VkSwapchainKHR& VulkanCore::getSwapchain() const {
  return m_swapchain;
}

const VkSurfaceFormatKHR& VulkanCore::getSwapchainSurfaceFormat() const {
  return m_swapchainSurfaceFormat;
}

const VkImage& VulkanCore::getImage(u32 index) const {
  if (index >= m_images.size())
    error("[VulkanCore::getImage] Invalid image index: [{}], available images: [{}]", index, m_images.size());
  return m_images[index];
}

const VkImageView& VulkanCore::getImageView(u32 index) const {
  if (index >= m_imageViews.size())
    error("[VulkanCore::getImageView] Invalid view image index: [{}], available view images: [{}]", index, m_imageViews.size());
  return m_imageViews[index];
}

VulkanQueue* VulkanCore::getQueue() {
  return &m_queue;
}

u32 VulkanCore::getQueueFamily() const {
  return m_queueFamily;
}

void VulkanCore::allocateCommandBuffers(u32 count, VkCommandBuffer* cmdBufs) {
  VkCommandBufferAllocateInfo cmdBufAllocInfo{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .pNext = nullptr,
    .commandPool = m_cmdBufPool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = count,
  };

  VkResult res = vkAllocateCommandBuffers(m_device, &cmdBufAllocInfo, cmdBufs);
  CHECK_VK_RESULT(res, "vkCreateImageView");
  dprint("Command buffers created: [{}]", count);
}

VulkanBufferAndMemory VulkanCore::createVertexBuffer(const void *vertices, size_t size) {
  // Step 1: create the staging buffer
  VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

  VulkanBufferAndMemory stagingVertexBuffer = VulkanBufferAndMemory::createBuffer(m_device, m_physDevices.selected().m_memProps, size, usage, memProps);

  // Step 2: map the memory of the stage buffer
  void* mem = nullptr;
  VkDeviceSize offest = 0;
  VkMemoryMapFlags flags = 0;
  VkResult res = vkMapMemory(m_device, stagingVertexBuffer.m_mem, offest, stagingVertexBuffer.m_allocationSize, flags, &mem);
  CHECK_VK_RESULT(res, "vkMapMemory");

  // Step 3: copy the vertices to the staging buffer
  memcpy(mem, vertices, size);

  // Step 4: unmap/release the mapped memory
  vkUnmapMemory(m_device, stagingVertexBuffer.m_mem);

  // Step 5: create the final buffer
  usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  VulkanBufferAndMemory vertexBuffer = VulkanBufferAndMemory::createBuffer(m_device, m_physDevices.selected().m_memProps, size, usage, memProps);

  // Step 6: copy the staging buffer to the final buffer
  copyBuffer(stagingVertexBuffer.m_buffer, vertexBuffer.m_buffer, size);

  // Step 7: release the resources of the staging buffer
  stagingVertexBuffer.destroy(m_device);

  return vertexBuffer;
}

void VulkanCore::freeCommandBuffers(u32 count, const VkCommandBuffer* cmdBufs) {
  m_queue.waitIdle();
  vkFreeCommandBuffers(m_device, m_cmdBufPool, count, cmdBufs);
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
    .apiVersion = VK_API_VERSION_1_1,
  };

  VkInstanceCreateInfo createInfo = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .pApplicationInfo = &appInfo,
#ifdef NDEBUG
    .enabledLayerCount = 0,
    .ppEnabledLayerNames = nullptr,
#else
    .enabledLayerCount = static_cast<u32>(layers.size()),
    .ppEnabledLayerNames = layers.data(),
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
    .messageSeverity =
      // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
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
  dprint("Created debug utils");
}

void VulkanCore::createSurface(GLFWwindow* window) {
  VkResult res = glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface);
  CHECK_VK_RESULT(res, "[VulkanCore::createSurface] Can't create GLFW window surface");
  dprint("Created GLFW window surface");
}

void VulkanCore::createLogicalDevice() {
  float qPriorities[] = {1.f};
  const auto& physDevice = m_physDevices.selected();

  VkDeviceQueueCreateInfo qCreateInfo{
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .queueFamilyIndex = m_queueFamily,
    .queueCount = 1,
    .pQueuePriorities = &qPriorities[0],
  };

  std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
  };

  if (physDevice.m_features.geometryShader == VK_FALSE)
    error("[VulkanCore::createLogicalDevice] The Geometry Shader is not supported");

  if (physDevice.m_features.tessellationShader == VK_FALSE)
    error("[VulkanCore::createLogicalDevice] The Tessellation Shader is not supported");

  VkPhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.geometryShader = VK_TRUE;
  deviceFeatures.tessellationShader = VK_TRUE;

  VkDeviceCreateInfo deviceCreateInfo{
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .queueCreateInfoCount = 1,
    .pQueueCreateInfos = &qCreateInfo,
    .enabledLayerCount = 0,         // DEPRECATED
    .ppEnabledLayerNames = nullptr, // DEPRECATED
    .enabledExtensionCount = static_cast<u32>(deviceExtensions.size()),
    .ppEnabledExtensionNames = deviceExtensions.data(),
    .pEnabledFeatures = &deviceFeatures,
  };

  VkResult res = vkCreateDevice(physDevice.m_physDevice, &deviceCreateInfo, nullptr, &m_device);
  CHECK_VK_RESULT(res, "vkCreateDevice");
  dprint("Created logical device");
}

void VulkanCore::createSwapchain(VkExtent2D windowExtent) {
  const auto& physDevice = m_physDevices.selected();
  const auto& surfaceCaps = physDevice.m_surfaceCaps;
  const auto& presentModes = physDevice.m_presentModes;

  u32 numImages = ChooseNumImages(surfaceCaps);
  VkPresentModeKHR presentMode = ChoosePresentMode(presentModes);
  VkExtent2D swapchainExtent = ChooseSwapExtent(surfaceCaps, windowExtent);
  m_swapchainSurfaceFormat = ChooseSurfaceFormatAndColorSpace(physDevice.m_surfaceFormats);

  VkSwapchainCreateInfoKHR swapchainCreateInfo{
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .pNext = nullptr,
    .flags = 0,
    .surface = m_surface,
    .minImageCount = numImages,
    .imageFormat = m_swapchainSurfaceFormat.format,
    .imageColorSpace = m_swapchainSurfaceFormat.colorSpace,
    .imageExtent = swapchainExtent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 1,
    .pQueueFamilyIndices = &m_queueFamily,
    .preTransform = surfaceCaps.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = presentMode,
    .clipped = VK_TRUE,
    .oldSwapchain = VK_NULL_HANDLE,
  };

  VkResult res = vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, nullptr, &m_swapchain);
  CHECK_VK_RESULT(res, "vkCreateSwapchainKHR");
  dprint("Created swapchain");

  u32 numSwapchainImaages = 0;
  res = vkGetSwapchainImagesKHR(m_device, m_swapchain, &numSwapchainImaages, nullptr);
  CHECK_VK_RESULT(res, "vkGetSwapchainImagesKHR (1)");
  assert(numImages == numSwapchainImaages);
  dprint("Num images: [{}]", numSwapchainImaages);

  m_images.resize(numSwapchainImaages);
  m_imageViews.resize(numSwapchainImaages);

  res = vkGetSwapchainImagesKHR(m_device, m_swapchain, &numSwapchainImaages, m_images.data());
  CHECK_VK_RESULT(res, "vkGetSwapchainImagesKHR (2)");

  u32 layerCount = 1;
  u32 mipLevels = 1;

  for (u32 i = 0; i < numSwapchainImaages; i++)
    m_imageViews[i] = CreateImageView(
      m_device, m_images[i], m_swapchainSurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, layerCount, mipLevels
    );
}

void VulkanCore::createQueue() {
  m_queue.init(m_device, m_swapchain, m_queueFamily, 0);
}

void VulkanCore::destroyQueue() {
  m_queue.destroy();
  dprint("Destroyed queue");
}

void VulkanCore::destroyCommandBufferPool() {
  vkDestroyCommandPool(m_device, m_cmdBufPool, nullptr);
  dprint("Destroyed command buffer pool");
}

void VulkanCore::destroySwapchain() {
  for (auto& imageView : m_imageViews)
    vkDestroyImageView(m_device, imageView, nullptr);
  vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

  dprint("Destroyed image views and swapchain");
}

void VulkanCore::destroyLogicalDevice() {
  vkDestroyDevice(m_device, nullptr);
  dprint("Destroyed Logical device");
}

void VulkanCore::destroySurface() {
  auto func = (PFN_vkDestroySurfaceKHR)vkGetInstanceProcAddr(m_instance, "vkDestroySurfaceKHR");
  if (!func)
    error("[VulkanCore::destroySurface] Can't find address of the function");

  func(m_instance, m_surface, nullptr); // Or vkDestroySurfaceKHR(instance, surface, nullptr);
  dprint("Destroyed GLFW window surface");
}

void VulkanCore::destroyInstance() {
  vkDestroyInstance(m_instance, nullptr);
  dprint("Destroyed Vulkan instance");
}

void VulkanCore::destroyDebugMessageCallback() {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
  if (!func)
    error("[VulkanCore::destroyDebugMessageCallback] Can't find address of the function");

  func(m_instance, m_debugMessenger, nullptr);
  dprint("Destroyed message callback");
}

void VulkanCore::copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) {
  wrappers::beginCommandBuffer(m_copyCmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  VkBufferCopy bufferCopy{
    .srcOffset = 0,
    .dstOffset = 0,
    .size = size,
  };

  vkCmdCopyBuffer(m_copyCmdBuf, src, dst, 1, &bufferCopy);
  vkEndCommandBuffer(m_copyCmdBuf);

  m_queue.submitSync(m_copyCmdBuf);
  m_queue.waitIdle();
}

} // namespace lve

