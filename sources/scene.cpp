#include "scene.hpp"

#include "assimp_importer.hpp"
#include "debug.hpp"
#include "fastgltf_importer.hpp"

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