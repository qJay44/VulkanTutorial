#version 460

layout(location = 0) out vec2 v_uv;

struct Vertex {
  float x, y, z;
  float u, v;
  float normalX, normalY, normalZ;
  float tangentX, tangentY, tangentZ;
  float bitangentX, bitangentY, bitangentZ;
};

layout(binding = 0) readonly buffer Vertices {
  Vertex data[];
} ssbo_vertices;

layout(binding = 1) readonly buffer Indices {
  uint data[];
} ssbo_indices;

layout(binding = 2) readonly uniform UniformBuffer {
  mat4 WVP;
} ubo_matrices;

void main() {
  uint idx = ssbo_indices.data[gl_VertexIndex];
  Vertex vert = ssbo_vertices.data[idx];
  vec3 pos = vec3(vert.x, vert.y, vert.z);

  gl_Position = ubo_matrices.WVP * vec4(pos, 1.f);

  v_uv = vec2(vert.u, vert.v);
}

