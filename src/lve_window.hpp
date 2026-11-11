#pragma once

namespace lve {

class LveWindow {
public:
  LveWindow(int w, int h, std::string name);
  ~LveWindow();

  LveWindow(const LveWindow&) = delete;
  LveWindow& operator=(const LveWindow&) = delete;

  bool shouldClose();
  VkExtent2D getExtent();
  bool wasWindowResized();
  void resetWindowResizedFlag();

  void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

private:
  GLFWwindow* window;
  int width, height;
  bool framebufferResized = false;
  std::string name;

private:
  static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
  void initWindow();
};

} // namespace lve

