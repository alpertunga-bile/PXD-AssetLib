#include "types.hpp"

namespace pxd::ass {
std::vector<Vertex>
Mesh::get_AoS()
{
  size_t              total_vertices = positions.size();
  std::vector<Vertex> temp_vertices(positions.size());

  for (size_t i = 0; i < total_vertices; i++) {
    Vertex vert = {
      .pos    = positions[i],
      .normal = normals[i],
      .uv     = uvs[i],
    };

    temp_vertices[i] = vert;
  }

  return temp_vertices;
}

void
Mesh::from_AoS(std::vector<Vertex>& vertices)
{
  size_t total_vertices = vertices.size();

  positions.clear();
  positions.resize(total_vertices);
  normals.clear();
  normals.resize(total_vertices);
  uvs.clear();
  uvs.resize(total_vertices);

  for (size_t i = 0; i < total_vertices; i++) {
    positions[i] = vertices[i].pos;
    normals[i]   = vertices[i].normal;
    uvs[i]       = vertices[i].uv;
  }
}
}