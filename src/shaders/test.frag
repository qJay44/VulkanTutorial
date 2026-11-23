#version 460

layout(location = 0) out vec4 o_color;

layout(location = 0) in vec2 v_uv;

layout(binding = 2) uniform sampler2D u_texSampler;

void main() {
  vec3 color = texture(u_texSampler, v_uv).rgb;
  o_color = vec4(color, 1.f);
}

