#version 450

// layout(location = 0) in vec2 a_pos;
// layout(location = 1) in vec3 a_color;

// layout(location = 0) out vec3 v_color;

vec2 positions[3] = vec2[] (
  vec2( 0.0f, -0.5f),
  vec2( 0.5f,  0.5f),
  vec2(-0.5f,  0.5f)
);

void main() {
  // gl_Position = vec4(a_pos, 0.f, 1.f);
  gl_Position = vec4(positions[gl_VertexIndex], 0.f, 1.f);
  // v_color = a_color;
}

