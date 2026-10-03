#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

namespace cg {

// Hidden GLFW window. Shader, program and texture tests need a current context.
class GLContext : public ::testing::Test {
 protected:
  void SetUp() override {
    if (!glfwInit()) {
      FAIL() << "Failed to initialize GLFW";
    }
    initialized_ = true;

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_ = glfwCreateWindow(1, 1, "cg_tests", nullptr, nullptr);
    if (!window_) {
      glfwTerminate();
      initialized_ = false;
      FAIL() << "Failed to create GLFW window";
    }

    glfwMakeContextCurrent(window_);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
      glfwDestroyWindow(window_);
      window_ = nullptr;
      glfwTerminate();
      initialized_ = false;
      FAIL() << "Failed to initialize GLEW";
    }

    // glewInit leaves GL_INVALID_ENUM in the error flag on a core profile.
    glGetError();
  }

  void TearDown() override {
    if (window_) {
      glfwDestroyWindow(window_);
      window_ = nullptr;
    }
    if (initialized_) {
      glfwTerminate();
      initialized_ = false;
    }
  }

  GLFWwindow* window_ = nullptr;
  bool initialized_ = false;
};

}  // namespace cg

#endif  // GL_CONTEXT_H
