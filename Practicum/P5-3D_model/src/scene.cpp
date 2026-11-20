#include "scene.hpp"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

// Computer Graphics Namespace (cg)
namespace cg {

Scene LoadScene(const std::filesystem::path& cfg) {
  std::ifstream in(cfg);
  if (!in)
    throw std::runtime_error("cannot open " + cfg.string());

  const auto root = nlohmann::json::parse(in);
  const auto dir = cfg.parent_path();
  const auto model = dir / (dir.filename().string() + ".gltf");

  const auto& cam = root.at("camera");
  const auto& xform = root.at("transform");
  const auto& light = root.at("light");
  const auto& mat = root.at("material");

  auto get_vec3 = [&](const nlohmann::json& element) -> glm::vec3 {
    return glm::vec3(element.at(0).get<float>(), element.at(1).get<float>(),
                     element.at(2).get<float>());
  };
  return Scene{
      .modelPath = model.generic_string(),
      .camera = {.eye = get_vec3(cam.at("eye")),
                 .target = get_vec3(cam.at("target")),
                 .up = get_vec3(cam.at("up")),
                 .fovDeg = cam.at("fovDeg").get<float>(),
                 .zNear = cam.at("zNear").get<float>(),
                 .zFar = cam.at("zFar").get<float>()},
      .transform = {.scale = get_vec3(xform.at("scale")),
                    .axis = get_vec3(xform.at("axis")),
                    .angularSpeed = xform.at("angularSpeed").get<float>()},
      .light = {.intensity = get_vec3(light.at("intensity")),
                .orbitRadius = light.at("orbitRadius").get<float>(),
                .orbitHeight = light.at("orbitHeight").get<float>()},
      .material = {.ka = get_vec3(mat.at("ka")),
                   .ks = get_vec3(mat.at("ks")),
                   .shininess = mat.at("shininess").get<float>()},
  };
}

}  // namespace cg
