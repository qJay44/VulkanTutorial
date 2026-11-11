#version 450

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec3 a_color;

layout(push_constant) uniform Push {
  vec2 offset;
  vec3 color;
} u;

void main() {
  gl_Position = vec4(a_pos + u.offset, 0.f, 1.f);
}

