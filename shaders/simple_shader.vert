#version 450

#define LVE_MAX_LIGHTS 10 // TODO: Implement specialization constants

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec3 a_normal;
layout(location = 3) in vec2 a_uv;

layout(location = 0) out vec3 v_color;
layout(location = 1) out vec3 v_worldPos;
layout(location = 2) out vec3 v_worldNormal;
layout(location = 3) out vec2 v_uv;

struct PointLight {
  vec4 position; // ignore w
  vec4 color; // w - intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 proj;
  mat4 view;
  vec4 lightAmbientColor;
  PointLight pointLights[LVE_MAX_LIGHTS];
  int numLights;
} ubo;

layout(push_constant) uniform Push {
  mat4 modelMatrix; // proj * view * model
  mat4 normalMatrix;
} push;

void main() {
  vec4 worldPos = push.modelMatrix * vec4(a_pos, 1.f);
  vec3 worldNormal = normalize(mat3(push.normalMatrix) * a_normal);

  gl_Position = ubo.proj * ubo.view * worldPos;

  v_color = a_color;
  v_worldPos = worldPos.xyz;
  v_worldNormal = worldNormal;
  v_uv = a_uv;
}

