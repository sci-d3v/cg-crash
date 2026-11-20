#include "model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "common/detailed_exception.h"

// Computer Graphics Namespace (cg)
namespace cg {

namespace {

// Assimp stores rows. glm::mat4 takes columns, so (a1, b1, c1, d1) is column 0.
glm::mat4 AiToGlm(const aiMatrix4x4& from) {
  return glm::mat4(from.a1, from.b1, from.c1, from.d1, from.a2, from.b2,
                   from.c2, from.d2, from.a3, from.b3, from.c3, from.d3,
                   from.a4, from.b4, from.c4, from.d4);
}

}  // namespace

Model::Model(std::string const& path, bool gamma)
    : gammaCorrection(gamma) {
  loadModel(path);
}

void Model::Draw(GLuint shaderProgramID) {
  for (unsigned int i = 0; i < meshes.size(); i++) {
    meshes[i].Draw(shaderProgramID);
  }
}

void Model::loadModel(std::string const& path) {
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(
      path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                aiProcess_FlipUVs);
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    THROW_DETAILED("ASSIMP_LOAD_FAILED", importer.GetErrorString());
  }

  const auto pos = path.find_last_of("/\\");
  directory = (pos == std::string::npos) ? std::string(".") : path.substr(0, pos);

  processNode(scene->mRootNode, scene, aiMatrix4x4());
}

void Model::processNode(aiNode* node, const aiScene* scene,
                        const aiMatrix4x4& parent) {
  const aiMatrix4x4 global = parent * node->mTransformation;
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(processMesh(mesh, scene, global));
  }
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene, global);
  }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene,
                        const aiMatrix4x4& global) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<std::shared_ptr<Texture>> textures;

  const glm::mat4 xform = AiToGlm(global);
  const glm::mat3 linear(xform);

  vertices.reserve(mesh->mNumVertices);
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex{};
    const glm::vec3 position(mesh->mVertices[i].x, mesh->mVertices[i].y,
                             mesh->mVertices[i].z);

    glm::vec3 normal(0.0f);
    if (mesh->HasNormals()) {
      normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y,
                         mesh->mNormals[i].z);
    }

    if (mesh->mTextureCoords[0]) {
      vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x,
                                   mesh->mTextureCoords[0][i].y);
    }

    vertex.Position = glm::vec3(xform * glm::vec4(position, 1.f));
    vertex.Normal = linear * normal;
    vertices.push_back(vertex);
  }

  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    const aiFace& face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j]);
    }
  }

  aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
  auto append = [&](aiTextureType type, std::string const& typeName) {
    auto loaded = loadMaterialTextures(material, type, typeName);
    textures.insert(textures.end(), loaded.begin(), loaded.end());
  };
  // glTF baseColorTexture is BASE_COLOR on current Assimp and DIFFUSE on older builds.
  append(aiTextureType_BASE_COLOR, "texture_diffuse");
  append(aiTextureType_DIFFUSE, "texture_diffuse");
  append(aiTextureType_SPECULAR, "texture_specular");

  return Mesh(std::move(vertices), std::move(indices), std::move(textures));
}

std::string Model::ResolveTexturePath(std::string const& uri) const {
  if (uri.empty()) {
    THROW_DETAILED("TEXTURE_PATH_ERROR", "Assimp returned an empty texture uri");
  }

  const bool absolute = uri[0] == '/' || uri[0] == '\\' ||
                        (uri.size() > 1 && uri[1] == ':');
  if (absolute) {
    return uri;
  }
  return directory + '/' + uri;
}

std::vector<std::shared_ptr<Texture>> Model::loadMaterialTextures(
    aiMaterial* mat, aiTextureType type, std::string const& typeName) {
  std::vector<std::shared_ptr<Texture>> textures;
  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString str;
    mat->GetTexture(type, i, &str);
    const std::string filename = ResolveTexturePath(str.C_Str());

    bool skip = false;
    for (const auto& loaded : textures_loaded) {
      if (loaded->GetPath() == filename) {
        textures.push_back(loaded);
        skip = true;
        break;
      }
    }
    if (skip) {
      continue;
    }

    auto texture = std::make_shared<Texture>(filename, typeName);
    textures.push_back(texture);
    textures_loaded.push_back(texture);
  }
  return textures;
}

}  // namespace cg
