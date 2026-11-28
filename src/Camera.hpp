struct Camera {
  vec3 pos{};
  vec2 sensitivity{2.f};
  vec3 orientation{0.f, 0.f, 1.f};
  vec3 up{0.f, 1.f, 0.f};
  float yaw = glm::radians(0.f);
  float pitch = glm::radians(0.f);
  float speed = 2.f;
  float speedMult = 2.f;

  mat4 proj{1.f};
  mat4 view{1.f};
  mat4 viewInv{1.f};

  void mouseMoveCallback(vec2 mousePos, vec2 windowSize) {
    constexpr float pi = glm::pi<float>();
    constexpr float pi_2 = pi * 0.5f;

    static bool firstMove = true;
    if (firstMove) {
      mousePos = windowSize * 0.5f;
      firstMove = false;
    }

    vec2 windowCenter = windowSize * 0.5f;
    vec2 distFromCenter = mousePos - windowCenter;

    vec2 delta = sensitivity * distFromCenter / windowCenter;
    yaw += -delta.x;
    pitch += -delta.y;

    yaw = glm::mod(yaw, 2.f * pi);
    pitch = glm::clamp(pitch, -pi_2 + 0.1f, pi_2 - 0.1f);
  }

  void setOrthographicProjection(float left, float right, float bottom, float top, float near, float far) {
    proj = glm::orthoLH_ZO(left, right, bottom, top, near, far);
  }

  void setPerspectiveProjection(float fovy, float aspect, float near, float far) {
    proj = glm::perspectiveLH_ZO(fovy, aspect, near, far);
  }

  void setViewDirection(vec3 position, vec3 dir, vec3 up) {
    view = glm::lookAtLH(position, position + dir, up);
    viewInv = glm::inverse(view);
  }

  void setViewTarget(vec3 position, vec3 target, vec3 up) {
    view = glm::lookAtLH(position, target, up);
    viewInv = glm::inverse(view);
  }

  void update() {
    orientation = normalize(vec3{
      cos(yaw) * cos(pitch),
      sin(pitch),
      sin(yaw) * cos(pitch)
    });

    setViewDirection(pos, orientation, up);
  }
};

