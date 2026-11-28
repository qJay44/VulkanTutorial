#version 460

layout(location = 0) out vec3 v_dir;

layout(binding = 2) readonly uniform UniformBuffer {
  mat4 VP;
} ubo_matrices;

const vec3 vertices[8] = vec3[8](
  vec3(-1.f, -1.f,  1.f),
  vec3( 1.f, -1.f,  1.f),
  vec3( 1.f,  1.f,  1.f),
  vec3(-1.f,  1.f,  1.f),

  vec3(-1.f, -1.f, -1.f),
  vec3( 1.f, -1.f, -1.f),
  vec3( 1.f,  1.f, -1.f),
  vec3(-1.f,  1.f, -1.f)
);

const uint indices[36] = uint[36](
  1, 0, 2,  3, 2, 0,
  5, 1, 6,  2, 6, 1,
  6, 7, 5,  4, 5, 7,
  0, 4, 3,  7, 3, 4,
  5, 4, 1,  0, 1, 4,
  2, 3, 6,  7, 6, 3
);

void main() {
  uint idx = indices[gl_VertexIndex];
  vec4 ndc = vec4(vertices[idx], 1.f);
  vec4 worldPos = ubo_matrices.VP * ndc;

  gl_Position = worldPos.xyww;

  v_dir = ndc.xyz;
}

