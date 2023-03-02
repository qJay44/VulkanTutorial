#include "first_app.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <stdlib.h>
#include <fstream>
#include <string>

int main() {
  lve::FirstApp app{};

  try {
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';

    std::ofstream MyErrorFile("error.txt");
    MyErrorFile << e.what();
    MyErrorFile.close();

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

