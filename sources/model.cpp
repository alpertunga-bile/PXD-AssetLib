#include "model.hpp"

#include "assimp_importer.hpp"
#include "fastgltf_importer.hpp"

#include "filesystem.hpp"
#include "logger.hpp"

#include "types.hpp"

#include "meshoptimizer.h"

namespace pxd::ass {
auto
Model::init(std::string_view filepath, IMPORTER importer) -> bool
{
  if (!pxd::fs::exists(filepath.data())) {
    PXD_LOG_WARNING("{} is not exists", filepath);
    return false;
  }

  switch (importer) {
    case IMPORTER::FASTGLTF: {
      if (!filepath.ends_with(".gltf") && !filepath.ends_with(".glb")) {
        PXD_LOG_WARNING("FASTGLTF importer is selected by {} is not glTF file",
                        filepath);
        return false;
      }
      FastGltfImport fastgltf_importer;
      return fastgltf_importer.init(filepath, meshes, nodes, parent_nodes);
    }
    case IMPORTER::ASSIMP: {
      AssimpImport assimp_importer;
      return assimp_importer.init(filepath, meshes, nodes, parent_nodes);
    }
    default:
      PXD_LOG_WARNING("Invalid Importer Type");
      return false;
  }
}

auto
Model::destroy() -> bool
{
  parent_nodes.clear();
  meshes.clear();
  nodes.clear();
  image_files.clear();

  return true;
}

void
Model::optimize_meshes()
{
  for (auto& [mesh_name, mesh] : meshes) {
    size_t              index_count    = mesh.indices.size();
    size_t              total_vertices = mesh.positions.size();
    std::vector<Vertex> temp_vertices  = mesh.get_AoS();

    std::vector<unsigned int> remap(index_count);

    size_t vertex_count = meshopt_generateVertexRemap(&remap[0],
                                                      mesh.indices.data(),
                                                      index_count,
                                                      &temp_vertices[0],
                                                      total_vertices,
                                                      sizeof(Vertex));

    std::vector<Vertex> target_vertices(vertex_count);

    meshopt_remapIndexBuffer(
      mesh.indices.data(), mesh.indices.data(), index_count, &remap[0]);
    meshopt_remapVertexBuffer(target_vertices.data(),
                              &temp_vertices[0],
                              total_vertices,
                              sizeof(Vertex),
                              &remap[0]);
    meshopt_optimizeVertexCache(
      mesh.indices.data(), mesh.indices.data(), index_count, vertex_count);
    meshopt_optimizeOverdraw(mesh.indices.data(),
                             mesh.indices.data(),
                             index_count,
                             &target_vertices[0].pos.x,
                             vertex_count,
                             sizeof(Vertex),
                             1.05f);
    meshopt_optimizeVertexFetch(target_vertices.data(),
                                mesh.indices.data(),
                                index_count,
                                target_vertices.data(),
                                vertex_count,
                                sizeof(Vertex));

    mesh.from_AoS(target_vertices);
  }
}

auto
Model::get_mesh_w_name(const std::string& mesh_name, Mesh& mesh) -> bool
{
  if (!meshes.contains(mesh_name)) {
    return false;
  }

  mesh = meshes[mesh_name];
  return true;
}

auto
Model::check_mesh_w_name(const std::string& mesh_name) -> bool
{
  return meshes.contains(mesh_name);
}

void
Model::set_transform(const glm::mat4x4& new_transform)
{
  for (auto&& parent_node : parent_nodes) {
    parent_node->reassign_transforms(new_transform);
  }
}

std::vector<Mesh>
Model::get_meshes()
{
  std::vector<Mesh> mesh_vec(meshes.size());

  for (auto&& [mesh_name, mesh] : meshes) {
    mesh_vec.push_back(mesh);
  }

  return mesh_vec;
}
}