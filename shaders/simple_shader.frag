#version 450

#define LVE_MAX_LIGHTS 10 // TODO: Implement specialization constants

layout(location = 0) in vec3 v_color;
layout(location = 1) in vec3 v_worldPos;
layout(location = 2) in vec3 v_worldNormal;
layout(location = 3) in vec2 v_uv;

layout(location = 0) out vec4 FragColor;

struct PointLight {
  vec4 position; // ignore w
  vec4 color; // w - intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 proj;
  mat4 view;
  vec4 lightAmbientColor;
  PointLight pointLights[LVE_MAX_LIGHTS];
  int numLights;
} ubo;

layout(push_constant) uniform Push {
  mat4 modelMatrix; // proj * view * model
  mat4 normalMatrix;
} push;

void main() {
  vec3 N = normalize(v_worldNormal);
  vec3 lightAmbientColor = ubo.lightAmbientColor.xyz * ubo.lightAmbientColor.w;
  vec3 Lo = lightAmbientColor;

  for (int i = 0; i < ubo.numLights; i++) {
    PointLight light = ubo.pointLights[i];
    vec3 lightVec = light.position.xyz - v_worldPos;
    vec3 L = normalize(lightVec);
    float attenuation = 1.f / dot(lightVec, lightVec);
    float NdotL = max(0.f, dot(N, L));

    vec3 lightColor = light.color.xyz * light.color.w * attenuation;
    vec3 diffuseColor = lightColor * NdotL;

    Lo += diffuseColor;
  }

  vec3 finalColor = Lo * v_color;

  FragColor = vec4(finalColor, 1.f);
}

