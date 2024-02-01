#include "scene.hpp"

#include "assimp_importer.hpp"
#include "debug.hpp"
#include "fastgltf_importer.hpp"

#include "meshoptimizer.h"

#include <filesystem>

namespace pxd::ass {
bool
Scene::init(std::string_view filepath, PXD_ASS_IMPORTER importer)
{
  if (!std::filesystem::exists(filepath)) {
    LOG_WARNING(std::format("{} is not exists", filepath).c_str());
    return false;
  }

  switch (importer) {
    case PXD_ASS_IMPORTER::FASTGLTF: {
      if (!filepath.ends_with(".gltf") && !filepath.ends_with(".glb")) {
        LOG_WARNING(
          std::format("FASTGLTF importer is selected by {} is not glTF file",
                      filepath)
            .c_str());
        return false;
      }
      FastGltfImport fastgltf_importer;
      return fastgltf_importer.init(filepath, meshes, nodes, parent_nodes);
    }
    case PXD_ASS_IMPORTER::ASSIMP: {
      AssimpImport assimp_importer;
      return assimp_importer.init(filepath, meshes, nodes, parent_nodes);
    }
    default:
      LOG_WARNING("Invalid Importer Type");
      return false;
  }
}

bool
Scene::destroy()
{
  parent_nodes.clear();
  meshes.clear();
  nodes.clear();
  image_files.clear();

  return true;
}

void
Scene::optimize_meshes()
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

bool
Scene::get_mesh_w_name(std::string& mesh_name, Mesh& mesh)
{
  if (meshes.find(mesh_name) == meshes.end()) {
    return false;
  }

  mesh = meshes[mesh_name];
  return true;
}

bool
Scene::check_mesh_w_name(std::string& mesh_name)
{
  return meshes.find(mesh_name) != meshes.end() ? true : false;
}

}