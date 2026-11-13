#version 450

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec3 a_normal;
layout(location = 3) in vec2 a_uv;

layout(location = 0) out vec3 v_color;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec2 v_uv;

layout(push_constant) uniform Push {
  mat4 transform;
  vec3 color;
} u;

void main() {
  gl_Position = u.transform * vec4(a_pos, 1.f);
  v_color = a_color;
  v_normal = a_normal;
  v_uv = a_uv;
}

