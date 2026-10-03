#include "expect_detailed.h"
#include "gl_context.h"
#include "temp_file.h"

#include "common/shader.hpp"
#include "common/shader_program.hpp"

#include <gtest/gtest.h>

#include <string>
#include <type_traits>

namespace cg {

static_assert(!std::is_copy_constructible_v<ShaderProgram<Shader, Shader>>);
static_assert(!std::is_copy_assignable_v<ShaderProgram<Shader, Shader>>);
static_assert(std::is_move_constructible_v<ShaderProgram<Shader, Shader>>);
static_assert(!std::is_move_assignable_v<ShaderProgram<Shader, Shader>>);

namespace {

constexpr const char* kVertexShader = R"(#version 330 core
layout (location = 0) in vec3 aPos;
void main() {
  gl_Position = vec4(aPos, 1.0);
}
)";

constexpr const char* kFragmentShader = R"(#version 330 core
uniform float uScale;
out vec4 FragColor;
void main() {
  FragColor = vec4(uScale, 0.0, 0.0, 1.0);
}
)";

class ShaderProgramTest : public GLContext {};

}  // namespace

TEST_F(ShaderProgramTest, LinksUsesAndForwardsUniforms) {
  const TempFile vertex_file(kVertexShader, ".vert");
  const TempFile fragment_file(kFragmentShader, ".frag");
  Shader vertex(GL_VERTEX_SHADER, vertex_file.path().string());
  Shader fragment(GL_FRAGMENT_SHADER, fragment_file.path().string());

  ShaderProgram program(vertex, fragment);
  static_assert(
      std::is_same_v<decltype(program), ShaderProgram<Shader, Shader>>);

  EXPECT_NE(program.GetId(), 0u);
  EXPECT_TRUE(glIsProgram(program.GetId()));

  GLint link_status = GL_FALSE;
  glGetProgramiv(program.GetId(), GL_LINK_STATUS, &link_status);
  EXPECT_EQ(link_status, GL_TRUE);

  program.Use();
  GLint current = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &current);
  EXPECT_EQ(static_cast<GLuint>(current), program.GetId());

  GLint location = -2;
  float received = 0.0f;
  program.SetValue(
      "uScale",
      [&](GLint uniform_location, float value) {
        location = uniform_location;
        received = value;
      },
      0.5f);
  EXPECT_GE(location, 0);
  EXPECT_FLOAT_EQ(received, 0.5f);
}

TEST_F(ShaderProgramTest, UnknownUniformStillInvokesCallable) {
  const TempFile vertex_file(kVertexShader, ".vert");
  const TempFile fragment_file(kFragmentShader, ".frag");
  Shader vertex(GL_VERTEX_SHADER, vertex_file.path().string());
  Shader fragment(GL_FRAGMENT_SHADER, fragment_file.path().string());
  ShaderProgram program(vertex, fragment);

  GLint location = 0;
  float received = -1.0f;
  program.SetValue(
      "uMissing",
      [&](GLint uniform_location, float value) {
        location = uniform_location;
        received = value;
      },
      0.25f);
  EXPECT_EQ(location, -1);
  EXPECT_FLOAT_EQ(received, 0.25f);
}

TEST_F(ShaderProgramTest, TwoVertexShadersFailLink) {
  const TempFile first_file(kVertexShader, ".vert");
  const TempFile second_file(kVertexShader, ".vert");
  Shader first(GL_VERTEX_SHADER, first_file.path().string());
  Shader second(GL_VERTEX_SHADER, second_file.path().string());

  ExpectDetailed([&] { ShaderProgram program(first, second); },
                 "SHADER_PROGRAM_LINK_FAILED");
}

TEST_F(ShaderProgramTest, FactoryReturnsLiveProgram) {
  const TempFile vertex_file(kVertexShader, ".vert");
  const TempFile fragment_file(kFragmentShader, ".frag");
  Shader vertex(GL_VERTEX_SHADER, vertex_file.path().string());
  Shader fragment(GL_FRAGMENT_SHADER, fragment_file.path().string());

  const auto program = make_shader_program(vertex, fragment);
  ASSERT_NE(program, nullptr);
  EXPECT_NE(program->GetId(), 0u);
  EXPECT_TRUE(glIsProgram(program->GetId()));
}

TEST_F(ShaderProgramTest, DestructorDeletesProgram) {
  const TempFile vertex_file(kVertexShader, ".vert");
  const TempFile fragment_file(kFragmentShader, ".frag");
  Shader vertex(GL_VERTEX_SHADER, vertex_file.path().string());
  Shader fragment(GL_FRAGMENT_SHADER, fragment_file.path().string());

  GLuint id = 0;
  {
    ShaderProgram program(vertex, fragment);
    id = program.GetId();
    EXPECT_TRUE(glIsProgram(id));
  }
  EXPECT_FALSE(glIsProgram(id));
}

TEST_F(ShaderProgramTest, MoveTransfersOwnership) {
  const TempFile vertex_file(kVertexShader, ".vert");
  const TempFile fragment_file(kFragmentShader, ".frag");
  Shader vertex(GL_VERTEX_SHADER, vertex_file.path().string());
  Shader fragment(GL_FRAGMENT_SHADER, fragment_file.path().string());

  ShaderProgram src(vertex, fragment);
  const GLuint id = src.GetId();
  ASSERT_TRUE(glIsProgram(id));

  {
    ShaderProgram dst(std::move(src));
    EXPECT_EQ(src.GetId(), 0u);
    EXPECT_EQ(dst.GetId(), id);
    EXPECT_TRUE(glIsProgram(id));
  }

  EXPECT_FALSE(glIsProgram(id));
  EXPECT_EQ(src.GetId(), 0u);
}

}  // namespace cg
