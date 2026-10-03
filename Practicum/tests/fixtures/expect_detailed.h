#ifndef EXPECT_DETAILED_H
#define EXPECT_DETAILED_H

#include "common/detailed_exception.h"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <utility>

namespace cg {

template <typename F>
void ExpectDetailed(F&& fn, std::string_view code, std::string_view extra = {}) {
  try {
    std::forward<F>(fn)();
    FAIL() << "expected DetailedException";
  } catch (const DetailedException& ex) {
    const std::string what = ex.what();
    EXPECT_NE(what.find(code), std::string::npos);
    if (!extra.empty()) {
      EXPECT_NE(what.find(extra), std::string::npos);
    }
  }
}

}  // namespace cg

#endif  // EXPECT_DETAILED_H
