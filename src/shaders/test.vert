#version 460

const vec2 vertices[3] = vec2[3](
  vec2(-0.7f,  0.7f),
  vec2( 0.7f,  0.7f),
  vec2( 0.0f, -0.7f)
);

void main() {
  gl_Position = vec4(vertices[gl_VertexIndex], 0.f, 1.f);
}

