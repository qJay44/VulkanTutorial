#version 450

layout(location = 0) in vec3 v_color;
layout(location = 1) in vec3 v_worldPos;
layout(location = 2) in vec3 v_worldNormal;
layout(location = 3) in vec2 v_uv;

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
  vec3 lightVec = ubo.lightPos - v_worldPos;
  vec3 N = normalize(v_worldNormal);
  vec3 L = normalize(lightVec);
  float attenuation = 1.f / dot(lightVec, lightVec);

  vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
  vec3 lightAmbientColor = ubo.lightAmbientColor.xyz * ubo.lightAmbientColor.w;
  vec3 diffuseColor = lightColor * max(0.f, dot(v_worldNormal, L));

  vec3 finalColor = (diffuseColor + lightAmbientColor) * v_color;

  FragColor = vec4(finalColor, 1.f);
}

