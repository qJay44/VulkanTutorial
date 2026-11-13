#pragma once

#include "lve_game_object.hpp"

namespace lve {

class KeyboardMovementController {
public:
  struct KeyMappings {
    int moveLeft     = GLFW_KEY_A;
    int moveRight    = GLFW_KEY_D;
    int moveForward  = GLFW_KEY_W;
    int moveBackward = GLFW_KEY_S;
    int moveUp       = GLFW_KEY_SPACE;
    int moveDown     = GLFW_KEY_LEFT_CONTROL;
    int lookLeft     = GLFW_KEY_LEFT;
    int lookRight    = GLFW_KEY_RIGHT;
    int lookUp       = GLFW_KEY_UP;
    int lookDown     = GLFW_KEY_DOWN;
    int speedUp      = GLFW_KEY_LEFT_SHIFT;
  };

  KeyMappings keys{};
  float moveSpeed{3.f};
  float moveSpeedMult{3.f};
  float lookSpeed{1.5f};

public:
  void moveInPlainXZ(GLFWwindow* window, float dt, LveGameObject& gameObject);

};

} // namespace lve

