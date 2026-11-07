#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <string>

#include "common/shader_program.hpp"

// Computer Graphics Namespace (cg)
namespace cg {

class Application {
 protected:
  GLFWwindow* window_;
  int width_, height_;
  std::unique_ptr<ShaderProgram<Shader>> computeProgram_;
  std::unique_ptr<ShaderProgram<Shader, Shader>> renderProgram_;

  unsigned int imageTexture_;
  unsigned int quadVao_, quadVbo_;

 public:
  ~Application();
  Application(std::string const& windowTitle, int width, int height);

  void InitApplication();
  void InitQuad();
  void InitTextures();

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
