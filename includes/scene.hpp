#pragma once

#include "debug.hpp"
#include "parser.hpp"

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_SIMD_AVX2
#define GLM_ENABLE_EXPERIMENTAL
#include "mat4x4.hpp"
#include "vec4.hpp"

#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace pxd::ass {
struct Bounds
{
  glm::vec3 aabb_min;
  float     sphere_radius;
  glm::vec3 aabb_max;
  float     pad;
};

struct Mesh
{
  std::string           name;
  Bounds                bounds;
  std::vector<uint32_t> indices;

  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> uvs;
};

struct MeshNode
{
  std::string name;

  MeshNode*              parent = nullptr;
  std::vector<MeshNode*> children;

  Mesh* mesh = nullptr;

  glm::mat4 local_transform;
  glm::mat4 world_transform;

  void reassign_transforms(const glm::mat4& parent_matrix)
  {
    world_transform = parent_matrix * local_transform;
    for (auto c : children) {
      c->reassign_transforms(world_transform);
    }
  }
};

class Scene
{
public:
  bool init(std::string_view filepath);
  bool destroy();

  bool get_mesh_w_name(std::string& mesh_name, Mesh& mesh);
  bool check_mesh_w_name(std::string& mesh_name);

private:
  void load_indices(fastgltf::Asset&     gltf,
                    fastgltf::Primitive& p,
                    Mesh&                new_mesh,
                    size_t               initial_vertex);
  void load_positions(fastgltf::Asset&     gltf,
                      fastgltf::Primitive& p,
                      Mesh&                new_mesh,
                      size_t               initial_vertex);
  void load_normals(fastgltf::Asset&     gltf,
                    fastgltf::Primitive& p,
                    Mesh&                new_mesh,
                    size_t               initial_vertex);
  void load_uvs(fastgltf::Asset&     gltf,
                fastgltf::Primitive& p,
                Mesh&                new_mesh,
                size_t               initial_vertex);
  void calculate_bounds(Mesh& new_mesh, size_t initial_vertex);
  void assign_transforms(std::unordered_map<std::string, MeshNode>& _nodes,
                         std::unordered_map<std::string, Mesh>&     _meshes,
                         std::vector<std::string>&                  _mesh_names,
                         std::vector<std::string>&                  _node_names,
                         fastgltf::Asset&                           gltf);

public:
  std::unordered_map<std::string, Mesh>             meshes;
  std::unordered_map<std::string, MeshNode>         nodes;
  std::vector<MeshNode*>                            parent_nodes;
  std::unordered_map<std::string, std::string_view> image_files;
};
}