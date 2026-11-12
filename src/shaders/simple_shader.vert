#version 450

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec3 a_color;

layout(push_constant) uniform Push {
  mat2 transform;
  vec2 offset;
  vec3 color;
} u;

void main() {
  vec2 pos = u.transform * a_pos + u.offset;
  gl_Position = vec4(pos, 0.f, 1.f);
}

