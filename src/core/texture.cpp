#include "texture.h"
#include <SDL3_image/SDL_image.h>
#include <memory>

std::unordered_map<std::string, std::shared_ptr<Texture>> Texture::sTextures{};

std::shared_ptr<Texture> Texture::Load(const std::string &path) {
  GLuint texture;
  SDL_Surface *image = IMG_Load("assets/textures/dirt.png");
  if (!image) {
    SDL_Log("Failed to load texture: %s", path.c_str());
    return nullptr;
  }

  if (sTextures.contains(path)) {
    return sTextures[path];
  }

  glCreateTextures(GL_TEXTURE_2D, 1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
  glGenerateMipmap(GL_TEXTURE_2D);
  SDL_DestroySurface(image);

  auto result = std::make_shared<Texture>(texture);
  sTextures[path] = result;
  return result;
}

void Texture::Cleanup() {
  sTextures.clear();
}

Texture::Texture(GLuint id) : mId(id) {
}

Texture::~Texture() {
  glDeleteTextures(1, &mId);
}

void Texture::Bind() {
  glBindTexture(GL_TEXTURE_2D, mId);
}
