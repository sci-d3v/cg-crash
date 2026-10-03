#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

// Computer Graphics Namespace (cg)
namespace cg {

struct Vertex {
  glm::vec3 position;
  glm::vec3 color;
};

struct Index {
  glm::vec3 indices;
};

struct Object3D {
  std::vector<Vertex> vertices;
  std::vector<Index> indices;
  std::vector<std::string> vertexNames;

  glm::vec3 rotation;
  glm::vec3 translation;
  glm::vec3 scale;
  bool autoRotate;
  glm::vec3 autoRotateSpeed;
};

class Application {
 protected:
  GLFWwindow* window_;
  int width_, height_;
  bool imguiReady_;

  std::vector<Object3D> objects_;

  glm::vec2 mousePos_;
  bool mouseClicked_;
  bool ctrlPressed_;
  int selectedObject_;

  float lastRayOrigin_[3];
  float lastRayDirection_[3];
  bool showRaycastDebug_;

  float globalRotationSpeed_;
  bool includeGUI_;
  bool saveScreenshotFlag_;
  int selectedVertex_;
  bool showTransformations_;

 public:
  ~Application();
  Application(std::string const& windowTitle, int width, int height);

  void InitApplication();
  void RenderLoop();
  void Render();

 protected:
  void InitObjects();

  static void MouseButtonCallback(GLFWwindow* window, int button, int action,
                                  int mods);
  static void WindowSizeCallback(GLFWwindow* window, int width, int height);
  static void FramebufferSizeCallback(GLFWwindow* window, int width,
                                      int height);

  void ApplyObjectTransform(Object3D const& obj);
  void UpdateAutoRotation(Object3D& obj);
  void ProjectToScreen(glm::vec4 position, glm::mat4 modelMatrix,
                       glm::mat4 projMatrix, glm::ivec4 viewport,
                       glm::vec2& screenPosition);
  void CreateRayFromMouse(double mouseX, double mouseY, int viewport[4],
                          float projMatrix[16], float modelMatrix[16],
                          float rayOrigin[3], float rayDirection[3]);
  bool RayTriangleIntersect(float const rayOrigin[3],
                            float const rayDirection[3], float const v0[3],
                            float const v1[3], float const v2[3], float& t);
  void TransformVertex(Object3D const& obj, float x, float y, float z,
                       float& outX, float& outY, float& outZ);
  int PerformVertexPicking(int objectIndex);
  int PerformObjectPicking();
  void RenderObject(int objectIndex, bool highlight = false);
  void RenderSelectedVertex(int selectedVertex, int objectIndex);
  std::string GenerateFileName(bool withGUI = false);
  void SaveFrameAsPNG(int width, int height, std::string const& filename = "",
                      bool withGUI = false);
  void RenderGui();
};

}  // namespace cg

#endif  // APPLICATION_HPP
