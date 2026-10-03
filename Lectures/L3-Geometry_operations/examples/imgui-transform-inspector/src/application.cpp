#include "application.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <limits>
#include <numbers>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>

#include "common/detailed_exception.h"

// Computer Graphics Namespace (cg)
namespace cg {

Application::~Application() {
  if (imguiReady_) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }
  glfwTerminate();
}

Application::Application(std::string const& windowTitle, int width, int height)
    : window_(nullptr),
      width_(width),
      height_(height),
      imguiReady_(false),
      mousePos_(0.0f, 0.0f),
      mouseClicked_(false),
      ctrlPressed_(false),
      selectedObject_(-1),
      lastRayOrigin_{0.0f, 0.0f, 0.0f},
      lastRayDirection_{0.0f, 0.0f, 1.0f},
      showRaycastDebug_(false),
      globalRotationSpeed_(1.0f),
      includeGUI_(false),
      saveScreenshotFlag_(false),
      selectedVertex_(-1),
      showTransformations_(true) {
  if (!glfwInit())
    THROW_DETAILED("GLFW_INITIALIZATION_FAILED", "Failed to initialize GLFW");

  window_ =
      glfwCreateWindow(width_, height_, windowTitle.c_str(), nullptr, nullptr);
  if (!window_) {
    glfwTerminate();
    THROW_DETAILED("GLFW_WINDOW_CREATION_FAILED",
                   "Failed to create GLFW window");
  }
  glfwMakeContextCurrent(window_);
  glfwSetWindowUserPointer(window_, this);

  glfwSetMouseButtonCallback(window_, MouseButtonCallback);
  glfwSetWindowSizeCallback(window_, WindowSizeCallback);
  glfwSetFramebufferSizeCallback(window_, FramebufferSizeCallback);

  glViewport(0, 0, width_, height_);
}

void Application::InitObjects() {
  std::vector<std::string> cubeVertexNames = {
      "Front left bottom", "Front right bottom", "Front right top",
      "Front left top",    "Back left bottom",   "Back right bottom",
      "Back right top",    "Back left top"};

  std::vector<Vertex> cubeVertices = {
      // Front face
      {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
      {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
      {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}},
      // Back face
      {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},
      {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}},
      {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
      {{-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}}};

  std::vector<Index> cubeIndices = {
      // Front face
      {{0, 1, 2}},
      {{2, 3, 0}},
      // Back face
      {{4, 5, 6}},
      {{6, 7, 4}},
      // Left face
      {{7, 3, 0}},
      {{0, 4, 7}},
      // Right face
      {{1, 5, 6}},
      {{6, 2, 1}},
      // Top face
      {{3, 2, 6}},
      {{6, 7, 3}},
      // Bottom face
      {{0, 1, 5}},
      {{5, 4, 0}}};

  const float goldenRatio = (1.f + std::sqrt(5.f)) * .5f;
  const float icosaScale = 0.5f;

  std::vector<Vertex> icosahedronVertices = {
      {{-icosaScale, goldenRatio * icosaScale, 0}, {1.0f, 0.0f, 0.0f}},
      {{icosaScale, goldenRatio * icosaScale, 0}, {0.0f, 1.0f, 0.0f}},
      {{-icosaScale, -goldenRatio * icosaScale, 0}, {0.0f, 0.0f, 1.0f}},
      {{icosaScale, -goldenRatio * icosaScale, 0}, {1.0f, 1.0f, 0.0f}},
      {{0, -icosaScale, goldenRatio * icosaScale}, {0.0f, 0.0f, 1.0f}},
      {{0, icosaScale, goldenRatio * icosaScale}, {1.0f, 0.0f, 0.0f}},
      {{0, -icosaScale, -goldenRatio * icosaScale}, {0.0f, 1.0f, 0.0f}},
      {{0, icosaScale, -goldenRatio * icosaScale}, {1.0f, 1.0f, 0.0f}},
      {{goldenRatio * icosaScale, 0, -icosaScale}, {0.0f, 0.0f, 1.0f}},
      {{goldenRatio * icosaScale, 0, icosaScale}, {1.0f, 0.0f, 0.0f}},
      {{-goldenRatio * icosaScale, 0, -icosaScale}, {0.0f, 1.0f, 0.0f}},
      {{-goldenRatio * icosaScale, 0, icosaScale}, {1.0f, 1.0f, 0.0f}}};

  std::vector<Index> icosahedronIndices = {
      // 5 граней вокруг точки 0
      {{0, 11, 5}},
      {{0, 5, 1}},
      {{0, 1, 7}},
      {{0, 7, 10}},
      {{0, 10, 11}},
      // 5 смежных граней
      {{1, 5, 9}},
      {{5, 11, 4}},
      {{11, 10, 2}},
      {{10, 7, 6}},
      {{7, 1, 8}},
      // 5 граней вокруг точки 3
      {{3, 9, 4}},
      {{3, 4, 2}},
      {{3, 2, 6}},
      {{3, 6, 8}},
      {{3, 8, 9}},
      // 5 смежных граней
      {{4, 9, 5}},
      {{2, 4, 11}},
      {{6, 2, 10}},
      {{8, 6, 7}},
      {{9, 8, 1}}};

  std::vector<std::string> icosahedronVertexNames = {
      "Верх A",    "Верх B",      "Низ A",    "Низ B",
      "Перед-низ", "Перед-верх",  "Зад-низ",  "Зад-верх",
      "Право-зад", "Право-перед", "Лево-зад", "Лево-перед"};

  objects_ = {
      {cubeVertices,
       cubeIndices,
       cubeVertexNames,
       {0.0f, 0.0f, 0.0f},
       {-1.5f, 0.0f, 0.0f},
       {1.0f, 1.0f, 1.0f},
       false,
       {0.5f, 0.3f, 0.2f}},
      {icosahedronVertices,
       icosahedronIndices,
       icosahedronVertexNames,
       {0.0f, 0.0f, 0.0f},
       {1.5f, 0.0f, 0.0f},
       {1.0f, 1.0f, 1.0f},
       false,
       {0.3f, 0.5f, 0.4f}}};
}

void Application::InitApplication() {
  InitObjects();

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  int width, height;
  glfwGetFramebufferSize(window_, &width, &height);
  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  float aspect = static_cast<float>(width) / static_cast<float>(height);
  glFrustum(-aspect, aspect, -1.0f, 1.0f, 2.0f, 10.0f);

  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(window_, true);
  ImGui_ImplOpenGL3_Init("#version 130");
  imguiReady_ = true;
}

void Application::RenderLoop() {
  InitApplication();

  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
    Render();
    glfwSwapBuffers(window_);
  }
}

