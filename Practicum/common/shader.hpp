#ifndef SHADER_HPP
#define SHADER_HPP

#include <GL/glew.h>
#include <string>

#include "detailed_exception.h"
#include "io_utils.h"

// computer graphics namespace
namespace cg {

class Shader {
 private:
  GLuint id_;
  GLuint type_;
  std::string path_;

  static std::string InfoLog(GLuint shader) {
    GLint logLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
    if (logLen <= 1) {
      return {};
    }
    std::string info(static_cast<std::size_t>(logLen - 1), '\0');
    glGetShaderInfoLog(shader, logLen, nullptr, info.data());
    return info;
  }

 public:
  ~Shader() { glDeleteShader(id_); }

  Shader(GLuint type, const std::string& path) : id_(0), type_(type), path_(path) {
    const std::string shaderCode = ReadTextFile(path_);
    const char* shaderSource = shaderCode.c_str();
    id_ = glCreateShader(type_);
    if (id_ == 0) {
      THROW_DETAILED("SHADER_COMPILATION_FAILED", "glCreateShader returned 0");
    }
    glShaderSource(id_, 1, &shaderSource, nullptr);
    glCompileShader(id_);
    GLint success = GL_FALSE;
    glGetShaderiv(id_, GL_COMPILE_STATUS, &success);
    if (!success) {
      const std::string infoLog = InfoLog(id_);
      glDeleteShader(id_);
      id_ = 0;
      THROW_DETAILED("SHADER_COMPILATION_FAILED", infoLog);
    }
  }

  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;
  Shader& operator=(Shader&&) = delete;

  Shader(Shader&& other) noexcept
      : id_(other.id_), type_(other.type_), path_(std::move(other.path_)) {
    other.id_ = 0;
  }

  GLuint GetId() const { return id_; }
  GLuint GetType() const { return type_; }
  const std::string& GetPath() const { return path_; }
};

}  // namespace cg

#endif  // SHADER_HPP
