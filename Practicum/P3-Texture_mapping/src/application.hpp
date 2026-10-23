#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include <vector>

#include "common/shader_program.hpp"
#include "common/texture.hpp"
#include "vertex.hpp"

// Computer Graphics Namespace (cg)
namespace cg {

struct VertexArrayObjectDeleter {
  void operator()(GLuint* ptr) {
    if (ptr && *ptr != 0) {
      glDeleteVertexArrays(1, ptr);
    }
  }
};

struct BufferDeleter {
  void operator()(GLuint* ptr) {
    if (ptr && *ptr != 0) {
      glDeleteBuffers(1, ptr);
    }
  }
};

class Application {
 protected:
  glm::ivec2 windowSize_;
  // вот это нужно обвязать в unique_ptr
  GLFWwindow* window_;
  // добавить делетер
  std::unique_ptr<ShaderProgram<Shader, Shader>> shaderProgram_;

  // CPU data
  std::vector<Vertex> vertices_;
  std::vector<GLuint> indices_;
  // GPU data (reference)
  // Vertex Array Object (VAO),
  // Vertex Buffer Object (VBO),
  // Element Buffer Object (IBO)
  // std::unique_ptr<GLuint, VertexArrayObjectDeleter> vao_;
  GLuint vao_, vbo_, ibo_;
  std::vector<Texture> textures_;

 public:
  ~Application();
  Application(std::string const& windowTitle, glm::ivec2 windowSize);
  void RenderLoop();

 protected:
  void InitApplication();
  void LoadCPUData();
  void LoadGPUData();

  void RenderFrame(uint32_t frameCounter);

 protected:
  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action,
                          int mode);
  static void FramebufferSizeCallback(GLFWwindow* window, int width,
                                      int height);
};

}  // namespace cg

#endif  // APPLICATION_HPP
