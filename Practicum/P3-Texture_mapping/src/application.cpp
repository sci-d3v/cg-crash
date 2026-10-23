#include "application.hpp"
#include <SOIL/SOIL.h>
#include <cmath>
#include <iostream>
// #include "common/detailed_exception.h"
#include <stdexcept>

// Computer Graphics Namespace (cg)
namespace cg {

Application::~Application() {
  // Member destructors run after this body. Release GL names while the
  // context is still current, then destroy the window and GLFW.
  textures_.clear();
  shaderProgram_.reset();
  // VAO, VBO, IBO cleanup
  glDeleteBuffers(1, &vbo_);
  glDeleteBuffers(1, &ibo_);
  glDeleteVertexArrays(1, &vao_);
  vao_ = vbo_ = ibo_ = 0;

  if (window_) {
    glfwDestroyWindow(window_);
    window_ = nullptr;
  }
  glfwTerminate();
}

Application::Application(std::string const& windowTitle, glm::ivec2 windowSize)
    : windowSize_(windowSize), vao_(0), vbo_(0), ibo_(0) {
  // Init GLFW
  if (!glfwInit())
    THROW_DETAILED("GLFW_INITIALIZATION_FAILED", "Failed to initialize GLFW");

  // Set all the required options for GLFW
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  // Create a GLFW window object that we can use for GLFW's functions
  window_ = glfwCreateWindow(windowSize_.x, windowSize_.y, windowTitle.c_str(),
                             nullptr, nullptr);
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
  glViewport(0, 0, windowSize_.x, windowSize_.y);

  // Turn off vertical synchronization
  glfwSwapInterval(1);
}

void Application::InitApplication() {
  // Create shader program using common utilities
  shaderProgram_ =
      make_shader_program(Shader(GL_VERTEX_SHADER, "shaders/vertex.glsl"),
                          Shader(GL_FRAGMENT_SHADER, "shaders/fragment.glsl"));

  LoadCPUData();
  LoadGPUData();

  // textures_.reserve(2);
  // textures_.emplace_back("textures/window_star.png", "texture_diffuse");
  // textures_.emplace_back("textures/planets.png", "texture_diffuse");

  textures_.push_back({"textures/window_star.png", "texture_diffuse"});
  textures_.push_back({"textures/planets.png", "texture_diffuse"});

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void Application::LoadCPUData() {
  // Define vertices with positions, colors and texture coordinates
  vertices_ = {
      // Star
      {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {360.f / 720.f, 360.f / 720.f}},
      {{-0.25f, 0.5f, 0.0f},
       {1.0f, 1.0f, 1.0f},
       {257.f / 720.f, 329.f / 720.f}},
      {{0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {200.f / 720.f, 146.f / 720.f}},
      {{0.25f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {355.f / 720.f, 260.f / 720.f}},
      {{0.8f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {520.f / 720.f, 146.f / 720.f}},
      {{0.4f, -0.05f, 0.0f},
       {1.0f, 1.0f, 1.0f},
       {456.f / 720.f, 330.f / 720.f}},
      {{0.6f, -0.6f, 0.0f}, {1.0f, 1.0f, 1.0f}, {613.f / 720.f, 446.f / 720.f}},
      {{0.0f, -0.2f, 0.0f}, {1.0f, 1.0f, 1.0f}, {415.f / 720.f, 446.f / 720.f}},
      {{-0.6f, -0.6f, 0.0f},
       {1.0f, 1.0f, 1.0f},
       {352.f / 720.f, 627.f / 720.f}},
      {{-0.4f, -0.05f, 0.0f},
       {1.0f, 1.0f, 1.0f},
       {293.f / 720.f, 446.f / 720.f}},
      {{-0.8f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {92.f / 720.f, 446.f / 720.f}},
      // Background
      {{1.f, 1.f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.f, 0.f}},
      {{1.f, -1.f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.f, 1.f}},
      {{-1.f, -1.f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.f, 1.f}},
      {{-1.f, 1.f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.f, 0.f}},
  };

  indices_ = {// Star
              0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1,
              // Background
              11, 12, 13, 14};
}

void Application::LoadGPUData() {
  glGenVertexArrays(1, &vao_);

  glGenBuffers(1, &vbo_);
  glGenBuffers(1, &ibo_);

  // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and
  // attribute pointer(s).
  glBindVertexArray(vao_);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(Vertex),
               vertices_.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(GLuint),
               indices_.data(), GL_STATIC_DRAW);

  // Position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
  glEnableVertexAttribArray(0);
  // Color attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (GLvoid*)(sizeof(vertices_[0].position)));
  glEnableVertexAttribArray(1);
  // Texture coordinate attribute
  glVertexAttribPointer(
      2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      (GLvoid*)(sizeof(vertices_[0].position) + sizeof(vertices_[0].color)));
  glEnableVertexAttribArray(2);

  // Unbind VAO
  glBindVertexArray(0);
}

void Application::RenderLoop() {
  InitApplication();
  uint32_t frameCounter = 0;
  // Render loop
  while (!glfwWindowShouldClose(window_)) {
    // Check if any events have been activated (key pressed, mouse moved etc.)
    // and call corresponding response functions
    glfwPollEvents();

    RenderFrame(frameCounter);

    // Swap the screen buffers
    glfwSwapBuffers(window_);
    frameCounter++;
  }
}

void Application::RenderFrame(uint32_t frameCounter) {
  // Clear the colorbuffer
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  shaderProgram_->Use();

  // Draw background (planets texture)
  {
    glBindTexture(GL_TEXTURE_2D, textures_.at(1).GetId());
    shaderProgram_->SetValue("translation", glUniform3f, 0.0f, 0.0f, 0.0f);
    shaderProgram_->SetValue("scale", glUniform3f, 1.0f, 1.0f, 1.0f);
    shaderProgram_->SetValue("angle", glUniform1f, 0.0f * M_PI);

    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT,
                   (void*)(12 * sizeof(GLuint)));
    glBindVertexArray(0);
  }

  // Draw first star
  {
    glBindTexture(GL_TEXTURE_2D, textures_.at(0).GetId());
    shaderProgram_->SetValue("translation", glUniform3f, 0.6f, -0.3f, 0.0f);
    shaderProgram_->SetValue("scale", glUniform3f, 0.3f, 0.3f, 1.0f);
    shaderProgram_->SetValue("angle", glUniform1f, 0.25f * M_PI);

    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLE_FAN, 12, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }

  // Draw animated star
  {
    glBindTexture(GL_TEXTURE_2D, textures_.at(0).GetId());
    shaderProgram_->SetValue("translation", glUniform3f,
                             1.f - (frameCounter % 2000) * 0.001f, 0.7f, 0.0f);
    shaderProgram_->SetValue("scale", glUniform3f, 0.1f, 0.1f, 1.0f);
    shaderProgram_->SetValue("angle", glUniform1f, 0.6f * M_PI);

    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLE_FAN, 12, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
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
