#include "VulkanApp.hpp"

int main() {
  if (!glfwInit())
    error("[main.cpp] Can't initialize GLFW");

  if (!glfwVulkanSupported())
    error("[main.cpp] Current GLFW doesn't support Vulkan");

  {
    lve::VulkanApp app("VulkanApp");
    app.render();
  }

  glfwTerminate();

  return EXIT_SUCCESS;
}

