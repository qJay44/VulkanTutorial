#pragma once

#define CHECK_VK_RESULT(res, msg) \
  if (res != VK_SUCCESS) \
    error("\n[CHECK_VK_RESULT] Error in [{}:{}]: \n[\n\n{}\n\n], code {}\n", __FILE__, __LINE__,  msg, (s32)res);

inline VkExtent2D GetWindowExtent(GLFWwindow* window) {
  int width, height;
  VkExtent2D windowExtent{};
  glfwGetWindowSize(window, &width, &height);

  windowExtent.width = static_cast<u32>(width);
  windowExtent.height = static_cast<u32>(height);

  return windowExtent;
}

