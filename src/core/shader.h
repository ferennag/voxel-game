#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include <GL/glew.h>
#include <memory>
#include <string>

class Shader {
public:
  static std::unique_ptr<Shader> Load(const std::string &vertexPath, const std::string &fragmentPath);

  Shader(GLuint id);
  ~Shader();

  void Bind() const;
  void Unbind() const;

  void UniformVec3(const std::string &name, const glm::vec3 &value) const;
  void UniformMat4(const std::string &name, const glm::mat4 &value) const;

private:
  GLuint mId;
};
