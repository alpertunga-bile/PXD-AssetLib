#pragma once

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_SIMD_AVX2
#define GLM_ENABLE_EXPERIMENTAL
#include "mat4x4.hpp"
#include "vec4.hpp"

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

  std::vector<Mesh*> meshes;

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

class IImporter
{
public:
  virtual bool init(std::string_view&                          filepath,
                    std::unordered_map<std::string, Mesh>&     meshes,
                    std::unordered_map<std::string, MeshNode>& nodes,
                    std::vector<MeshNode*> parent_nodes) = 0;
};
}