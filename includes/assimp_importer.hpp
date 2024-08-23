#pragma once

#include "base_importer.hpp"

struct aiNode;
struct aiMesh;
struct aiScene;

namespace pxd::ass {
class AssimpImport : public IImporter
{
public:
  virtual auto init(std::string_view&                           filepath,
                    absl::flat_hash_map<std::string, Mesh>&     meshes,
                    absl::flat_hash_map<std::string, MeshNode>& nodes,
                    std::vector<MeshNode*>& parent_nodes) -> bool override;

private:
  void process_node(aiNode*                                     node,
                    const aiScene*                              scene,
                    absl::flat_hash_map<std::string, Mesh>&     meshes,
                    absl::flat_hash_map<std::string, MeshNode>& nodes);
  auto process_mesh(aiMesh* mesh, const aiScene* scene) -> Mesh;
  void assign_children(aiNode*                                     root_node,
                       absl::flat_hash_map<std::string, MeshNode>& nodes);
  void add_parents(absl::flat_hash_map<std::string, MeshNode>& nodes,
                   std::vector<MeshNode*>&                     parent_nodes);
};

}