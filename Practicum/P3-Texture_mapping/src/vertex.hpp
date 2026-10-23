#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <glm/glm.hpp>

// Computer Graphics Namespace (cg)
namespace cg {

// Define a struct to hold position, color and texture coordinates for each vertex
struct Vertex {
  glm::vec3 position;
  glm::vec3 color;
  glm::vec2 textureCoordinates;
};

}  // namespace cg

#endif  // VERTEX_HPP
