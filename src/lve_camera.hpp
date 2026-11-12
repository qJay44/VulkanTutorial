#pragma once

namespace lve {

class LveCamera {
public:
  void setOrthographicProjection(float left, float right, float bottom, float top, float near, float far);
  void setPerspectiveProjection(float fovy, float aspect, float near, float far);

  const mat4& getProjectioinMatrix() const;

private:
  mat4 projectionMatrix{1.f};
};

} // namespace lve

