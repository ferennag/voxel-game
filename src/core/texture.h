#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>

class Texture {
public:
  static std::shared_ptr<Texture> Load(const std::string &path);
  static void Cleanup();

  Texture(GLuint id, const glm::ivec2 &dimensions);
  ~Texture();

  void Bind();

private:
  static std::unordered_map<std::string, std::shared_ptr<Texture>> sTextures;
  GLuint mId;
  glm::ivec2 mDimensions;
};
