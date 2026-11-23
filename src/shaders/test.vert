#version 460

layout(location = 0) out vec2 v_uv;

struct Vertex {
  float x, y, z;
  float u, v;
};

layout(binding = 0) readonly buffer Vertices {
  Vertex data[];
} ssbo_vertices;

layout(binding = 1) readonly uniform UniformBuffer {
  mat4 WVP;
} ubo_matrices;

void main() {
  Vertex vert = ssbo_vertices.data[gl_VertexIndex];
  vec3 pos = vec3(vert.x, vert.y, vert.z);
  gl_Position = ubo_matrices.WVP * vec4(pos, 1.f);
  v_uv = vec2(vert.u, vert.v);
}

