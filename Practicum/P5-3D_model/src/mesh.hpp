#ifndef MESH_HPP
#define MESH_HPP

#include <GL/glew.h>

#include <memory>
#include <vector>

#include "common/texture.hpp"
#include "vertex.hpp"

// Computer Graphics Namespace (cg)
namespace cg {

class Mesh {
 public:
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<std::shared_ptr<Texture>> textures;
  unsigned int VAO;

  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
       std::vector<std::shared_ptr<Texture>> textures);
  ~Mesh();

  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;
  Mesh(Mesh&& other) noexcept;
  Mesh& operator=(Mesh&& other) noexcept;

  void Draw(GLuint shaderProgramID);

 private:
  unsigned int VBO, EBO;

  void setupMesh();
};

}  // namespace cg

#endif  // MESH_HPP
