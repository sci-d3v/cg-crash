#ifndef SCENE_HPP
#define SCENE_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <filesystem>
#include <string>

// Computer Graphics Namespace (cg)
namespace cg {

struct Camera {
  glm::vec3 eye;
  glm::vec3 target;
  glm::vec3 up;
  float fovDeg;
  float zNear;
  float zFar;

  glm::mat4 ViewMatrix() const { return glm::lookAt(eye, target, up); }

  glm::mat4 ProjectionMatrix(float aspect) const {
    return glm::perspective(glm::radians(fovDeg), aspect, zNear, zFar);
  }
};

struct ModelTransform {
  glm::vec3 scale;
  glm::vec3 axis;
  float angularSpeed;

  float Angle(float time) const { return angularSpeed * time; }

  glm::mat4 Matrix(float time) const {
    glm::mat4 m(1.f);
    m = glm::scale(m, scale);
    return glm::rotate(m, Angle(time), axis);
  }
};

struct Light {
  glm::vec3 intensity;
  float orbitRadius;
  float orbitHeight;

  glm::vec4 WorldPosition(float angle) const {
    return glm::vec4(orbitRadius * std::cos(angle), orbitHeight,
                     orbitRadius * std::sin(angle), 1.f);
  }
};

// Diffuse color comes from the model texture, not from these coefficients.
struct Material {
  glm::vec3 ka;
  glm::vec3 ks;
  float shininess;
};

// Frame values for one model. Not a scene graph.
struct Scene {
  std::string modelPath;
  Camera camera;
  ModelTransform transform;
  Light light;
  Material material;
};

Scene LoadScene(const std::filesystem::path& cfg);

}  // namespace cg

#endif  // SCENE_HPP
