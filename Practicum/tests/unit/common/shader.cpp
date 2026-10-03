#include "expect_detailed.h"
#include "gl_context.h"
#include "temp_file.h"

#include "common/shader.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <type_traits>

namespace cg {

static_assert(!std::is_copy_constructible_v<Shader>);
static_assert(!std::is_copy_assignable_v<Shader>);
static_assert(std::is_move_constructible_v<Shader>);
static_assert(!std::is_move_assignable_v<Shader>);

namespace {

constexpr const char* kVertexShader = R"(#version 330 core
layout (location = 0) in vec3 aPos;
void main() {
  gl_Position = vec4(aPos, 1.0);
}
)";

constexpr const char* kFragmentShader = R"(#version 330 core
out vec4 FragColor;
void main() {
  FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)";

class ShaderTest : public GLContext {};

}  // namespace

TEST_F(ShaderTest, ValidVertexShader) {
  const TempFile file(kVertexShader, ".glsl");
  Shader shader(GL_VERTEX_SHADER, file.path().string());

  EXPECT_NE(shader.GetId(), 0u);
  EXPECT_TRUE(glIsShader(shader.GetId()));
  EXPECT_EQ(shader.GetType(), static_cast<GLuint>(GL_VERTEX_SHADER));
  EXPECT_EQ(shader.GetPath(), file.path().string());
}

TEST_F(ShaderTest, ValidFragmentShader) {
  const TempFile file(kFragmentShader, ".glsl");
  const Shader shader(GL_FRAGMENT_SHADER, file.path().string());

  EXPECT_NE(shader.GetId(), 0u);
  EXPECT_TRUE(glIsShader(shader.GetId()));
  EXPECT_EQ(shader.GetType(), static_cast<GLuint>(GL_FRAGMENT_SHADER));
}

TEST_F(ShaderTest, DestructorDeletesShader) {
  const TempFile file(kVertexShader, ".glsl");
  GLuint id = 0;
  {
    Shader shader(GL_VERTEX_SHADER, file.path().string());
    id = shader.GetId();
    EXPECT_TRUE(glIsShader(id));
  }
  EXPECT_FALSE(glIsShader(id));
}

TEST_F(ShaderTest, MoveTransfersOwnership) {
  const TempFile file(kVertexShader, ".glsl");
  Shader src(GL_VERTEX_SHADER, file.path().string());
  const GLuint id = src.GetId();
  ASSERT_TRUE(glIsShader(id));

  {
    Shader dst(std::move(src));
    EXPECT_EQ(src.GetId(), 0u);
    EXPECT_EQ(dst.GetId(), id);
    EXPECT_TRUE(glIsShader(id));
  }

  EXPECT_FALSE(glIsShader(id));
  EXPECT_EQ(src.GetId(), 0u);
}

TEST_F(ShaderTest, InvalidShaderThrows) {
  const TempFile file("not a shader", ".glsl");
  ExpectDetailed([&] { Shader shader(GL_VERTEX_SHADER, file.path().string()); },
                 "SHADER_COMPILATION_FAILED");
}

TEST_F(ShaderTest, MissingFileThrows) {
  const auto path =
      std::filesystem::temp_directory_path() / "cg_test_missing_shader.glsl";
  std::filesystem::remove(path);

  ExpectDetailed([&] { Shader shader(GL_VERTEX_SHADER, path.string()); },
                 "FILE_NOT_SUCCESSFULLY_OPENED", path.string());
}

}  // namespace cg
