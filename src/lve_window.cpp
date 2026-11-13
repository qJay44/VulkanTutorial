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

bool LveWindow::wasWindowResized() {
  return framebufferResized;
}

void LveWindow::resetWindowResizedFlag() {
  framebufferResized = false;
}

GLFWwindow* LveWindow::getGFLWwindow() const {
  return window;
}

void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
  if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
    error("[LveWindow::createWindowSurface] Failed to create window surface");
}

void LveWindow::setCursorMoveCallback(std::function<void(double,double)>&& func) {
  onCursorMove = std::move(func);
}

void LveWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
  auto lveWindow = reinterpret_cast<LveWindow*>(glfwGetWindowUserPointer(window));
  lveWindow->framebufferResized = true;
  lveWindow->width = width;
  lveWindow->height = height;
}

void LveWindow::cursorPosCallback(GLFWwindow* window, double x, double y) {
  auto lveWindow = reinterpret_cast<LveWindow*>(glfwGetWindowUserPointer(window));
  lveWindow->onCursorMove(x, y);
}

void LveWindow::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPos(window, width / 2.f, height / 2.f);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
  glfwSetCursorPosCallback(window, cursorPosCallback);
}

} // namespace lve

