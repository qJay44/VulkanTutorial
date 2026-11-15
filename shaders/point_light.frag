#version 450

layout(location = 0) in vec2 v_offset;

layout(location = 0) out vec4 FragColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 proj;
  mat4 view;
  vec4 lightAmbientColor;
  vec3 lightPos;
  vec4 lightColor;
} ubo;

layout(push_constant) uniform Push {
  mat4 modelMatrix; // proj * view * model
  mat4 normalMatrix;
} push;

void main() {
  if (dot(v_offset, v_offset) >= 1.f)
    discard;

  vec3 finalColor = ubo.lightColor.xyz;

  FragColor = vec4(finalColor, 1.f);
}

