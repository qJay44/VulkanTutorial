#pragma once

namespace lve {

class LveCamera {
public:
  void setOrthographicProjection(float left, float right, float bottom, float top, float near, float far);
  void setPerspectiveProjection(float fovy, float aspect, float near, float far);

  // NOTE: Author uses vec3(0.f, -1.f, 0.f) as up for manual calculation
  void setViewDirection(vec3 position, vec3 dir, vec3 up = vec3(0.f, 1.f, 0.f));
  void setViewTarget(vec3 position, vec3 target, vec3 up = vec3(0.f, 1.f, 0.f));
  void setViewYXZ(vec3 position, vec3 rotation);

  const mat4& getProjectioinMatrix() const;
  const mat4& getViewMatrix() const;

private:
  mat4 projectionMatrix{1.f};
  mat4 viewMatrix{1.f};
};

} // namespace lve

