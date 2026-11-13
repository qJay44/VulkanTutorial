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
  GLFWwindow* getGFLWwindow() const;

  void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

  void setCursorMoveCallback(std::function<void(double,double)>&& func);

private:
  GLFWwindow* window;
  int width, height;
  bool framebufferResized = false;
  std::string name;

  std::function<void(double,double)> onCursorMove{};

private:
  static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
  static void cursorPosCallback(GLFWwindow* window, double x, double y);

  void initWindow();
};

} // namespace lve

