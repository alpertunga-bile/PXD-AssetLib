#include "types.hpp"

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_SIMD_AVX2
#define GLM_ENABLE_EXPERIMENTAL
#include "mat4x4.hpp"
#include "vec4.hpp"

#include "absl/flat_hash_map.hpp"

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

void
Mesh::calculate_triangles()
{
  triangles.clear();
  const size_t indices_size = indices.size();

  triangles.resize(indices_size / 3);

  for (size_t idx = 0; idx < indices_size; idx += 3) {
    Triangle t;

    t.i_0 = indices[idx];
    t.i_1 = indices[idx + 1];
    t.i_2 = indices[idx + 2];

    triangles[idx / 3] = t;
  }
}

struct QuadTriangle
{
  Triangle              main_triangle;
  std::vector<Triangle> adjacent_triangles;
};

bool
is_adjacent(Triangle& a, Triangle& b)
{
  return (
    (a.i_0 == b.i_0 && a.i_1 == b.i_1) || (a.i_0 == b.i_0 && a.i_1 == b.i_2) ||
    (a.i_0 == b.i_0 && a.i_2 == b.i_1) || (a.i_0 == b.i_0 && a.i_2 == b.i_2) ||

    (a.i_0 == b.i_1 && a.i_1 == b.i_0) || (a.i_0 == b.i_1 && a.i_1 == b.i_2) ||
    (a.i_0 == b.i_1 && a.i_2 == b.i_0) || (a.i_0 == b.i_1 && a.i_2 == b.i_2) ||

    (a.i_0 == b.i_2 && a.i_1 == b.i_1) || (a.i_0 == b.i_2 && a.i_1 == b.i_0) ||
    (a.i_0 == b.i_2 && a.i_2 == b.i_1) || (a.i_0 == b.i_2 && a.i_2 == b.i_0));
}

uint32_t
get_different_index(Triangle& a, Triangle& b)
{
  uint32_t diff_index = 0;

  if (a.i_0 == b.i_0 && a.i_1 == b.i_1) {
    diff_index = b.i_2;
  } else if (a.i_0 == b.i_0 && a.i_1 == b.i_2) {
    diff_index = b.i_1;
  } else if (a.i_0 == b.i_0 && a.i_2 == b.i_1) {
    diff_index = b.i_2;
  } else if (a.i_0 == b.i_0 && a.i_2 == b.i_2) {
    diff_index = b.i_1;
  }

  if (a.i_0 == b.i_1 && a.i_1 == b.i_0) {
    diff_index = b.i_2;
  } else if (a.i_0 == b.i_1 && a.i_1 == b.i_2) {
    diff_index = b.i_0;
  } else if (a.i_0 == b.i_1 && a.i_2 == b.i_0) {
    diff_index = b.i_2;
  } else if (a.i_0 == b.i_1 && a.i_2 == b.i_2) {
    diff_index = b.i_0;
  }

  if (a.i_0 == b.i_2 && a.i_1 == b.i_0) {
    diff_index = b.i_1;
  } else if (a.i_0 == b.i_2 && a.i_1 == b.i_1) {
    diff_index = b.i_0;
  } else if (a.i_0 == b.i_2 && a.i_2 == b.i_0) {
    diff_index = b.i_1;
  } else if (a.i_0 == b.i_2 && a.i_2 == b.i_1) {
    diff_index = b.i_0;
  }

  return diff_index;
}

void
fill_quad_triangles(const std::vector<Triangle>& triangles,
                    std::vector<QuadTriangle>&   quad_triangles)
{
  const size_t triangle_size = triangles.size();

  for (int i = 0; i < triangle_size; i++) {
    QuadTriangle qt;
    qt.main_triangle = triangles[i];

    for (int j = 0; j < triangle_size; j++) {
      Triangle tri = triangles[j];

      if (qt.main_triangle == tri || !is_adjacent(qt.main_triangle, tri)) {
        continue;
      }

      qt.adjacent_triangles.push_back(tri);
    }

    quad_triangles.push_back(qt);
  }
}

std::string
get_quad_string(Quad& q)
{
  uint32_t index[4] = { q.i_0, q.i_1, q.i_2, q.i_3 };
  std::sort(index, index + 4);

  return fmt::format("{}_{}_{}_{}", index[0], index[1], index[2], index[3]);
}

void
Mesh::calculate_quads()
{
  if (triangles.empty()) {
    calculate_triangles();
  }

  std::vector<QuadTriangle> quad_triangles;
  fill_quad_triangles(triangles, quad_triangles);

  const size_t                           quad_size = quad_triangles.size();
  absl::flat_hash_map<std::string, Quad> temp_quads;

  for (size_t i = 0; i < quad_size; i++) {
    Quad         q;
    QuadTriangle qt = quad_triangles[i];

    q.i_0 = qt.main_triangle.i_0;
    q.i_1 = qt.main_triangle.i_1;
    q.i_2 = qt.main_triangle.i_2;
    q.t1  = qt.main_triangle;

    const size_t adjacent_triangles_size = qt.adjacent_triangles.size();
    Triangle     best_match              = Triangle();
    float        min_dot                 = 1000.f;

    for (size_t j = 0; j < adjacent_triangles_size; j++) {
      Triangle adj_tri = qt.adjacent_triangles[j];

      double dot_val =
        glm::abs(glm::dot(normals[qt.main_triangle.i_0], normals[adj_tri.i_0]));

      if (dot_val < min_dot) {
        best_match = adj_tri;
        min_dot    = dot_val;
      }
    }

    q.t2  = best_match;
    q.i_3 = get_different_index(q.t1, q.t2);

    temp_quads.insert({ get_quad_string(q), q });
  }

  const size_t temp_quad_size = temp_quads.size();
  auto         it             = temp_quads.begin();
  quads.clear();
  quads.resize(temp_quad_size);

  for (int i = 0; i < temp_quad_size; i++) {
    quads[i] = it->second;
    it++;
  }
}

}