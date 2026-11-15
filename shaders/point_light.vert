#version 450

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.f, -1.f),
  vec2(-1.f,  1.f),
  vec2( 1.f, -1.f),
  vec2( 1.f, -1.f),
  vec2(-1.f,  1.f),
  vec2( 1.f,  1.f)
);

const float LIGHT_RADIUS = 0.05f;

layout(location = 0) out vec2 v_offset;

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
  vec2 offset = OFFSETS[gl_VertexIndex];
  vec3 camRight = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
  vec3 camUp    = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};

  vec3 worldPos = ubo.lightPos +
    LIGHT_RADIUS * offset.x * camRight +
    LIGHT_RADIUS * offset.y * camUp;

  gl_Position = ubo.proj * ubo.view * vec4(worldPos, 1.f);

  v_offset = offset;
}