void Application::ApplyObjectTransform(Object3D const& obj) {
  glTranslatef(obj.translation.x, obj.translation.y, obj.translation.z);
  glRotatef(obj.rotation.x, 1.0f, 0.0f, 0.0f);
  glRotatef(obj.rotation.y, 0.0f, 1.0f, 0.0f);
  glRotatef(obj.rotation.z, 0.0f, 0.0f, 1.0f);
  glScalef(obj.scale.x, obj.scale.y, obj.scale.z);
}

void Application::UpdateAutoRotation(Object3D& obj) {
  if (obj.autoRotate) {
    obj.rotation += obj.autoRotateSpeed;
    auto normalizeAngle = [](float angle) {
      while (angle >= 360.0f) angle -= 360.0f;
      while (angle < 0.0f) angle += 360.0f;
      return angle;
    };
    obj.rotation.x = normalizeAngle(obj.rotation.x);
    obj.rotation.y = normalizeAngle(obj.rotation.y);
    obj.rotation.z = normalizeAngle(obj.rotation.z);
  }
}

void Application::MouseButtonCallback(GLFWwindow* window, int button, int action,
                                      int mods) {
  auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    if (!ImGui::GetIO().WantCaptureMouse) {
      double xpos, ypos;
      glfwGetCursorPos(window, &xpos, &ypos);

      glm::ivec2 winSize, fbSize;
      glfwGetWindowSize(window, &winSize.x, &winSize.y);
      glfwGetFramebufferSize(window, &fbSize.x, &fbSize.y);

      app->mousePos_.x = static_cast<float>(xpos) *
                         static_cast<float>(fbSize.x) /
                         static_cast<float>(winSize.x);
      app->mousePos_.y = static_cast<float>(ypos) *
                         static_cast<float>(fbSize.y) /
                         static_cast<float>(winSize.y);

      app->mouseClicked_ = true;
      app->ctrlPressed_ = (mods & GLFW_MOD_CONTROL) != 0;
    }
  }
}

void Application::WindowSizeCallback(GLFWwindow* window, int width, int height) {
  (void)width;
  (void)height;
  glm::ivec2 fbSize;
  glfwGetFramebufferSize(window, &fbSize.x, &fbSize.y);
  FramebufferSizeCallback(window, fbSize.x, fbSize.y);
}

