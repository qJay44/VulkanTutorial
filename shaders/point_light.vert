#version 450

#define LVE_MAX_LIGHTS 10 // TODO: Implement specialization constants

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.f, -1.f),
  vec2(-1.f,  1.f),
  vec2( 1.f, -1.f),
  vec2( 1.f, -1.f),
  vec2(-1.f,  1.f),
  vec2( 1.f,  1.f)
);

layout(location = 0) out vec2 v_offset;

struct PointLight {
  vec4 position; // ignore w
  vec4 color; // w - intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 proj;
  mat4 view;
  mat4 viewInv;
  vec4 lightAmbientColor;
  PointLight pointLights[LVE_MAX_LIGHTS];
  int numLights;
} ubo;

layout(push_constant) uniform Push {
  vec4 position;
  vec4 color;
  float radius;
} push;

void main() {
  vec2 offset = OFFSETS[gl_VertexIndex];
  vec3 camRight = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
  vec3 camUp    = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};

  vec3 worldPos = push.position.xyz +
    push.radius * offset.x * camRight +
    push.radius * offset.y * camUp;

  gl_Position = ubo.proj * ubo.view * vec4(worldPos, 1.f);

  v_offset = offset;
}

