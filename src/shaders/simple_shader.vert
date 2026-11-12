#version 450

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;

layout(location = 0) out vec3 v_color;

layout(push_constant) uniform Push {
  mat4 transform;
  vec3 color;
} u;

void main() {
  gl_Position = u.transform * vec4(a_pos, 1.f);
  v_color = a_color;
}

