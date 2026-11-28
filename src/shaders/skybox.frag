#version 460

layout(location = 0) out vec4 o_color;

layout(location = 0) in vec3 v_dir;

layout(binding = 4) uniform samplerCube u_texCubemap;

void main() {
  o_color = texture(u_texCubemap, v_dir);
}

