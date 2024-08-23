#pragma once

#include "../third-party/PXD-STL/includes/absl/flat_hash_map.hpp"
#include "base_importer.hpp"

namespace fastgltf {
class Asset;
class Primitive;
} // namespace fastgltf

namespace pxd::ass {

struct Mesh;
struct MeshNode;

class FastGltfImport : public IImporter
{
public:
  virtual auto init(std::string_view&                           filepath,
                    absl::flat_hash_map<std::string, Mesh>&     meshes,
                    absl::flat_hash_map<std::string, MeshNode>& nodes,
                    std::vector<MeshNode*>& parent_nodes) -> bool override;

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
  void assign_transforms(absl::flat_hash_map<std::string, MeshNode>& _nodes,
                         absl::flat_hash_map<std::string, Mesh>&     _meshes,
                         std::vector<std::string>& _mesh_names,
                         std::vector<std::string>& _node_names,
                         fastgltf::Asset&          gltf);
};
}