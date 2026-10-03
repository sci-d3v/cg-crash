#include "common/detailed_exception.h"

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

namespace cg {

TEST(DetailedExceptionTest, FormatsMessage) {
  const DetailedException with_context("msg", "ctx", "dir/sub/file.cpp", 42,
                                       "Foo");
  EXPECT_EQ(std::string(with_context.what()),
            "[ERROR] file.cpp:42 in Foo(): msg (ctx)");

  const DetailedException empty_context("msg", "", "a\\b\\file.h", 7, "Bar");
  EXPECT_EQ(std::string(empty_context.what()),
            "[ERROR] file.h:7 in Bar(): msg");
}

TEST(DetailedExceptionTest, StripsEitherSeparator) {
  const DetailedException slash("m", "c", "only/name.cpp", 1, "F");
  EXPECT_EQ(std::string(slash.what()), "[ERROR] name.cpp:1 in F(): m (c)");

  const DetailedException backslash("m", "c", "only\\name.cpp", 1, "F");
  EXPECT_EQ(std::string(backslash.what()), "[ERROR] name.cpp:1 in F(): m (c)");

  const DetailedException mixed("m", "", "dir/sub\\file.cpp", 3, "F");
  EXPECT_EQ(std::string(mixed.what()), "[ERROR] file.cpp:3 in F(): m");
}

TEST(DetailedExceptionTest, MacroThrowsRuntimeError) {
  int line = 0;
  try {
    line = __LINE__ + 1;
    THROW_DETAILED("boom", "ctx");
    FAIL() << "expected DetailedException";
  } catch (const DetailedException& ex) {
    const std::string expected =
        "[ERROR] detailed_exception.cpp:" + std::to_string(line) +
        " in TestBody(): boom (ctx)";
    EXPECT_EQ(std::string(ex.what()), expected);
  }

  EXPECT_THROW(THROW_DETAILED("boom", "ctx"), std::runtime_error);
  EXPECT_THROW(THROW_DETAILED("boom", "ctx"), std::exception);
}

TEST(DetailedExceptionTest, EmptyContextOmitsParentheses) {
  int line = 0;
  try {
    line = __LINE__ + 1;
    THROW_DETAILED("boom", "");
    FAIL() << "expected DetailedException";
  } catch (const std::exception& ex) {
    const std::string expected =
        "[ERROR] detailed_exception.cpp:" + std::to_string(line) +
        " in TestBody(): boom";
    EXPECT_EQ(std::string(ex.what()), expected);
  }
}

}  // namespace cg
