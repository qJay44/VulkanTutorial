#version 450

layout(location = 0) in vec3 v_color;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;

layout(location = 0) out vec4 FragColor;

layout(push_constant) uniform Push {
  mat4 transform;
  vec3 color;
} u;

void main() {
  const vec3 lightDir = normalize(vec3(0.f, 0.5f, 0.5f));
  float diffuse = max(0.f, dot(v_normal, lightDir));
  vec3 finalColor = v_color * diffuse;

  FragColor = vec4(finalColor, 1.f);
}

