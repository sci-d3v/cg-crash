#ifndef SHADER_PROGRAM_HPP
#define SHADER_PROGRAM_HPP

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "detailed_exception.h"
#include "shader.hpp"

// Computer Graphics Namespace (cg)
namespace cg {

template <typename... ShaderTypes>
class ShaderProgram {
 private:
  GLuint id_;

  static std::string InfoLog(GLuint program) {
    GLint logLen = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
    if (logLen <= 1) {
      return {};
    }
    std::string info(static_cast<std::size_t>(logLen - 1), '\0');
    glGetProgramInfoLog(program, logLen, nullptr, info.data());
    return info;
  }

 public:
  GLuint GetId() const { return id_; }

  ~ShaderProgram() { glDeleteProgram(id_); }

  template <typename... Args>
  ShaderProgram(Args&&... args) : id_(0) {
    id_ = glCreateProgram();
    if (id_ == 0) {
      THROW_DETAILED("SHADER_PROGRAM_LINK_FAILED", "glCreateProgram returned 0");
    }
    ((glAttachShader(id_, args.GetId()), ...));
    glLinkProgram(id_);

    GLint success = GL_FALSE;
    glGetProgramiv(id_, GL_LINK_STATUS, &success);
    if (!success) {
      const std::string infoLog = InfoLog(id_);
      glDeleteProgram(id_);
      id_ = 0;
      THROW_DETAILED("SHADER_PROGRAM_LINK_FAILED", infoLog);
    }
  }

  ShaderProgram(const ShaderProgram&) = delete;
  ShaderProgram& operator=(const ShaderProgram&) = delete;
  ShaderProgram& operator=(ShaderProgram&&) = delete;

  ShaderProgram(ShaderProgram&& other) noexcept : id_(other.id_) { other.id_ = 0; }

  // activate the shader program
  void Use() const { glUseProgram(id_); }

  // Instead of (as example)
  // GLint ulMatModel = glGetUniformLocation(shaderProgram, "matModel");
  // glUniformMatrix4fv(ulMatModel, 1, GL_FALSE, &matModel[0][0]);
  // Use (as example)
  // SetValue("matModel", glUniformMatrix4fv, 1, GL_FALSE, &matModel[0][0]);
  // utility function for setting uniform values
  template <typename F, typename... Types>
  void SetValue(std::string const& name, F f, Types&&... values) const {
    f(glGetUniformLocation(id_, name.c_str()), std::forward<Types>(values)...);
  }
};

// The pack guide also matches ShaderProgram&& and would deduce a nested
// program type. This guide keeps CTAD on the move constructor.
template <typename... ShaderTypes>
ShaderProgram(ShaderProgram<ShaderTypes...>&&) -> ShaderProgram<ShaderTypes...>;

// CTAD for ShaderProgram
template <typename... Args>
ShaderProgram(Args&&...) -> ShaderProgram<std::decay_t<Args>...>;

// Factory function to avoid explicitly writing ShaderProgram<...>
template <typename... Args>
auto make_shader_program(Args&&... args) {
  using SP = ShaderProgram<std::decay_t<Args>...>;
  return std::make_unique<SP>(std::forward<Args>(args)...);
}

}  // namespace cg

#endif  // SHADER_PROGRAM_HPP
