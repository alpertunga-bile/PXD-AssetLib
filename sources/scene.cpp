#include "scene.hpp"

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_SIMD_AVX2
#include "mat4x4.hpp"
#include "vec4.hpp"

#include "glm_element_traits.hpp"
#include "tools.hpp"

namespace pxd_ass {
bool
Scene::init(std::string_view filepath)
{
  LOG_INFO(std::format("Loading {} GLTF scene", filepath.data()).c_str());

  fastgltf::Parser parser(
    fastgltf::Extensions::MSFT_texture_dds |
    fastgltf::Extensions::KHR_texture_basisu |
    fastgltf::Extensions::KHR_lights_punctual |
    fastgltf::Extensions::EXT_texture_webp |
    fastgltf::Extensions::KHR_materials_volume |
    fastgltf::Extensions::KHR_materials_ior |
    fastgltf::Extensions::KHR_materials_transmission |
    fastgltf::Extensions::KHR_materials_emissive_strength |
    fastgltf::Extensions::KHR_materials_iridescence |
    fastgltf::Extensions::KHR_materials_sheen |
    fastgltf::Extensions::KHR_materials_specular);

  constexpr auto gltf_options = fastgltf::Options::DontRequireValidAssetMember |
                                fastgltf::Options::AllowDouble |
                                fastgltf::Options::LoadExternalBuffers;

  fastgltf::GltfDataBuffer data;
  data.loadFromFile(filepath);

  fastgltf::Asset       gltf;
  std::filesystem::path path = filepath;

  fastgltf::GltfType type = fastgltf::determineGltfFileType(&data);

  switch (type) {
    case fastgltf::GltfType::glTF: {
      auto load = parser.loadGltf(&data, path.parent_path(), gltf_options);

      if (load) {
        gltf = std::move(load.get());
      } else {
        LOG_WARNING(std::format("Failed to load {} glTF scene with {} error",
                                filepath,
                                fastgltf::to_underlying(load.error()))
                      .c_str());
        return false;
      }
    } break;
    case fastgltf::GltfType::GLB: {
      auto load =
        parser.loadGltfBinary(&data, path.parent_path(), gltf_options);

      if (load) {
        gltf = std::move(load.get());
      } else {
        LOG_WARNING(std::format("Failed to load {} glTF scene with {} error",
                                filepath,
                                fastgltf::to_underlying(load.error()))
                      .c_str());
        return false;
      }
    } break;
    default:
      LOG_WARNING(
        std::format("Can't define {} scene's file format", filepath).c_str());
      return false;
  }

  LOG_INFO(std::format("{} scene is loaded", filepath).c_str());

  for (fastgltf::Mesh& mesh : gltf.meshes) {
    Mesh new_mesh;

    for (auto&& p : mesh.primitives) {
      size_t initial_vertex = new_mesh.positions.size();

      load_indices(gltf, p, new_mesh, initial_vertex);

      load_positions(gltf, p, new_mesh, initial_vertex);

      load_normals(gltf, p, new_mesh, initial_vertex);

      load_uvs(gltf, p, new_mesh, initial_vertex);
    }

    meshes[mesh.name] = new_mesh;
  }

  return true;
}

bool
Scene::destroy()
{
  meshes.clear();
  nodes.clear();
  image_files.clear();
  top_nodes.clear();

  return true;
}

void
Scene::load_indices(fastgltf::Asset&     gltf,
                    fastgltf::Primitive& p,
                    Mesh&                new_mesh,
                    size_t               initial_vertex)
{
  fastgltf::Accessor& index_accessor =
    gltf.accessors[p.indicesAccessor.value()];
  new_mesh.indices.reserve(new_mesh.indices.size() + index_accessor.count);

  fastgltf::iterateAccessor<std::uint32_t>(
    gltf, index_accessor, [&](std::uint32_t idx) {
      new_mesh.indices.push_back(idx + initial_vertex);
    });
}

void
Scene::load_positions(fastgltf::Asset&     gltf,
                      fastgltf::Primitive& p,
                      Mesh&                new_mesh,
                      size_t               initial_vertex)
{
  fastgltf::Accessor& pos_accessor =
    gltf.accessors[p.findAttribute("POSITION")->second];

  new_mesh.positions.resize(new_mesh.positions.size() + pos_accessor.count);
  new_mesh.normals.resize(new_mesh.normals.size() + pos_accessor.count);
  new_mesh.uvs.resize(new_mesh.uvs.size() + pos_accessor.count);

  fastgltf::iterateAccessorWithIndex<glm::vec3>(
    gltf, pos_accessor, [&](glm::vec3 v, size_t index) {
      new_mesh.positions[initial_vertex + index] = v;
    });
}

void
Scene::load_normals(fastgltf::Asset&     gltf,
                    fastgltf::Primitive& p,
                    Mesh&                new_mesh,
                    size_t               initial_vertex)
{
  auto normals = p.findAttribute("NORMAL");
  if (normals != p.attributes.end()) {
    fastgltf::iterateAccessorWithIndex<glm::vec3>(
      gltf, gltf.accessors[(*normals).second], [&](glm::vec3 v, size_t index) {
        new_mesh.normals[initial_vertex + index] = v;
      });
  }
}

void
Scene::load_uvs(fastgltf::Asset&     gltf,
                fastgltf::Primitive& p,
                Mesh&                new_mesh,
                size_t               initial_vertex)
{
  auto uv = p.findAttribute("TEXCOORD_0");
  if (uv != p.attributes.end()) {
    fastgltf::iterateAccessorWithIndex<glm::vec2>(
      gltf, gltf.accessors[(*uv).second], [&](glm::vec2 v, size_t index) {
        new_mesh.uvs[initial_vertex + index] = v;
      });
  }
}
}