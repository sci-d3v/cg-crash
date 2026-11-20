#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <string>

#include "common/shader_program.hpp"
#include "scene.hpp"

// Computer Graphics Namespace (cg)
namespace cg {

class Model;

class Application {
 protected:
  GLFWwindow* window_;
  int width_, height_;
  Scene scene_;
  std::unique_ptr<ShaderProgram<Shader, Shader>> shaderProgram_;
  std::unique_ptr<Model> model_;

 public:
  ~Application();
  Application(std::string const& windowTitle, int width, int height,
              Scene scene);

  void RenderLoop();

 protected:
  void InitApplication();
  void Render();

  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action,
                          int mode);
  static void FramebufferSizeCallback(GLFWwindow* window, int width,
                                      int height);
};

}  // namespace cg

#endif  // APPLICATION_HPP
