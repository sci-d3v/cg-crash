#ifndef MODEL_HPP
#define MODEL_HPP

#include <assimp/scene.h>

#include <memory>
#include <string>
#include <vector>

#include "mesh.hpp"

// Computer Graphics Namespace (cg)
namespace cg {

class Model {
 public:
  std::vector<std::shared_ptr<Texture>> textures_loaded;
  std::vector<Mesh> meshes;
  std::string directory;
  bool gammaCorrection;

  explicit Model(std::string const& path, bool gamma = false);
  void Draw(GLuint shaderProgramID);

 private:
  void loadModel(std::string const& path);
  void processNode(aiNode* node, const aiScene* scene,
                   const aiMatrix4x4& parent);
  Mesh processMesh(aiMesh* mesh, const aiScene* scene,
                   const aiMatrix4x4& global);
  std::string ResolveTexturePath(std::string const& uri) const;
  std::vector<std::shared_ptr<Texture>> loadMaterialTextures(
      aiMaterial* mat, aiTextureType type, std::string const& typeName);
};

}  // namespace cg

#endif  // MODEL_HPP
