#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <GL/glew.h>
#include <SOIL/SOIL.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

#include "detailed_exception.h"

// Computer Graphics Namespace (cg)
namespace cg {

class Texture {
 private:
  GLuint id_;
  std::string type_;
  std::string path_;
  glm::ivec2 size_;
  int channelCount_;
  GLenum internalFormat_;
  GLenum sourceFormat_;

 public:
  GLuint GetId() const { return id_; }
  const std::string& GetType() const { return type_; }
  const std::string& GetPath() const { return path_; }
  const glm::ivec2& GetSize() const { return size_; }
  int GetChannelCount() const { return channelCount_; }
  GLenum GetInternalFormat() const { return internalFormat_; }
  GLenum GetSourceFormat() const { return sourceFormat_; }

  ~Texture() {
    if (id_ != 0) {
      glDeleteTextures(1, &id_);
    }
  }

  Texture(std::string const& path, std::string const& type)
      : id_(0),
        type_(type),
        path_(path),
        size_(0),
        channelCount_(0),
        internalFormat_(0),
        sourceFormat_(0) {
    if (path_.empty()) {
      THROW_DETAILED("TEXTURE_PATH_ERROR", "Invalid texture path");
    }

    unsigned char* dataImage = SOIL_load_image(
        path_.c_str(), &size_.x, &size_.y, &channelCount_, SOIL_LOAD_AUTO);

    // SOIL owns the buffer until SOIL_free_image_data.
    struct ImageDeleter {
      void operator()(unsigned char* ptr) const {
        if (ptr) {
          SOIL_free_image_data(ptr);
        }
      }
    };
    std::unique_ptr<unsigned char, ImageDeleter> imageGuard(dataImage);
    if (dataImage == nullptr) {
      THROW_DETAILED("TEXTURE_LOAD_ERROR",
                     "SOIL error: " + std::string(SOIL_last_result()) +
                         " for path: " + path_);
    }

    if (size_.x <= 0 || size_.y <= 0) {
      THROW_DETAILED("TEXTURE_SIZE_ERROR",
                     "Invalid dimensions: " + std::to_string(size_.x) + "x" +
                         std::to_string(size_.y));
    }

    switch (channelCount_) {
      case 1:
        internalFormat_ = GL_R8;
        sourceFormat_ = GL_RED;
        break;
      case 2:
        internalFormat_ = GL_RG8;
        sourceFormat_ = GL_RG;
        break;
      case 3:
        internalFormat_ = GL_RGB8;
        sourceFormat_ = GL_RGB;
        break;
      case 4:
        internalFormat_ = GL_RGBA8;
        sourceFormat_ = GL_RGBA;
        break;
      default:
        THROW_DETAILED(
            "TEXTURE_FORMAT_ERROR",
            "Unsupported channel count: " + std::to_string(channelCount_) +
                " for path: " + path_);
    }

    glGenTextures(1, &id_);
    if (id_ == 0) {
      THROW_DETAILED("OPENGL_ERROR", "glGenTextures returned 0");
    }

    glBindTexture(GL_TEXTURE_2D, id_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGetError();
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat_, size_.x, size_.y, 0,
                 sourceFormat_, GL_UNSIGNED_BYTE, dataImage);
    const GLenum uploadError = glGetError();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (uploadError != GL_NO_ERROR) {
      glDeleteTextures(1, &id_);
      id_ = 0;
      THROW_DETAILED("OPENGL_ERROR",
                     "glTexImage2D failed: " + std::to_string(uploadError));
    }
  }

  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;
  Texture& operator=(Texture&&) = delete;

  // Zero id_ so the moved-from destructor does not delete the GL name.
  Texture(Texture&& other) noexcept
      : id_(other.id_),
        type_(std::move(other.type_)),
        path_(std::move(other.path_)),
        size_(other.size_),
        channelCount_(other.channelCount_),
        internalFormat_(other.internalFormat_),
        sourceFormat_(other.sourceFormat_) {
    other.id_ = 0;
  }
};

}  // namespace cg

#endif  // TEXTURE_HPP
