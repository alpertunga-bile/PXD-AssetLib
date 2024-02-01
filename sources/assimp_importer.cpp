#include "assimp_importer.hpp"

#include "debug.hpp"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

#include <filesystem>

namespace pxd::ass {
bool
AssimpImport::init(std::string_view&                          filepath,
                   std::unordered_map<std::string, Mesh>&     meshes,
                   std::unordered_map<std::string, MeshNode>& nodes,
                   std::vector<MeshNode*>&                    parent_nodes)
{

  Assimp::Importer importer;

  importer.SetPropertyInteger("AI_CONFIG_PP_FD_REMOVE", 1);

  const aiScene* scene = importer.ReadFile(
    filepath.data(),
    aiProcess_CalcTangentSpace | aiProcess_ImproveCacheLocality |
      aiProcess_Triangulate | aiProcess_GenSmoothNormals |
      aiProcess_SplitLargeMeshes | aiProcess_FixInfacingNormals |
      aiProcess_FindDegenerates | aiProcess_GenUVCoords |
      aiProcess_OptimizeMeshes | aiProcess_FlipUVs |
      aiProcess_GenBoundingBoxes);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    LOG_WARNING(std::format("Scene cannot load for {} with error {}",
                            filepath,
                            importer.GetErrorString())
                  .c_str());
    return false;
  }

  process_node(scene->mRootNode, scene, meshes, nodes);

  assign_children(scene->mRootNode, nodes);
  add_parents(nodes, parent_nodes);

  return true;
}

void
AssimpImport::process_node(aiNode*                                    node,
                           const aiScene*                             scene,
                           std::unordered_map<std::string, Mesh>&     meshes,
                           std::unordered_map<std::string, MeshNode>& nodes)
{
  aiMesh*     mesh = nullptr;
  std::string mesh_name;

  MeshNode mesh_node;
  mesh_node.name = node->mName.C_Str();

  for (int i = 0; i < node->mNumMeshes; i++) {
    mesh      = scene->mMeshes[node->mMeshes[i]];
    mesh_name = mesh->mName.C_Str();

    Mesh mesh_struct;
    meshes[mesh_name] = process_mesh(mesh, scene);

    mesh_node.meshes.push_back(&meshes[mesh_name]);
  }

  mesh_node.local_transform = ai2glm_mat4x4(node->mTransformation);

  nodes[mesh_node.name] = mesh_node;

  for (int i = 0; i < node->mNumChildren; i++) {
    process_node(node->mChildren[i], scene, meshes, nodes);
  }
}

Mesh
AssimpImport::process_mesh(aiMesh* mesh, const aiScene* scene)
{
  unsigned int size = mesh->mNumVertices;

  Mesh temp_mesh;

  temp_mesh.bounds.aabb_min = ai2glm(mesh->mAABB.mMin);
  temp_mesh.bounds.aabb_max = ai2glm(mesh->mAABB.mMax);
  temp_mesh.bounds.sphere_radius =
    glm::length(temp_mesh.bounds.aabb_max - temp_mesh.bounds.aabb_min) / 2.f;

  temp_mesh.positions.resize(size);
  temp_mesh.normals.resize(size);
  temp_mesh.uvs.resize(size);

  for (unsigned int i = 0; i < size; i++) {
    temp_mesh.positions[i] = ai2glm(mesh->mVertices[i]);

    if (mesh->HasNormals()) {
      temp_mesh.normals[i] = ai2glm(mesh->mNormals[i]);
    } else {
      temp_mesh.normals[i] = glm::vec3{ 0.f };
    }

    if (mesh->mTextureCoords[0]) {
      temp_mesh.uvs[i] = ai2glm(mesh->mTextureCoords[0][i]);
    } else {
      temp_mesh.uvs[i] = glm::vec2{ 0.f };
    }
  }

  aiFace face;

  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    face = mesh->mFaces[i];

    for (unsigned int j = 0; j < face.mNumIndices; j++) {
      temp_mesh.indices.push_back(face.mIndices[j]);
    }
  }

  return temp_mesh;
}

void
AssimpImport::assign_children(aiNode* root_node,
                              std::unordered_map<std::string, MeshNode>& nodes)
{
  for (auto& [node_name, node] : nodes) {
    aiNode* parent_node = root_node->FindNode(node.name.c_str());

    if (parent_node == nullptr) {
      continue;
    }

    for (int i = 0; i < parent_node->mNumChildren; i++) {
      std::string child_name = parent_node->mChildren[i]->mName.C_Str();

      nodes[child_name].parent = &nodes[node_name];
      nodes[node_name].children.push_back(&nodes[child_name]);
    }
  }
}

void
AssimpImport::add_parents(std::unordered_map<std::string, MeshNode>& nodes,
                          std::vector<MeshNode*>& parent_nodes)
{
  for (auto& [node_name, node] : nodes) {
    if (node.parent != nullptr) {
      continue;
    }

    node.reassign_transforms(glm::mat4{ 1.f });
    parent_nodes.push_back(&node);
  }
}
}