#version 450

#define LVE_MAX_LIGHTS 10 // TODO: Implement specialization constants
#define PI 3.1415926538f

layout(location = 0) in vec2 v_offset;

layout(location = 0) out vec4 FragColor;

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
  float dist = dot(v_offset, v_offset);
  if (dist >= 1.f)
    discard;

  vec3 finalColor = push.color.xyz;
  float alpha = cos(PI * dist) * 0.5f + 0.5f;

  FragColor = vec4(finalColor, alpha);
}

