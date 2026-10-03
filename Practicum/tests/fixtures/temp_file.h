#ifndef TEMP_FILE_H
#define TEMP_FILE_H

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace cg {

class TempFile {
 public:
  TempFile(std::string_view content, std::string_view suffix = ".txt") {
    path_ = MakePath(suffix);
    Write(content);
  }

  TempFile(const TempFile&) = delete;
  TempFile& operator=(const TempFile&) = delete;

  ~TempFile() {
    std::error_code ec;
    std::filesystem::remove(path_, ec);
  }

  const std::filesystem::path& path() const { return path_; }

  void Write(std::string_view content) {
    std::ofstream file(path_, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
      throw std::runtime_error("Cannot create test file: " + path_.string());
    }
    file.write(content.data(), static_cast<std::streamsize>(content.size()));
    if (!file) {
      std::error_code ec;
      std::filesystem::remove(path_, ec);
      throw std::runtime_error("Cannot write test file: " + path_.string());
    }
  }

 private:
  static std::filesystem::path MakePath(std::string_view suffix) {
    static std::atomic<std::uint64_t> seq{0};
    const auto stamp =
        std::chrono::steady_clock::now().time_since_epoch().count();
    const auto id = seq.fetch_add(1, std::memory_order_relaxed);
    return std::filesystem::temp_directory_path() /
           ("cg_test_" + std::to_string(stamp) + "_" + std::to_string(id) +
            std::string(suffix));
  }

  std::filesystem::path path_;
};

}  // namespace cg

#endif  // TEMP_FILE_H
