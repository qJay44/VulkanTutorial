#version 450

layout(location = 0) in vec3 v_color;

layout(location = 0) out vec4 FragColor;

layout(push_constant) uniform Push {
  mat4 transform;
  vec3 color;
} u;

void main() {
  FragColor = vec4(v_color, 1.f);
}

