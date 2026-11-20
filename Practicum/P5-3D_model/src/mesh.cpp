#include "mesh.hpp"

#include <cstddef>

// Computer Graphics Namespace (cg)
namespace cg {

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
           std::vector<std::shared_ptr<Texture>> textures)
    : vertices(std::move(vertices)),
      indices(std::move(indices)),
      textures(std::move(textures)),
      VAO(0),
      VBO(0),
      EBO(0) {
  setupMesh();
}

Mesh::~Mesh() {
  if (EBO != 0) {
    glDeleteBuffers(1, &EBO);
  }
  if (VBO != 0) {
    glDeleteBuffers(1, &VBO);
  }
  if (VAO != 0) {
    glDeleteVertexArrays(1, &VAO);
  }
}

Mesh::Mesh(Mesh&& other) noexcept
    : vertices(std::move(other.vertices)),
      indices(std::move(other.indices)),
      textures(std::move(other.textures)),
      VAO(other.VAO),
      VBO(other.VBO),
      EBO(other.EBO) {
  other.VAO = 0;
  other.VBO = 0;
  other.EBO = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  if (EBO != 0) {
    glDeleteBuffers(1, &EBO);
  }
  if (VBO != 0) {
    glDeleteBuffers(1, &VBO);
  }
  if (VAO != 0) {
    glDeleteVertexArrays(1, &VAO);
  }

  vertices = std::move(other.vertices);
  indices = std::move(other.indices);
  textures = std::move(other.textures);
  VAO = other.VAO;
  VBO = other.VBO;
  EBO = other.EBO;
  other.VAO = 0;
  other.VBO = 0;
  other.EBO = 0;
  return *this;
}

void Mesh::Draw(GLuint shaderProgramID) {
  (void)shaderProgramID;
  if (VAO == 0) {
    return;
  }

  for (const auto& texture : textures) {
    if (texture->GetType() != "texture_diffuse") {
      continue;
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->GetId());
  }

  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()),
                 GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  glActiveTexture(GL_TEXTURE0);
}

void Mesh::setupMesh() {
  if (vertices.empty() || indices.empty()) {
    return;
  }

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
               vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void*)offsetof(Vertex, Position));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void*)offsetof(Vertex, Normal));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void*)offsetof(Vertex, TexCoords));

  glBindVertexArray(0);
}

}  // namespace cg
