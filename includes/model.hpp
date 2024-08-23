#pragma once

#include "../third-party/PXD-STL/includes/absl/flat_hash_map.hpp"
#include "../third-party/glm/glm/mat4x4.hpp"
#include "../third-party/glm/glm/vec4.hpp"

namespace pxd::ass {

struct Mesh;
struct MeshNode;

enum class IMPORTER : uint8_t
{
  FASTGLTF,
  ASSIMP
};

struct Model
{
  auto init(std::string_view filepath, IMPORTER importer) -> bool;
  auto destroy() -> bool;

  void optimize_meshes();

  auto get_mesh_w_name(const std::string& mesh_name, Mesh& mesh) -> bool;
  auto check_mesh_w_name(const std::string& mesh_name) -> bool;

  void set_transform(const glm::mat4x4& new_transform);

  std::vector<Mesh> get_meshes();

  absl::flat_hash_map<std::string, Mesh>        meshes       = {};
  absl::flat_hash_map<std::string, MeshNode>    nodes        = {};
  std::vector<MeshNode*>                        parent_nodes = {};
  absl::flat_hash_map<std::string, std::string> image_files  = {};
};

} // namespace pxd::ass