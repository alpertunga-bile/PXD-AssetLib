#include "assimp_importer.hpp"

#include "types.hpp"

#include "logger.hpp"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "mat4x4.hpp"
#include "vec2.hpp"
#include "vec3.hpp"

namespace pxd::ass {

template<typename T = aiVector3D>
auto
ai2glm_vec3(T&& ai) -> glm::vec3
{
  return glm::vec3{ ai.x, ai.y, ai.z };
}

template<typename T = aiVector2D>
auto
ai2glm_vec2(T&& ai) -> glm::vec2
{
  return glm::vec2{ ai.x, ai.y };
}

template<typename T = aiMatrix4x4>
auto
ai2glm_mat4x4(aiMatrix4x4& ai) -> glm::mat4
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

bool
AssimpImport::init(std::string_view&                           filepath,
                   absl::flat_hash_map<std::string, Mesh>&     meshes,
                   absl::flat_hash_map<std::string, MeshNode>& nodes,
                   std::vector<MeshNode*>&                     parent_nodes)
{

  Assimp::Importer importer;

  importer.SetPropertyInteger("AI_CONFIG_PP_FD_REMOVE", 1);

  unsigned int read_flags =
    aiProcess_CalcTangentSpace | aiProcess_ImproveCacheLocality |
    aiProcess_Triangulate | aiProcess_GenSmoothNormals |
    aiProcess_SplitLargeMeshes | aiProcess_FixInfacingNormals |
    aiProcess_FindDegenerates | aiProcess_GenUVCoords |
    aiProcess_OptimizeMeshes | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes;

#if defined(PXD_ASS_SPLIT_LARGE_MESHES)
  read_flags |= aiProcess_SplitLargeMeshes
#endif

    const aiScene* scene = importer.ReadFile(filepath.data(), read_flags);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    PXD_LOG_WARNING("Scene cannot load for {} with error {}",
                    filepath,
                    importer.GetErrorString());
    return false;
  }

  process_node(scene->mRootNode, scene, meshes, nodes);

  assign_children(scene->mRootNode, nodes);
  add_parents(nodes, parent_nodes);

  return true;
}

void
AssimpImport::process_node(aiNode*                                     node,
                           const aiScene*                              scene,
                           absl::flat_hash_map<std::string, Mesh>&     meshes,
                           absl::flat_hash_map<std::string, MeshNode>& nodes)
{
  aiMesh*     mesh = nullptr;
  std::string mesh_name;

  MeshNode mesh_node;
  mesh_node.name = node->mName.C_Str();
  mesh_node.meshes.reserve(node->mNumMeshes);

  for (int i = 0; i < node->mNumMeshes; ++i) {
    mesh      = scene->mMeshes[node->mMeshes[i]];
    mesh_name = mesh->mName.C_Str();

    Mesh mesh_struct;
    meshes.insert({ mesh_name, process_mesh(mesh, scene) });

    mesh_node.meshes.push_back(&meshes[mesh_name]);
  }

  mesh_node.local_transform = ai2glm_mat4x4(node->mTransformation);

  nodes.insert({ mesh_node.name, mesh_node });

  for (int i = 0; i < node->mNumChildren; ++i) {
    process_node(node->mChildren[i], scene, meshes, nodes);
  }
}

auto
AssimpImport::process_mesh(aiMesh* mesh, const aiScene* scene) -> Mesh
{
  unsigned int size = mesh->mNumVertices;

  Mesh temp_mesh;

  temp_mesh.name            = mesh->mName.C_Str();
  temp_mesh.bounds.aabb_min = ai2glm_vec3(mesh->mAABB.mMin);
  temp_mesh.bounds.aabb_max = ai2glm_vec3(mesh->mAABB.mMax);
  temp_mesh.bounds.sphere_radius =
    glm::length(temp_mesh.bounds.aabb_max - temp_mesh.bounds.aabb_min) / 2.f;

  temp_mesh.positions.resize(size);
  temp_mesh.normals.resize(size);
  temp_mesh.uvs.resize(size);

  for (unsigned int i = 0; i < size; ++i) {
    temp_mesh.positions[i] = ai2glm_vec3(mesh->mVertices[i]);

    if (mesh->HasNormals()) {
      temp_mesh.normals[i] = ai2glm_vec3(mesh->mNormals[i]);
    } else {
      temp_mesh.normals[i] = glm::vec3{ 0.f };
    }

    if (mesh->mTextureCoords[0]) {
      temp_mesh.uvs[i] = ai2glm_vec2(mesh->mTextureCoords[0][i]);
    } else {
      temp_mesh.uvs[i] = glm::vec2{ 0.f };
    }
  }

  temp_mesh.indices.reserve(mesh->mNumFaces * mesh->mFaces[0].mNumIndices);

  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];

    for (unsigned int j = 0; j < face.mNumIndices; ++j) {
      temp_mesh.indices.push_back(face.mIndices[j]);
    }
  }

  return temp_mesh;
}

void
AssimpImport::assign_children(aiNode* root_node,
                              absl::flat_hash_map<std::string, MeshNode>& nodes)
{
  for (auto&& [node_name, node] : nodes) {
    aiNode* parent_node = root_node->FindNode(node.name.c_str());

    if (parent_node == nullptr) {
      continue;
    }

    nodes[node_name].children.reserve(parent_node->mNumChildren);

    for (int i = 0; i < parent_node->mNumChildren; ++i) {
      std::string child_name = parent_node->mChildren[i]->mName.C_Str();

      nodes[child_name].parent = &nodes[node_name];
      nodes[node_name].children.push_back(&nodes[child_name]);
    }
  }
}

void
AssimpImport::add_parents(absl::flat_hash_map<std::string, MeshNode>& nodes,
                          std::vector<MeshNode*>& parent_nodes)
{
  for (auto&& [node_name, node] : nodes) {
    if (node.parent != nullptr) {
      continue;
    }

    node.reassign_transforms(glm::mat4{ 1.f });
    parent_nodes.push_back(&node);
  }
}
}