void Application::FramebufferSizeCallback(GLFWwindow* window, int width,
                                          int height) {
  (void)window;
  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  float aspect = static_cast<float>(width) / static_cast<float>(height);
  glFrustum(-aspect, aspect, -1.0f, 1.0f, 2.0f, 10.0f);

  glMatrixMode(GL_MODELVIEW);
}

void Application::ProjectToScreen(glm::vec4 position, glm::mat4 modelMatrix,
                                  glm::mat4 projMatrix, glm::ivec4 viewport,
                                  glm::vec2& screenPosition) {
  glm::vec4 result = modelMatrix * position;
  glm::vec4 projected = projMatrix * result;

  if (projected[3] != 0.0f && std::fabs(projected[3]) > 1e-6f) {
    float ndcX = projected[0] / projected[3];
    float ndcY = projected[1] / projected[3];

    screenPosition.x = (ndcX + 1.0f) * 0.5f * viewport[2] + viewport[0];
    screenPosition.y = (1.0f - ndcY) * 0.5f * viewport[3] + viewport[1];
  } else {
    screenPosition.x = -10000.0f;
    screenPosition.y = -10000.0f;
  }
}

void Application::CreateRayFromMouse(double mouseX, double mouseY,
                                     int viewport[4], float projMatrix[16],
                                     float modelMatrix[16], float rayOrigin[3],
                                     float rayDirection[3]) {
  (void)modelMatrix;

  float x = (2.0f * static_cast<float>(mouseX)) / viewport[2] - 1.0f;
  float y = 1.0f - (2.0f * static_cast<float>(mouseY)) / viewport[3];

  float w_near = -2.0f / (projMatrix[14] + projMatrix[10]);
  float w_far = -2.0f / (projMatrix[14] - projMatrix[10]);

  float eye_near[3] = {x * (-w_near) / projMatrix[0],
                       y * (-w_near) / projMatrix[5], w_near};

  float eye_far[3] = {x * (-w_far) / projMatrix[0],
                      y * (-w_far) / projMatrix[5], w_far};

  rayOrigin[0] = eye_near[0];
  rayOrigin[1] = eye_near[1];
  rayOrigin[2] = eye_near[2] - 5.0f;

  rayDirection[0] = eye_far[0] - eye_near[0];
  rayDirection[1] = eye_far[1] - eye_near[1];
  rayDirection[2] = eye_far[2] - eye_near[2];

  float length = std::sqrt(rayDirection[0] * rayDirection[0] +
                           rayDirection[1] * rayDirection[1] +
                           rayDirection[2] * rayDirection[2]);
  if (length > 0.0f) {
    rayDirection[0] /= length;
    rayDirection[1] /= length;
    rayDirection[2] /= length;
  }
}

bool Application::RayTriangleIntersect(float const rayOrigin[3],
                                       float const rayDirection[3],
                                       float const v0[3], float const v1[3],
                                       float const v2[3], float& t) {
  const float EPSILON = 0.0000001f;
  float edge1[3], edge2[3], h[3], s[3], q[3];
  float a, f, u, v;

  edge1[0] = v1[0] - v0[0];
  edge1[1] = v1[1] - v0[1];
  edge1[2] = v1[2] - v0[2];
  edge2[0] = v2[0] - v0[0];
  edge2[1] = v2[1] - v0[1];
  edge2[2] = v2[2] - v0[2];

  h[0] = rayDirection[1] * edge2[2] - rayDirection[2] * edge2[1];
  h[1] = rayDirection[2] * edge2[0] - rayDirection[0] * edge2[2];
  h[2] = rayDirection[0] * edge2[1] - rayDirection[1] * edge2[0];

  a = edge1[0] * h[0] + edge1[1] * h[1] + edge1[2] * h[2];

  if (a > -EPSILON && a < EPSILON) return false;

  f = 1.0f / a;
  s[0] = rayOrigin[0] - v0[0];
  s[1] = rayOrigin[1] - v0[1];
  s[2] = rayOrigin[2] - v0[2];

  u = f * (s[0] * h[0] + s[1] * h[1] + s[2] * h[2]);
  if (u < 0.0f || u > 1.0f) return false;

  q[0] = s[1] * edge1[2] - s[2] * edge1[1];
  q[1] = s[2] * edge1[0] - s[0] * edge1[2];
  q[2] = s[0] * edge1[1] - s[1] * edge1[0];

  v = f * (rayDirection[0] * q[0] + rayDirection[1] * q[1] +
           rayDirection[2] * q[2]);
  if (v < 0.0f || u + v > 1.0f) return false;

  t = f * (edge2[0] * q[0] + edge2[1] * q[1] + edge2[2] * q[2]);

  return t > EPSILON;
}

