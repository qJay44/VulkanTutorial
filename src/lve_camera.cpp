#include "lve_camera.hpp"

#include "glm/gtx/euler_angles.hpp"

namespace lve {

void LveCamera::setOrthographicProjection(float left, float right, float bottom, float top, float near, float far) {
  projectionMatrix = glm::orthoLH_ZO(left, right, bottom, top, near, far); // This one seems fine
}

void LveCamera::setPerspectiveProjection(float fovy, float aspect, float near, float far) {
  projectionMatrix = glm::perspectiveLH_ZO(fovy, aspect, near, far);
}

void LveCamera::setViewDirection(vec3 position, vec3 dir, vec3 up) {
  viewMatrix = glm::lookAtLH(position, position + dir, up);
}

void LveCamera::setViewTarget(vec3 position, vec3 target, vec3 up) {
  viewMatrix = glm::lookAtLH(position, target, up);
}

void LveCamera::setViewYXZ(vec3 position, vec3 rotation) {
  mat4 rot = glm::eulerAngleYXZ(rotation.y, rotation.x, rotation.z);
  mat4 invRot = glm::transpose(rot);
  mat4 invTrans = glm::translate(mat4(1.f), position);

  viewMatrix = invRot * invTrans;
}

const mat4& LveCamera::getProjectioinMatrix() const {
  return projectionMatrix;
}

const mat4& LveCamera::getViewMatrix() const {
  return viewMatrix;
}

} // namespace lve

