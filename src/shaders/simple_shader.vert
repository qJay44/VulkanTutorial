#version 450

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec3 a_normal;
layout(location = 3) in vec2 a_uv;

layout(location = 0) out vec3 v_color;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec2 v_uv;

layout(push_constant) uniform Push {
  mat4 transform; // proj * view * model
  mat4 normalMatrix;
} push;

const vec3 lightDir = normalize(vec3(1.f, -3.f, -1.f));
const float ambient = 0.02f;

void main() {
  gl_Position = push.transform * vec4(a_pos, 1.f);

  vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * a_normal);
  float lightIntensity = max(0.f, dot(normalWorldSpace, lightDir)) + ambient;

  v_color = lightIntensity * a_color;
  v_normal = a_normal;
  v_uv = a_uv;
}

