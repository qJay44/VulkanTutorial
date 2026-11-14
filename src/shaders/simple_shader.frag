#version 450

layout(location = 0) in vec3 v_color;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;

layout(location = 0) out vec4 FragColor;

layout(push_constant) uniform Push {
  mat4 transform; // proj * view * model
  mat4 normalMatrix;
} push;

void main() {
  vec3 finalColor = v_color;

  FragColor = vec4(finalColor, 1.f);
}

