#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <glm/glm.hpp>

// Computer Graphics Namespace (cg)
namespace cg {

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
};

}  // namespace cg

#endif  // VERTEX_HPP
