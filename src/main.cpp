#include "first_app.hpp"
#include "utils/utils.hpp"

int main() {
  lve::FirstApp app{};

  try {
    app.run();
  } catch (std::exception& e) {
    error("[main.cpp]:\n{}", e.what());
  }

  return EXIT_SUCCESS;
}

