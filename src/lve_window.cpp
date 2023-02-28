#include "lve_window.hpp"
#include "GLFW/glfw3.h"

namespace lve {
  LveWindow::LveWindow(int w, int h, std::string name)
    : width{w}, height{h}, windowName{name} {
      initWindow();
  }

  LveWindow::~LveWindow() {
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  void LveWindow::initWindow() {
    glfwInit();

    // Tell glfw to not create OpenGL context since we will use Vulkan
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Window resize will be handled in a special way
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Last two params: 1) fullscreen window, 2) OpenGL context
    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
  }
}


