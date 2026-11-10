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

  void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

private:
  GLFWwindow* window;
  const int width, height;
  std::string name;

private:
  void initWindow();
};

} // namespace lve

