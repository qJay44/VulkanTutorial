#include "first_app.hpp"
#include "GLFW/glfw3.h"
#include <iostream>

namespace lve {
  void FirstApp::run() {
    while (!lveWindow.shouldClose()) {
      glfwPollEvents();
    }
  }
}

