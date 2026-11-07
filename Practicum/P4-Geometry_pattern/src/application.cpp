#include "application.hpp"
#include "common/detailed_exception.h"

// Computer Graphics Namespace (cg)
namespace cg {

Application::~Application() {
  glDeleteTextures(1, &imageTexture_);
  glDeleteVertexArrays(1, &quadVao_);
  glDeleteBuffers(1, &quadVbo_);
  glfwTerminate();
}

Application::Application(std::string const& windowTitle, int width, int height)
    : width_(width),
      height_(height),
      imageTexture_(0),
      quadVao_(0),
      quadVbo_(0) {
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
  computeProgram_ = make_shader_program(
      Shader(GL_COMPUTE_SHADER, "shaders/geometry_pattern_cs.glsl"));
  renderProgram_ = make_shader_program(
      Shader(GL_VERTEX_SHADER, "shaders/render_vs.glsl"),
      Shader(GL_FRAGMENT_SHADER, "shaders/render_fs.glsl"));

  InitQuad();
  InitTextures();

  glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
}

void Application::InitQuad() {
  // positions (3 elements), texture coordinates (2 elements)
  float quadVertices[] = {
      -1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
      1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
  };

  glGenVertexArrays(1, &quadVao_);
  glGenBuffers(1, &quadVbo_);

  glBindVertexArray(quadVao_);

  glBindBuffer(GL_ARRAY_BUFFER, quadVbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (GLvoid*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (GLvoid*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

void Application::InitTextures() {
  glGenTextures(1, &imageTexture_);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, imageTexture_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width_, height_, 0, GL_RGBA,
               GL_FLOAT, nullptr);

  glBindImageTexture(0, imageTexture_, 0, GL_FALSE, 0, GL_READ_WRITE,
                     GL_RGBA32F);

  glClearTexImage(imageTexture_, 0, GL_RGBA, GL_FLOAT, nullptr);
}

void Application::RenderLoop() {
  InitApplication();

  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();

    Render();

    glfwSwapBuffers(window_);
  }
}

void Application::Render() {
  glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  computeProgram_->Use();
  computeProgram_->SetValue("texture_size", glUniform2f,
                            static_cast<float>(width_),
                            static_cast<float>(height_));

  glDispatchCompute(static_cast<unsigned int>(width_) / 10,
                    static_cast<unsigned int>(height_) / 10, 1);
  // make sure writing to image has finished before read
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  renderProgram_->Use();
  renderProgram_->SetValue("imageTexture", glUniform1i, 0);
  glBindVertexArray(quadVao_);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
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
