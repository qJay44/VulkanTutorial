#version 450

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec3 a_normal;
layout(location = 3) in vec2 a_uv;

layout(location = 0) out vec3 v_color;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec2 v_uv;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projectionViewMatrix;
  vec4 lightAmbientColor;
  vec3 lightPos;
  vec4 lightColor;
} ubo;

layout(push_constant) uniform Push {
  mat4 modelMatrix; // proj * view * model
  mat4 normalMatrix;
} push;

void main() {
  vec4 worldPos = push.modelMatrix * vec4(a_pos, 1.f);
  gl_Position = ubo.projectionViewMatrix * worldPos;

  vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * a_normal);
  vec3 lightDir = ubo.lightPos - worldPos.xyz;
  float attenuation = 1.f / dot(lightDir, lightDir);

  vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
  vec3 lightAmbientColor = ubo.lightAmbientColor.xyz * ubo.lightAmbientColor.w;
  vec3 diffuseColor = lightColor * max(0.f, dot(normalWorldSpace, normalize(lightDir)));

  v_color = (diffuseColor + lightAmbientColor) * a_color;
  v_normal = a_normal;
  v_uv = a_uv;
}

