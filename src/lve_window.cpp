#include "lve_window.hpp"
#include "utils/utils.hpp"

namespace lve {

LveWindow::LveWindow(int w, int h, std::string name)
  : width(w), height(h), name(name)
{
  initWindow();
}

LveWindow::~LveWindow() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

bool LveWindow::shouldClose() {
  return glfwWindowShouldClose(window);
}

VkExtent2D LveWindow::getExtent() {
  return {
    static_cast<u32>(width),
    static_cast<u32>(height)
  };
}

void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
  if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
    error("[LveWindow::createWindowSurface] Failed to create window surface");

}

void LveWindow::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
}

} // namespace lve