void Application::TransformVertex(Object3D const& obj, float x, float y,
                                  float z, float& outX, float& outY,
                                  float& outZ) {
  x *= obj.scale.x;
  y *= obj.scale.y;
  z *= obj.scale.z;

  float cosX = std::cos(obj.rotation.x * std::numbers::pi_v<float> / 180.0f);
  float sinX = std::sin(obj.rotation.x * std::numbers::pi_v<float> / 180.0f);
  float newY = y * cosX - z * sinX;
  float newZ = y * sinX + z * cosX;
  y = newY;
  z = newZ;

  float cosY = std::cos(obj.rotation.y * std::numbers::pi_v<float> / 180.0f);
  float sinY = std::sin(obj.rotation.y * std::numbers::pi_v<float> / 180.0f);
  float newX = x * cosY + z * sinY;
  newZ = -x * sinY + z * cosY;
  x = newX;
  z = newZ;

  float cosZ = std::cos(obj.rotation.z * std::numbers::pi_v<float> / 180.0f);
  float sinZ = std::sin(obj.rotation.z * std::numbers::pi_v<float> / 180.0f);
  newX = x * cosZ - y * sinZ;
  newY = x * sinZ + y * cosZ;
  x = newX;
  y = newY;

  outX = x + obj.translation.x;
  outY = y + obj.translation.y;
  outZ = z + obj.translation.z;
}

int Application::PerformVertexPicking(int objectIndex) {
  glm::mat4 projMatrix = glm::mat4(1.0f);
  glm::ivec4 viewport = glm::ivec4(0, 0, 0, 0);

  glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(projMatrix));
  glGetIntegerv(GL_VIEWPORT, glm::value_ptr(viewport));

  glm::mat4 cameraMatrix = glm::mat4(1.0f);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(0.0f, 0.0f, -5.0f);
  ApplyObjectTransform(objects_[objectIndex]);
  glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(cameraMatrix));
  glPopMatrix();

  int closestVertex = -1;
  float minDistance = std::numeric_limits<float>::max();
  const float tolerance = 30.0f;

  Object3D& obj = objects_[objectIndex];

  for (int i = 0; i < static_cast<int>(obj.vertices.size()); i++) {
    Vertex vertex = obj.vertices[i];

    glm::vec2 screenPosition;
    ProjectToScreen({vertex.position, 1.0f}, cameraMatrix, projMatrix, viewport,
                    screenPosition);

    float dx = screenPosition.x - mousePos_.x;
    float dy = screenPosition.y - mousePos_.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance < tolerance && distance < minDistance) {
      minDistance = distance;
      closestVertex = i;
    }
  }

  return closestVertex;
}

int Application::PerformObjectPicking() {
  glm::mat4 projMatrix = glm::mat4(1.0f);
  glm::ivec4 viewport = glm::ivec4(0, 0, 0, 0);

  glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(projMatrix));
  glGetIntegerv(GL_VIEWPORT, glm::value_ptr(viewport));

  const float tolerance = 120.0f;

  struct PickCandidate {
    int objectIndex;
    float distance;
    float depth;
  };

  std::vector<PickCandidate> candidates;

  for (int objIdx = 0; objIdx < static_cast<int>(objects_.size()); objIdx++) {
    Object3D& object = objects_[objIdx];

    glm::mat4 objectMatrix = glm::mat4(1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.0f);
    ApplyObjectTransform(object);
    glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(objectMatrix));
    glPopMatrix();

    float minDistance = std::numeric_limits<float>::max();
    float objDepth = -std::numeric_limits<float>::max();
    bool hasHit = false;

    for (int i = 0; i < static_cast<int>(object.vertices.size()); i++) {
      Vertex vertex = object.vertices[i];

      glm::vec2 screenPosition;
      ProjectToScreen({vertex.position, 1.0f}, objectMatrix, projMatrix,
                      viewport, screenPosition);

      float dx = screenPosition.x - mousePos_.x;
      float dy = screenPosition.y - mousePos_.y;
      float distance = std::sqrt(dx * dx + dy * dy);

      if (distance < tolerance) {
        hasHit = true;
        if (distance < minDistance) {
          minDistance = distance;
          float transformedX, transformedY, transformedZ;
          TransformVertex(object, vertex.position.x, vertex.position.y,
                          vertex.position.z, transformedX, transformedY,
                          transformedZ);
          objDepth = transformedZ;
        }
      }
    }

    if (hasHit) {
      candidates.push_back({objIdx, minDistance, objDepth});
    }
  }

  if (candidates.empty()) {
    return -1;
  }

  std::sort(candidates.begin(), candidates.end(),
            [](PickCandidate const& a, PickCandidate const& b) {
              if (std::fabs(a.depth - b.depth) > 0.1f) {
                return a.depth > b.depth;
              }
              return a.distance < b.distance;
            });

  return candidates[0].objectIndex;
}

