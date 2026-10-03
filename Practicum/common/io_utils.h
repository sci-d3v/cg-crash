#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <filesystem>
#include <fstream>
#include <string>

#include "detailed_exception.h"

// Computer Graphics Namespace (cg)
namespace cg {

inline std::string ReadTextFile(const std::filesystem::path& path) {
  std::ifstream file(path, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    THROW_DETAILED("FILE_NOT_SUCCESSFULLY_OPENED", path.string());
  }

  const auto end = file.tellg();
  if (end < 0) {
    THROW_DETAILED("FILE_NOT_SUCCESSFULLY_READ", path.string());
  }

  const auto size = static_cast<std::size_t>(end);
  std::string content;
  content.resize(size);

  file.seekg(0);
  file.read(content.data(), static_cast<std::streamsize>(size));
  if (!file) {
    THROW_DETAILED("FILE_NOT_SUCCESSFULLY_READ", path.string());
  }

  return content;
}

}  // namespace cg

#endif  // IO_UTILS_H
