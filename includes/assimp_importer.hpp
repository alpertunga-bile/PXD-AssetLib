#pragma once

#include "assimp/scene.h"
#include "types.hpp"

namespace pxd::ass {
class AssimpImport : public IImporter
{
public:
  virtual bool init(std::string_view&                          filepath,
                    std::unordered_map<std::string, Mesh>&     meshes,
                    std::unordered_map<std::string, MeshNode>& nodes,
                    std::vector<MeshNode*> parent_nodes) override;

private:
  void process_node(aiNode*                                    node,
                    const aiScene*                             scene,
                    std::unordered_map<std::string, Mesh>&     meshes,
                    std::unordered_map<std::string, MeshNode>& nodes);
  Mesh process_mesh(aiMesh* mesh, const aiScene* scene);
  void assign_children(aiNode*                                    root_node,
                       std::unordered_map<std::string, MeshNode>& nodes);
  void add_parents(std::unordered_map<std::string, MeshNode>& nodes,
                   std::vector<MeshNode*>&                    parent_nodes);

  glm::vec3 ai2glm(aiVector3D& ai) { return glm::vec3{ ai.x, ai.y, ai.z }; }
  glm::vec2 ai2glm(aiVector2D& ai) { return glm::vec2{ ai.x, ai.y }; }
  glm::mat4 ai2glm_mat4x4(aiMatrix4x4& ai)
  {
    return glm::mat4(ai.a1,
                     ai.b1,
                     ai.c1,
                     ai.d1,
                     ai.a2,
                     ai.b2,
                     ai.c2,
                     ai.d2,
                     ai.a3,
                     ai.b3,
                     ai.c3,
                     ai.d3,
                     ai.a4,
                     ai.b4,
                     ai.c4,
                     ai.d4);
  }
};
}