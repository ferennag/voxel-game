#pragma once

#include <GL/glew.h>
#include <memory>
#include <string>
#include <unordered_map>

class Texture {
public:
  static std::shared_ptr<Texture> Load(const std::string &path);
  static void Cleanup();

  Texture(GLuint id);
  ~Texture();

  void Bind();

private:
  static std::unordered_map<std::string, std::shared_ptr<Texture>> sTextures;
  GLuint mId;
};
