#include "fastgltf_importer.hpp"

#include "debug.hpp"

#include "glm_element_traits.hpp"
#include "parser.hpp"
#include "tools.hpp"

#include "gtx/quaternion.hpp"

namespace pxd::ass {
bool
FastGltfImport::init(std::string_view&                          filepath,
                     std::unordered_map<std::string, Mesh>&     meshes,
                     std::unordered_map<std::string, MeshNode>& nodes,
                     std::vector<MeshNode*>                     parent_nodes)
{
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

  /////////////////////////////////////////////////////////////////////////////////
  // SCENE LOADING

  fastgltf::GltfDataBuffer data;
  data.loadFromFile(filepath);

  fastgltf::Asset       gltf;
  std::filesystem::path path = filepath;

  fastgltf::GltfType type = fastgltf::determineGltfFileType(&data);

  if (type == fastgltf::GltfType::Invalid) {
    LOG_WARNING(
      std::format("Failed to load {} glTF scene, INVALID_SCENE", filepath)
        .c_str());
    return false;
  }

  auto load = parser.loadGltf(&data, path.parent_path(), gltf_options);

  gltf = std::move(load.get());

  /////////////////////////////////////////////////////////////////////////////////
  // MESH LOADING

  std::vector<std::string> mesh_names;
  std::vector<std::string> node_names;

  for (fastgltf::Mesh& mesh : gltf.meshes) {
    Mesh new_mesh;
    new_mesh.name = mesh.name.c_str();

    for (auto&& p : mesh.primitives) {
      size_t initial_vertex = new_mesh.positions.size();

      load_indices(gltf, p, new_mesh, initial_vertex);
      load_positions(gltf, p, new_mesh, initial_vertex);
      load_normals(gltf, p, new_mesh, initial_vertex);
      load_uvs(gltf, p, new_mesh, initial_vertex);

      calculate_bounds(new_mesh, initial_vertex);
    }

    mesh_names.push_back(new_mesh.name);
    meshes[new_mesh.name] = new_mesh;
  }

  assign_transforms(nodes, meshes, mesh_names, node_names, gltf);

  /////////////////////////////////////////////////////////////////////////////////
  // PARENT & CHILDREN ASSIGNING

  for (int i = 0; i < gltf.nodes.size(); i++) {
    for (auto& c : gltf.nodes[i].children) {
      std::string child_node_name  = node_names[c];
      std::string parent_node_name = node_names[i];

      nodes[child_node_name].parent = &nodes[parent_node_name];
      nodes[parent_node_name].children.push_back(&nodes[child_node_name]);
    }
  }

  for (auto& [name, node] : nodes) {
    if (node.parent != nullptr) {
      continue;
    }

    node.reassign_transforms(glm::mat4{ 1.f });
    parent_nodes.push_back(&node);
  }

  return true;
}

void
FastGltfImport::load_indices(fastgltf::Asset&     gltf,
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
FastGltfImport::load_positions(fastgltf::Asset&     gltf,
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
FastGltfImport::load_normals(fastgltf::Asset&     gltf,
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
FastGltfImport::load_uvs(fastgltf::Asset&     gltf,
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

void
FastGltfImport::calculate_bounds(Mesh& new_mesh, size_t initial_vertex)
{
  glm::vec3 min_pos = new_mesh.positions[initial_vertex];
  glm::vec3 max_pos = min_pos;

  for (int i = initial_vertex; i < new_mesh.positions.size(); i++) {
    min_pos = glm::min(min_pos, new_mesh.positions[i]);
    max_pos = glm::max(max_pos, new_mesh.positions[i]);
  }

  new_mesh.bounds.aabb_min      = min_pos;
  new_mesh.bounds.aabb_max      = max_pos;
  new_mesh.bounds.sphere_radius = glm::length(max_pos - min_pos) / 2.f;
}

void
FastGltfImport::assign_transforms(
  std::unordered_map<std::string, MeshNode>& _nodes,
  std::unordered_map<std::string, Mesh>&     _meshes,
  std::vector<std::string>&                  _mesh_names,
  std::vector<std::string>&                  _node_names,
  fastgltf::Asset&                           gltf)
{
  for (fastgltf::Node& node : gltf.nodes) {
    MeshNode new_node;
    new_node.name = node.name.c_str();

    if (node.meshIndex.has_value()) {
      std::string mesh_name = _mesh_names[*node.meshIndex];
      new_node.meshes.push_back(&_meshes[mesh_name]);
    }

    std::visit(
      fastgltf::visitor{
        [&](fastgltf::Node::TransformMatrix matrix) {
          memcpy(&new_node.local_transform, matrix.data(), sizeof(matrix));
        },
        [&](fastgltf::TRS transform) {
          glm::vec3 tl(transform.translation[0],
                       transform.translation[1],
                       transform.translation[2]);
          glm::quat rot(transform.rotation[3],
                        transform.rotation[0],
                        transform.rotation[1],
                        transform.rotation[2]);
          glm::vec3 sc(
            transform.scale[0], transform.scale[1], transform.scale[2]);

          glm::mat4 tm = glm::translate(glm::mat4(1.f), tl);
          glm::mat4 rm = glm::toMat4(rot);
          glm::mat4 sm = glm::scale(glm::mat4(1.f), sc);

          new_node.local_transform = tm * rm * sm;
        } },
      node.transform);

    _node_names.push_back(new_node.name);
    _nodes[new_node.name] = new_node;
  }
}
}