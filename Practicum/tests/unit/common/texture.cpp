#include "expect_detailed.h"
#include "gl_context.h"
#include "temp_file.h"

#include "common/texture.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <type_traits>

namespace cg {

static_assert(!std::is_copy_constructible_v<Texture>);
static_assert(!std::is_copy_assignable_v<Texture>);
static_assert(std::is_move_constructible_v<Texture>);
static_assert(!std::is_move_assignable_v<Texture>);

namespace {

class TextureTest : public GLContext {};

std::string Make1x1Bmp() {
  unsigned char bmp[58] = {};
  bmp[0] = 'B';
  bmp[1] = 'M';
  bmp[2] = 58;
  bmp[10] = 54;
  bmp[14] = 40;
  bmp[18] = 1;
  bmp[22] = 1;
  bmp[26] = 1;
  bmp[28] = 24;
  bmp[34] = 4;
  bmp[54] = 10;
  bmp[55] = 20;
  bmp[56] = 30;
  return std::string(reinterpret_cast<const char*>(bmp), sizeof(bmp));
}

}  // namespace

TEST(TextureLoad, EmptyPathThrows) {
  ExpectDetailed([&] { Texture texture("", "diffuse"); }, "TEXTURE_PATH_ERROR");
}

TEST(TextureLoad, MissingFileThrows) {
  const auto path =
      std::filesystem::temp_directory_path() / "cg_test_missing_texture.bmp";
  std::filesystem::remove(path);

  ExpectDetailed([&] { Texture texture(path.string(), "diffuse"); },
                 "TEXTURE_LOAD_ERROR", path.string());
}

TEST_F(TextureTest, Loads1x1Bmp) {
  const TempFile image(Make1x1Bmp(), ".bmp");
  Texture texture(image.path().string(), "diffuse");

  EXPECT_NE(texture.GetId(), 0u);
  EXPECT_TRUE(glIsTexture(texture.GetId()));
  EXPECT_EQ(texture.GetType(), "diffuse");
  EXPECT_EQ(texture.GetPath(), image.path().string());
  EXPECT_EQ(texture.GetChannelCount(), 3);
  EXPECT_EQ(texture.GetInternalFormat(), static_cast<GLenum>(GL_RGB8));
  EXPECT_EQ(texture.GetSourceFormat(), static_cast<GLenum>(GL_RGB));
  EXPECT_EQ(texture.GetSize().x, 1);
  EXPECT_EQ(texture.GetSize().y, 1);
}

TEST_F(TextureTest, DestructorDeletesTexture) {
  const TempFile image(Make1x1Bmp(), ".bmp");
  GLuint id = 0;
  {
    Texture texture(image.path().string(), "diffuse");
    id = texture.GetId();
    EXPECT_TRUE(glIsTexture(id));
  }
  EXPECT_FALSE(glIsTexture(id));
}

TEST_F(TextureTest, MoveTransfersOwnership) {
  const TempFile image(Make1x1Bmp(), ".bmp");
  Texture src(image.path().string(), "diffuse");
  const GLuint id = src.GetId();
  ASSERT_TRUE(glIsTexture(id));

  {
    Texture dst(std::move(src));
    EXPECT_EQ(src.GetId(), 0u);
    EXPECT_EQ(dst.GetId(), id);
    EXPECT_TRUE(glIsTexture(id));
  }

  EXPECT_FALSE(glIsTexture(id));
  EXPECT_EQ(src.GetId(), 0u);
}

}  // namespace cg
