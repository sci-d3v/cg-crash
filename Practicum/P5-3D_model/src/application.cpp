#include "application.hpp"

#include "common/detailed_exception.h"
#include "model.hpp"

// Computer Graphics Namespace (cg)
namespace cg {

Application::~Application() {
  // Release GL names while the context is still current. Member destructors
  // run after this body, so drop the model and the program first.
  model_.reset();
  shaderProgram_.reset();

  if (window_) {
    glfwDestroyWindow(window_);
    window_ = nullptr;
  }
  glfwTerminate();
}

Application::Application(std::string const& windowTitle, int width, int height,
                         Scene scene)
    : window_(nullptr),
      width_(width),
      height_(height),
      scene_(std::move(scene)) {
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
  glEnable(GL_DEPTH_TEST);

  shaderProgram_ =
      make_shader_program(Shader(GL_VERTEX_SHADER, "shaders/vertex.glsl"),
                          Shader(GL_FRAGMENT_SHADER, "shaders/fragment.glsl"));

  model_ = std::make_unique<Model>(scene_.modelPath);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  shaderProgram_->Use();

  const float aspect = static_cast<float>(width_) / height_;
  const glm::mat4 projection = scene_.camera.ProjectionMatrix(aspect);
  const glm::mat4 view = scene_.camera.ViewMatrix();

  const float time = static_cast<float>(glfwGetTime());
  const glm::mat4 model = scene_.transform.Matrix(time);

  const glm::mat4 mv = view * model;
  const glm::mat3 normalMatrix{glm::vec3(mv[0]), glm::vec3(mv[1]),
                               glm::vec3(mv[2])};
  const glm::mat4 mvp = projection * mv;

  shaderProgram_->SetValue("ModelViewMatrix", glUniformMatrix4fv, 1, GL_FALSE,
                           &mv[0][0]);
  shaderProgram_->SetValue("NormalMatrix", glUniformMatrix3fv, 1, GL_FALSE,
                           &normalMatrix[0][0]);
  shaderProgram_->SetValue("ProjectionMatrix", glUniformMatrix4fv, 1, GL_FALSE,
                           &projection[0][0]);
  shaderProgram_->SetValue("MVP", glUniformMatrix4fv, 1, GL_FALSE,
                           &mvp[0][0]);

  const glm::vec3& intensity = scene_.light.intensity;
  shaderProgram_->SetValue("Light.Intensity", glUniform3f, intensity.x,
                           intensity.y, intensity.z);
  const glm::vec4 lightPos =
      view * scene_.light.WorldPosition(scene_.transform.Angle(time));
  shaderProgram_->SetValue("Light.Position", glUniform4f, lightPos.x,
                           lightPos.y, lightPos.z, lightPos.w);
  const glm::vec3& ks = scene_.material.ks;
  const glm::vec3& ka = scene_.material.ka;
  shaderProgram_->SetValue("Material.Ks", glUniform3f, ks.x, ks.y, ks.z);
  shaderProgram_->SetValue("Material.Ka", glUniform3f, ka.x, ka.y, ka.z);
  shaderProgram_->SetValue("Material.Shininess", glUniform1f,
                           scene_.material.shininess);

  model_->Draw(shaderProgram_->GetId());
}

void Application::FramebufferSizeCallback(GLFWwindow* window, int width,
                                          int height) {
  glViewport(0, 0, width, height);
}

void Application::KeyCallback(GLFWwindow* window, int key, int scancode,
                              int action, int mode) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }
}

}  // namespace cg