void Application::RenderObject(int objectIndex, bool highlight) {
  Object3D& obj = objects_[objectIndex];

  glPushMatrix();
  ApplyObjectTransform(obj);

  if (highlight) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(3.0f);
    glColor3f(1.0f, 1.0f, 0.0f);

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < static_cast<int>(obj.indices.size()); i++) {
      auto vertexIndex = obj.indices[i];
      for (int j : {vertexIndex.indices.x, vertexIndex.indices.y,
                    vertexIndex.indices.z}) {
        Vertex vertex = obj.vertices[j];
        glVertex3f(vertex.position.x, vertex.position.y, vertex.position.z);
      }
    }
    glEnd();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glLineWidth(1.0f);
  }

  glBegin(GL_TRIANGLES);
  for (int i = 0; i < static_cast<int>(obj.indices.size()); i++) {
    auto vertexIndex = obj.indices[i];
    for (int j : {vertexIndex.indices.x, vertexIndex.indices.y,
                  vertexIndex.indices.z}) {
      Vertex vertex = obj.vertices[j];
      glColor3f(vertex.color.x, vertex.color.y, vertex.color.z);
      glVertex3f(vertex.position.x, vertex.position.y, vertex.position.z);
    }
  }
  glEnd();

  glPopMatrix();
}

void Application::RenderSelectedVertex(int selectedVertex, int objectIndex) {
  Object3D& obj = objects_[objectIndex];

  if (selectedVertex >= 0 &&
      selectedVertex < static_cast<int>(obj.vertices.size())) {
    Vertex vertex = obj.vertices[selectedVertex];

    glPushMatrix();
    ApplyObjectTransform(obj);

    glPointSize(15.0f);
    glBegin(GL_POINTS);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(vertex.position.x, vertex.position.y, vertex.position.z);
    glEnd();

    glPushMatrix();
    glTranslatef(vertex.position.x, vertex.position.y, vertex.position.z);
    glColor4f(1.0f, 0.0f, 0.0f, 0.3f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPointSize(20.0f);
    glBegin(GL_POINTS);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glEnd();

    glDisable(GL_BLEND);
    glPopMatrix();
    glPopMatrix();
  }
}

std::string Application::GenerateFileName(bool withGUI) {
  auto now = std::time(nullptr);
  auto tm = *std::localtime(&now);
  std::ostringstream oss;
  oss << "cube_screenshot_" << (withGUI ? "withGUI_" : "")
      << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".png";
  return oss.str();
}

void Application::SaveFrameAsPNG(int width, int height,
                                 std::string const& filename, bool withGUI) {
  std::vector<unsigned char> pixels(width * height * 3);

  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

  std::vector<unsigned char> flipped(width * height * 3);
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int src_idx = ((height - 1 - y) * width + x) * 3;
      int dst_idx = (y * width + x) * 3;
      flipped[dst_idx] = pixels[src_idx];
      flipped[dst_idx + 1] = pixels[src_idx + 1];
      flipped[dst_idx + 2] = pixels[src_idx + 2];
    }
  }

  std::string outputFilename = filename.empty() ? GenerateFileName(withGUI)
                                                : filename;
  int result = stbi_write_png(outputFilename.c_str(), width, height, 3,
                              flipped.data(), width * 3);

  if (result) {
    std::printf("Screenshot saved as: %s\n", outputFilename.c_str());
  } else {
    std::printf("Failed to save screenshot!\n");
  }
}

