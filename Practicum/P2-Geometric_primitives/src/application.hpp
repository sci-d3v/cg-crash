#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

#include "common/shader_program.hpp"

// Computer Graphics Namespace (cg)
namespace cg {

// Define a struct to hold both position and color for each vertex
struct Vertex {
  glm::vec3 position;
  glm::vec3 color;
};

class Application {
 protected:
  GLFWwindow* window_;
  int width_, height_;
  std::unique_ptr<ShaderProgram<Shader, Shader>> shaderProgram_;

  unsigned int vao_, vbo_;
  std::vector<Vertex> vertices_;

 public:
  ~Application();
  Application(std::string const& windowTitle, int width, int height);

  void InitApplication();
  void InitVertices();
  void InitBuffers();

  void RenderLoop();
  void Render();

 protected:
  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action,
                          int mode);
  static void FramebufferSizeCallback(GLFWwindow* window, int width,
                                      int height);
};

}  // namespace cg

#endif  // APPLICATION_HPP
