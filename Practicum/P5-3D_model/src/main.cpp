#include <iostream>

#include "application.hpp"
#include "common/detailed_exception.h"
#include "scene.hpp"

int main(int argc, char** argv) try {
  const char* name = argc > 1 ? argv[1] : "house";
  const auto cfg = std::filesystem::path("assets") / name / "scene.json";
  cg::Application application("CG_P5", 800, 600, cg::LoadScene(cfg));
  application.RenderLoop();
  return 0;
} catch (const cg::DetailedException& e) {
  std::cout << "Detailed error: " << e.what() << std::endl;
  return 1;
} catch (const std::exception& e) {
  std::cout << "Error: " << e.what() << std::endl;
  return 2;
} catch (...) {
  std::cout << "Unknown compilation error" << std::endl;
  return 3;
}
