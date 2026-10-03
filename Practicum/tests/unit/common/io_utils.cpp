#include "common/io_utils.h"

#include "expect_detailed.h"
#include "temp_file.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

namespace cg {

TEST(ReadTextFile, MissingFileThrows) {
  const auto path =
      std::filesystem::temp_directory_path() / "cg_test_missing_file.txt";
  std::filesystem::remove(path);

  ExpectDetailed([&] { ReadTextFile(path); }, "FILE_NOT_SUCCESSFULLY_OPENED",
                 path.string());
}

TEST(ReadTextFile, EmptyFile) {
  const TempFile file("", ".txt");
  EXPECT_EQ(ReadTextFile(file.path()), "");
}

TEST(ReadTextFile, PreservesBinaryBytes) {
  const std::string payload("a\nb\r\nc\0d\xff", 9);
  const TempFile file(payload, ".bin");
  EXPECT_EQ(ReadTextFile(file.path()), payload);
}

}  // namespace cg
