#include "application.hpp"
#include <cmath>
#include <iostream>
#include "common/detailed_exception.h"

// Computer Graphics Namespace (cg)
namespace cg {

Application::~Application() {
  glDeleteVertexArrays(1, &vao_);
  glDeleteBuffers(1, &vbo_);
  glfwTerminate();
}

Application::Application(std::string const& windowTitle, int width, int height)
    : width_(width), height_(height), vao_(0), vbo_(0) {
  // Init GLFW
  if (!glfwInit())
    THROW_DETAILED("GLFW_INITIALIZATION_FAILED", "Failed to initialize GLFW");

  // Set all the required options for GLFW
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  // Create a GLFW window object that we can use for GLFW's functions
  window_ =
      glfwCreateWindow(width_, height_, windowTitle.c_str(), nullptr, nullptr);
  if (!window_) {
    glfwTerminate();
    THROW_DETAILED("GLFW_WINDOW_CREATION_FAILED",
                   "Failed to create GLFW window");
  }
  glfwMakeContextCurrent(window_);

  // Set the required callback functions
  glfwSetKeyCallback(window_, KeyCallback);
  glfwSetFramebufferSizeCallback(window_, FramebufferSizeCallback);

  // Set this to true so GLEW knows to use a modern approach to retrieving
  // function pointers and extensions
  glewExperimental = GL_TRUE;
  // Initialize GLEW to set up the OpenGL Function pointers
  if (glewInit() != GLEW_OK) {
    glfwTerminate();
    THROW_DETAILED("GLEW_INITIALIZATION_FAILED", "Failed to initialize GLEW");
  }

  // Define the viewport dimensions
  glViewport(0, 0, width_, height_);

  // Turn off vertical synchronization
  glfwSwapInterval(1);
}

void Application::InitApplication() {
  // Create shader program using common utilities
  shaderProgram_ =
      make_shader_program(Shader(GL_VERTEX_SHADER, "shaders/vertex.glsl"),
                          Shader(GL_FRAGMENT_SHADER, "shaders/fragment.glsl"));

  InitVertices();
  InitBuffers();

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void Application::InitVertices() {
  // Define vertices with positions and colors (from original code)
  vertices_ = {// Line strip vertices (green)
               {{-0.5f, -0.2f, 0.0f}, {0.5f, 1.0f, 0.5f}},
               {{-0.3f, -0.3f, 0.0f}, {0.5f, 1.0f, 0.5f}},
               {{0.0f, -0.1f, 0.0f}, {0.5f, 1.0f, 0.5f}},
               {{0.2f, 0.0f, 0.0f}, {0.5f, 1.0f, 0.5f}},
               {{0.6f, 0.1f, 0.0f}, {0.5f, 1.0f, 0.5f}},
               {{0.5f, 0.4f, 0.0f}, {0.5f, 1.0f, 0.5f}},
               {{0.2f, 0.3f, 0.0f}, {0.5f, 1.0f, 0.5f}},
               {{0.2f, 0.0f, 0.0f}, {0.5f, 1.0f, 0.5f}},

               // Star vertices (yellow)
               {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},
               {{-0.25f, 0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}},
               {{0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},
               {{0.25f, 0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}},
               {{0.8f, 0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}},
               {{0.4f, -0.05f, 0.0f}, {1.0f, 1.0f, 0.0f}},
               {{0.6f, -0.6f, 0.0f}, {1.0f, 1.0f, 0.0f}},
               {{0.0f, -0.2f, 0.0f}, {1.0f, 1.0f, 0.0f}},
               {{-0.6f, -0.6f, 0.0f}, {1.0f, 1.0f, 0.0f}},
               {{-0.4f, -0.05f, 0.0f}, {1.0f, 1.0f, 0.0f}},
               {{-0.8f, 0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}},
               {{-0.25f, 0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}}};
}

void Application::InitBuffers() {
  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);

  // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and
  // attribute pointer(s).
  glBindVertexArray(vao_);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(Vertex),
               vertices_.data(), GL_STATIC_DRAW);

  // Position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
  glEnableVertexAttribArray(0);
  // Color attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (GLvoid*)(sizeof(vertices_[0].position)));
  glEnableVertexAttribArray(1);

  // Unbind VAO
  glBindVertexArray(0);
}

void Application::RenderLoop() {
  InitApplication();

  // Render loop
  while (!glfwWindowShouldClose(window_)) {
    // Check if any events have been activated (key pressed, mouse moved etc.)
    // and call corresponding response functions
    glfwPollEvents();

    Render();

    // Swap the screen buffers
    glfwSwapBuffers(window_);
  }
}

void Application::Render() {
  // Clear the colorbuffer
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  shaderProgram_->Use();
  // Draw the line strip
  {
    shaderProgram_->SetValue("translation", glUniform3f, 0.0f, 0.0f, 0.0f);
    shaderProgram_->SetValue("scale", glUniform3f, 1.0f, 1.0f, 1.0f);
    shaderProgram_->SetValue("angle", glUniform1f, 0.0f * M_PI);

    glBindVertexArray(vao_);
    glDrawArrays(GL_LINE_STRIP, 0, 8);
    glBindVertexArray(0);
  }
  // Draw the stars
  {
    GLfloat scaleArray[] = {0.05f, 0.04f, 0.04f, 0.06f, 0.05f, 0.03f, 0.04f};
    GLfloat angleArray[] = {-0.03f, 0.08f,  -0.03f, 0.08f,
                            0.03f,  -0.05f, -0.06f};

    for (int position = 0; position < 7; ++position) {
      shaderProgram_->SetValue(
          "translation", glUniform3f, vertices_[position].position.x,
          vertices_[position].position.y, vertices_[position].position.z);
      shaderProgram_->SetValue("scale", glUniform3f, scaleArray[position],
                               scaleArray[position], 1.0f);
      shaderProgram_->SetValue("angle", glUniform1f,
                               angleArray[position] * M_PI);

      glBindVertexArray(vao_);
      glDrawArrays(GL_TRIANGLE_FAN, 8, 12);
      glBindVertexArray(0);
    }
  }
}

void Application::FramebufferSizeCallback(GLFWwindow* window, int width,
                                          int height) {
  glViewport(0, 0, width, height);
}

void Application::KeyCallback(GLFWwindow* window, int key, int scancode,
                              int action, int mode) {
  // Is called whenever a key is pressed/released via GLFW
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }
}

}  // namespace cg