void Application::Render() {
  for (int i = 0; i < static_cast<int>(objects_.size()); i++) {
    objects_[i].autoRotateSpeed.x *= globalRotationSpeed_;
    objects_[i].autoRotateSpeed.y *= globalRotationSpeed_;
    objects_[i].autoRotateSpeed.z *= globalRotationSpeed_;
    objects_[i].autoRotateSpeed.x /= globalRotationSpeed_;
    objects_[i].autoRotateSpeed.y /= globalRotationSpeed_;
    objects_[i].autoRotateSpeed.z /= globalRotationSpeed_;
    UpdateAutoRotation(objects_[i]);
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.0f, 0.0f, -5.0f);

  for (int i = 0; i < static_cast<int>(objects_.size()); i++) {
    bool isSelected = (i == selectedObject_);
    RenderObject(i, isSelected);
  }

  if (selectedObject_ >= 0 && selectedVertex_ >= 0) {
    RenderSelectedVertex(selectedVertex_, selectedObject_);
  }

  if (mouseClicked_) {
    if (ctrlPressed_) {
      if (selectedObject_ >= 0) {
        int pickedVertex = PerformVertexPicking(selectedObject_);
        if (pickedVertex >= 0) {
          selectedVertex_ = pickedVertex;
        } else {
          selectedVertex_ = -1;
        }
      }
    } else {
      int pickedObject = PerformObjectPicking();
      if (pickedObject >= 0) {
        selectedObject_ = pickedObject;
        selectedVertex_ = -1;
      } else {
        selectedObject_ = -1;
        selectedVertex_ = -1;
      }
    }
    mouseClicked_ = false;
  }

  if (saveScreenshotFlag_ && !includeGUI_) {
    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);
    SaveFrameAsPNG(width, height, "", false);
    saveScreenshotFlag_ = false;
  }

  RenderGui();

  if (saveScreenshotFlag_ && includeGUI_) {
    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);
    SaveFrameAsPNG(width, height, "", true);
    saveScreenshotFlag_ = false;
  }
}

