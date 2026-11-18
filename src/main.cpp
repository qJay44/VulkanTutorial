#include "VulkanApp.hpp"

void GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);

  (void)scancode;
  (void)mods;
}

int main() {
  if (!glfwInit())
    error("[main.cpp] Can't initialize GLFW");

  if (!glfwVulkanSupported())
    error("[main.cpp] Current GLFW doesn't support Vulkan");

  // NOTE: Hyprland currently resizing window which calls current swapchain validation error
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  GLFWwindow* window = glfwCreateWindow(1600, 900, "MyProgram", nullptr, nullptr);
  if (!window)
    error("[main.cpp] Didn't create window");

  glfwSetKeyCallback(window, GLFW_KeyCallback);

  lve::VulkanApp* app = new lve::VulkanApp();
  app->init("VulkanApp", window);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    app->render();
  }

  app->destroy();

  glfwDestroyWindow(window);
  glfwTerminate();

  return EXIT_SUCCESS;
}

