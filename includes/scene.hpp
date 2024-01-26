#pragma once

#include "types.hpp"
#include <unordered_map>

namespace pxd::ass {
enum class PXD_ASS_IMPORTER : uint8_t
{
  FASTGLTF,
  ASSIMP
};

struct Scene
{
  bool init(std::string_view filepath, PXD_ASS_IMPORTER importer);
  bool destroy();

  bool get_mesh_w_name(std::string& mesh_name, Mesh& mesh);
  bool check_mesh_w_name(std::string& mesh_name);

  std::unordered_map<std::string, Mesh>             meshes;
  std::unordered_map<std::string, MeshNode>         nodes;
  std::vector<MeshNode*>                            parent_nodes;
  std::unordered_map<std::string, std::string_view> image_files;
};
}