void Application::RenderGui() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::Begin("3D Objects Controls");

  ImGui::Text("Object Selection:");
  char const* objectNames[] = {"None", "Cube", "Icosahedron"};
  int comboIndex = selectedObject_ + 1;
  if (ImGui::Combo("Current Object", &comboIndex, objectNames,
                   static_cast<int>(objects_.size()) + 1)) {
    selectedObject_ = comboIndex - 1;
    selectedVertex_ = -1;
  }

  ImGui::Text("Mouse Controls:");
  ImGui::BulletText("Click near object to select it (depth-sorted)");
  ImGui::BulletText("Ctrl + Click near vertex to select vertex");
  ImGui::BulletText("Click in empty space to deselect");

  if (selectedObject_ >= 0) {
    ImGui::Text("Selected Object: %s (ID: %d)", objectNames[selectedObject_ + 1],
                selectedObject_);
    if (selectedVertex_ >= 0 &&
        selectedVertex_ <
            static_cast<int>(objects_[selectedObject_].vertices.size())) {
      ImGui::Text(
          "Selected Vertex: %d - %s", selectedVertex_,
          objects_[selectedObject_].vertexNames[selectedVertex_].c_str());
    } else {
      ImGui::Text("Selected Vertex: None");
    }
  } else {
    ImGui::Text("Selected Object: None");
    ImGui::Text("Selected Vertex: None");
  }

  if (ImGui::Button("Clear Object Selection")) {
    selectedObject_ = -1;
    selectedVertex_ = -1;
  }
  if (selectedObject_ >= 0) {
    ImGui::SameLine();
    if (ImGui::Button("Clear Vertex Selection")) {
      selectedVertex_ = -1;
    }
  }

  ImGui::Separator();

  if (selectedObject_ >= 0) {
    Object3D& currentObj = objects_[selectedObject_];
    ImGui::Text("Transform Controls for %s:", objectNames[selectedObject_ + 1]);

    ImGui::Text("Translation:");
    ImGui::SliderFloat("Translate X", &currentObj.translation.x, -5.0f, 5.0f);
    ImGui::SliderFloat("Translate Y", &currentObj.translation.y, -5.0f, 5.0f);
    ImGui::SliderFloat("Translate Z", &currentObj.translation.z, -5.0f, 5.0f);

    ImGui::Text("Rotation:");
    ImGui::SliderFloat("Rotation X", &currentObj.rotation.x, 0.0f, 360.0f);
    ImGui::SliderFloat("Rotation Y", &currentObj.rotation.y, 0.0f, 360.0f);
    ImGui::SliderFloat("Rotation Z", &currentObj.rotation.z, 0.0f, 360.0f);

    ImGui::Text("Scale:");
    ImGui::SliderFloat("Scale X", &currentObj.scale.x, 0.1f, 3.0f);
    ImGui::SliderFloat("Scale Y", &currentObj.scale.y, 0.1f, 3.0f);
    ImGui::SliderFloat("Scale Z", &currentObj.scale.z, 0.1f, 3.0f);

    ImGui::Separator();

    ImGui::Text("Auto Rotation:");
    ImGui::Checkbox("Enable Auto Rotation", &currentObj.autoRotate);
    ImGui::SliderFloat("Auto Speed X", &currentObj.autoRotateSpeed.x, -2.0f,
                       2.0f);
    ImGui::SliderFloat("Auto Speed Y", &currentObj.autoRotateSpeed.y, -2.0f,
                       2.0f);
    ImGui::SliderFloat("Auto Speed Z", &currentObj.autoRotateSpeed.z, -2.0f,
                       2.0f);

    ImGui::Text("Global Speed Multiplier:");
    ImGui::SliderFloat("Global Rotation Speed", &globalRotationSpeed_, 0.0f,
                       3.0f);

    ImGui::Separator();

    if (ImGui::Button("Reset Transforms")) {
      currentObj.rotation.x = currentObj.rotation.y = currentObj.rotation.z =
          0.0f;
      currentObj.translation.x = (selectedObject_ == 0) ? -1.5f : 1.5f;
      currentObj.translation.y = currentObj.translation.z = 0.0f;
      currentObj.scale.x = currentObj.scale.y = currentObj.scale.z = 1.0f;
    }
  } else {
    ImGui::Text("No object selected");
    ImGui::Text("Select an object to control its transformations");

    ImGui::Separator();

    ImGui::Text("Global Speed Multiplier:");
    ImGui::SliderFloat("Global Rotation Speed", &globalRotationSpeed_, 0.0f,
                       3.0f);
  }

  ImGui::Separator();

  ImGui::Text("Transformations:");
  ImGui::Checkbox("Show Vertex Transformations", &showTransformations_);

  ImGui::Separator();

  ImGui::Text("Screenshot:");
  ImGui::Checkbox("Include GUI in screenshot", &includeGUI_);
  if (ImGui::Button("Save PNG Screenshot")) {
    saveScreenshotFlag_ = true;
  }
  ImGui::SameLine();
  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip(
        "Enable checkbox to include ImGui interface\nin the screenshot, or "
        "leave unchecked for\nonly the 3D scene");
  }

  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

  ImGui::End();

  if (showTransformations_) {
    ImGui::Begin("Vertex Transformations", &showTransformations_);

    if (selectedObject_ < 0) {
      ImGui::Text("No object selected");
      ImGui::Text("Select an object to see vertex transformations");
      ImGui::End();
    } else {
      Object3D& currentObj = objects_[selectedObject_];

      ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Mouse Picking:");
      ImGui::Text("Current object: %s", objectNames[selectedObject_ + 1]);
      ImGui::Text("Click to select object, Ctrl+Click to select vertex!");
      if (selectedVertex_ >= 0) {
        ImGui::Text("Selected vertex is highlighted in red.");
      } else {
        ImGui::Text("No vertex selected. Use Ctrl+Click to select a vertex.");
      }

      ImGui::Text("Debug info:");
      ImGui::Text("Mouse pos: %.1f, %.1f", mousePos_.x, mousePos_.y);
      ImGui::Text("Mouse clicked: %s", mouseClicked_ ? "YES" : "NO");
      ImGui::Text("Ctrl pressed: %s", ctrlPressed_ ? "YES" : "NO");

      int winWidth, winHeight, fbWidth, fbHeight, vpX, vpY, vpWidth, vpHeight;
      glfwGetWindowSize(glfwGetCurrentContext(), &winWidth, &winHeight);
      glfwGetFramebufferSize(glfwGetCurrentContext(), &fbWidth, &fbHeight);
      glGetIntegerv(GL_VIEWPORT, (int*)&vpX);
      vpY = ((int*)&vpX)[1];
      vpWidth = ((int*)&vpX)[2];
      vpHeight = ((int*)&vpX)[3];
      vpX = ((int*)&vpX)[0];

      ImGui::Text("Window: %dx%d, Framebuffer: %dx%d", winWidth, winHeight,
                  fbWidth, fbHeight);
      ImGui::Text("Viewport: %dx%d at (%d,%d)", vpWidth, vpHeight, vpX, vpY);

      ImGui::Separator();
      ImGui::Checkbox("Show Ray Casting Debug", &showRaycastDebug_);
      if (showRaycastDebug_) {
        ImGui::Text("Ray Casting Info:");
        ImGui::Text("Ray Origin: (%.3f, %.3f, %.3f)", lastRayOrigin_[0],
                    lastRayOrigin_[1], lastRayOrigin_[2]);
        ImGui::Text("Ray Direction: (%.3f, %.3f, %.3f)", lastRayDirection_[0],
                    lastRayDirection_[1], lastRayDirection_[2]);
        ImGui::Text("Ray casting allows clicking anywhere on object surface!");
      }

      glm::mat4 modelMatrix = glm::mat4(1.0f);
      glm::mat4 projMatrix = glm::mat4(1.0f);
      glm::ivec4 viewport = glm::ivec4(0, 0, 0, 0);
      glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(modelMatrix));
      glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(projMatrix));
      glGetIntegerv(GL_VIEWPORT, glm::value_ptr(viewport));

      ImGui::Text("All vertices screen positions for %s:",
                  objectNames[selectedObject_ + 1]);

      glm::mat4 correctMatrix = glm::mat4(1.0f);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
      glTranslatef(0.0f, 0.0f, -5.0f);
      ApplyObjectTransform(currentObj);
      glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(correctMatrix));
      glPopMatrix();

      for (int i = 0; i < static_cast<int>(currentObj.vertices.size()); i++) {
        auto vertex = currentObj.vertices[i];

        glm::vec2 screenPosition;
        ProjectToScreen({vertex.position, 1.0f}, correctMatrix, projMatrix,
                        viewport, screenPosition);

        float dx = screenPosition.x - mousePos_.x;
        float dy = screenPosition.y - mousePos_.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        ImGui::Text("V%d: screen(%.1f, %.1f) distance: %.1f %s", i,
                    screenPosition.x, screenPosition.y, distance,
                    (i == selectedVertex_) ? "SELECTED" : "");
      }

      ImGui::Text("Quick vertex selection:");
      for (int i = 0; i < static_cast<int>(currentObj.vertices.size()); i++) {
        if (i > 0 && i % 3 != 0) ImGui::SameLine();
        if (ImGui::Button(("V" + std::to_string(i)).c_str())) {
          selectedVertex_ = i;
        }
      }
      ImGui::SameLine();
      if (ImGui::Button("Clear")) {
        selectedVertex_ = -1;
      }

      ImGui::Separator();

      if (selectedVertex_ >= 0 &&
          selectedVertex_ < static_cast<int>(currentObj.vertices.size())) {
        glm::vec4 originalVertex =
            glm::vec4(currentObj.vertices[selectedVertex_].position.x,
                      currentObj.vertices[selectedVertex_].position.y,
                      currentObj.vertices[selectedVertex_].position.z, 1.0f);

        glm::mat4 combinedMatrix = glm::mat4(1.0f);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -5.0f);
        ApplyObjectTransform(currentObj);
        glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(combinedMatrix));
        glPopMatrix();

        glm::vec3 objTransformed;
        TransformVertex(currentObj, originalVertex.x, originalVertex.y,
                        originalVertex.z, objTransformed.x, objTransformed.y,
                        objTransformed.z);

        glm::vec4 finalTransformedVertex =
            combinedMatrix * glm::vec4(originalVertex[0], originalVertex[1],
                                       originalVertex[2], 1.0f);

        ImGui::Text("Selected vertex %d: %s", selectedVertex_,
                    currentObj.vertexNames[selectedVertex_].c_str());
        ImGui::Text("Original coordinates:");
        ImGui::Text("  X: %.3f", originalVertex[0]);
        ImGui::Text("  Y: %.3f", originalVertex[1]);
        ImGui::Text("  Z: %.3f", originalVertex[2]);

        ImGui::Separator();

        ImGui::Text("After object transforms:");
        ImGui::Text("  X: %.3f", objTransformed.x);
        ImGui::Text("  Y: %.3f", objTransformed.y);
        ImGui::Text("  Z: %.3f", objTransformed.z);

        ImGui::Separator();

        ImGui::Text("Final camera-transformed coordinates:");
        ImGui::Text("  X: %.3f", finalTransformedVertex[0]);
        ImGui::Text("  Y: %.3f", finalTransformedVertex[1]);
        ImGui::Text("  Z: %.3f", finalTransformedVertex[2]);

        ImGui::Separator();
      } else {
        ImGui::Text("No vertex selected");
        ImGui::Text("Use Ctrl+Click on a vertex or press a 'V' button above");
      }

      ImGui::Separator();

      ImGui::Text("Combined transformation matrix (Camera + Object):");

      float displayMatrix[16];
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
      glTranslatef(0.0f, 0.0f, -5.0f);
      ApplyObjectTransform(currentObj);
      glGetFloatv(GL_MODELVIEW_MATRIX, displayMatrix);
      glPopMatrix();

      for (int row = 0; row < 4; row++) {
        ImGui::Text("%.3f  %.3f  %.3f  %.3f", displayMatrix[row],
                    displayMatrix[row + 4], displayMatrix[row + 8],
                    displayMatrix[row + 12]);
      }

      ImGui::End();
    }
  }

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

}  // namespace cg
