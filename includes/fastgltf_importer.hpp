#pragma once

#include "parser.hpp"
#include "types.hpp"

namespace pxd::ass {
class FastGltfImport : public IImporter
{
public:
  virtual bool init(std::string_view&                          filepath,
                    std::unordered_map<std::string, Mesh>&     meshes,
                    std::unordered_map<std::string, MeshNode>& nodes,
                    std::vector<MeshNode*> parent_nodes) override;

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
};
}