#pragma once

#include "debug.hpp"
#include "parser.hpp"

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_SIMD_AVX2
#include "mat4x4.hpp"
#include "vec4.hpp"

#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace pxd_ass {
struct Bounds
{
  glm::vec3 aabb_min;
  float     sphere_radius;
  glm::vec3 aabb_max;
  float     pad;
};

struct MeshNode
{
  std::weak_ptr<MeshNode>                parent;
  std::vector<std::shared_ptr<MeshNode>> children;

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

struct Mesh
{
  std::string_view      name;
  Bounds                bounds;
  std::vector<uint32_t> indices;

  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> uvs;
};

class Scene
{
public:
  bool init(std::string_view filepath);
  bool destroy();

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

public:
  std::unordered_map<std::string_view, Mesh>             meshes;
  std::unordered_map<std::string_view, MeshNode>         nodes;
  std::unordered_map<std::string_view, std::string_view> image_files;

  std::vector<MeshNode> top_nodes;
};
}