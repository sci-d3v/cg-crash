#include <iostream>
#include "common/detailed_exception.h"
#include "application.hpp"

int main(int argc, char* argv[]) try {
  cg::Application application("CG_Geometric_Primitives", 800, 800);
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
