#pragma once

template<typename Texture>
struct CoreModel {
  struct Vertex {
    vec3 position;
    vec2 texCoords;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
  };

  struct BasicMeshEntry {
    u32 numIndices = 0;
    u32 numVertices = 0;
    u32 baseVertex = 0;
    u32 baseIndex = 0;
    u32 validFaces = 0;
    int materialIndex = -1;
    mat4 trans{1.f};
  };

  struct Material {
    Texture* diffuse = NULL;
  };

  std::vector<BasicMeshEntry> m_meshes;
  std::vector<Material> m_materials;
};

