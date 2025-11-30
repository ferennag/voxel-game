#include "shader.h"
#include "SDL3/SDL_log.h"

#include <SDL3/SDL.h>
#include <fstream>
#include <memory>
#include <optional>
#include <sstream>

enum ShaderType {
  VERTEX,
  FRAGMENT
};

static std::optional<std::string> loadFile(const std::string &path) {
  std::ifstream in(path);
  if (!in) {
    SDL_Log("Failed to load file: %s", path.c_str());
    return std::nullopt;
  }

  std::string contents(static_cast<std::stringstream const &>(std::stringstream() << in.rdbuf()).str());
  return contents;
}

static std::optional<GLuint> compileShader(const std::string &path, ShaderType type) {
  const auto source = loadFile(path);
  if (!source) {
    return std::nullopt;
  }

  GLint shader = -1;
  switch (type) {
    case VERTEX:
      shader = glCreateShader(GL_VERTEX_SHADER);
      break;
    case FRAGMENT:
      shader = glCreateShader(GL_FRAGMENT_SHADER);
      break;
  }

  if (shader < 0) {
    return std::nullopt;
  }

  const char *cSource = source->c_str();
  glShaderSource(shader, 1, &cSource, nullptr);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    int length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    std::string str;
    str.reserve(length);
    glGetShaderInfoLog(shader, length, nullptr, str.data());
    SDL_Log("Failed to compile shader (%s) : %s", path.c_str(), str.c_str());
    return std::nullopt;
  }

  return shader;
}

std::unique_ptr<Shader> Shader::Load(const std::string &vertexPath, const std::string &fragmentPath) {
  auto vertex = compileShader(vertexPath, ShaderType::VERTEX);
  auto fragment = compileShader(fragmentPath, ShaderType::FRAGMENT);

  if (!vertex || !fragment) {
    if (vertex) {
      glDeleteShader(*vertex);
    }

    if (fragment) {
      glDeleteShader(*fragment);
    }

    return nullptr;
  }

  GLuint program = glCreateProgram();
  glAttachShader(program, *vertex);
  glAttachShader(program, *fragment);
  glLinkProgram(program);

  glDeleteShader(*vertex);
  glDeleteShader(*fragment);

  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    int length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    std::string str;
    str.reserve(length);
    glGetProgramInfoLog(program, length, nullptr, str.data());
    SDL_Log("Failed to link shader program (%s, %s) : %s", vertexPath.c_str(), fragmentPath.c_str(), str.c_str());
    return nullptr;
  }

  auto shader = std::make_unique<Shader>(program);
  return shader;
}

Shader::Shader(GLuint id) : mId(id) {
}

Shader::~Shader() {
  glDeleteProgram(mId);
}

void Shader::Bind() const {
  glUseProgram(mId);
}

void Shader::Unbind() const {
  glUseProgram(0);
}

void Shader::UniformMat4(const std::string &name, const glm::mat4 &value) const {
  auto location = glGetUniformLocation(mId, name.c_str());
  if (location < 0) {
    SDL_Log("Could not find uniform location %s", name.c_str());
    return;
  }

  glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
}
