#pragma once

#include "core/shader.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>

class Sky {
  GLuint mVao, mVbo;
  GLuint mTexture;

  std::unique_ptr<Shader> mShader;

public:
  Sky();
  ~Sky();

  void Render(const glm::mat4 &projection, const glm::mat4 &view) const;
};
