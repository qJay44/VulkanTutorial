#version 460

struct Vertex {
  float x, y, z;
  float u, v;
};

layout(binding = 0) readonly buffer Vertices {
  Vertex data[];
} b_vertices;

void main() {
  Vertex vert = b_vertices.data[gl_VertexIndex];
  vec3 pos = vec3(vert.x, vert.y, vert.z);
  gl_Position = vec4(pos, 1.f);
}

