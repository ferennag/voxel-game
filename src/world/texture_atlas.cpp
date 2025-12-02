#include "texture_atlas.h"
#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <glm/glm.hpp>

TextureAtlasBuilder::TextureAtlasBuilder(const int tileSize) : mTileSize(tileSize) {
}

void TextureAtlasBuilder::AddTexture(TextureType type, const std::string &texture) {
  mEntries.push_back({.type = type, .texturePath = texture});
}

std::unique_ptr<TextureAtlas> TextureAtlasBuilder::Build() {
  if (mEntries.size() == 0) {
    return nullptr;
  }

  const float size = 4096.0f;

  for (auto &entry : mEntries) {
    SDL_Surface *surface = IMG_Load(entry.texturePath.c_str());
    entry.image = surface;
  }

  GLuint texture;
  glCreateTextures(GL_TEXTURE_2D, 1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

  std::vector<TextureAtlasEntry> entries;
  int offsetX = 0;
  int offsetY = 0;

  for (auto &entry : mEntries) {
    glTextureSubImage2D(texture, 0, offsetX, offsetY, mTileSize, mTileSize, GL_RGBA, GL_UNSIGNED_BYTE,
                        entry.image->pixels);

    entries.push_back({
        .type = entry.type,
        .start = {offsetX / size, offsetY / size},
        .end = {(offsetX + mTileSize) / size, (offsetY + mTileSize) / size},
    });

    offsetX += mTileSize;
    if (offsetX >= size) {
      offsetX = 0;
      offsetY += mTileSize;
    }

    SDL_DestroySurface(entry.image);
  }

  glGenerateMipmap(GL_TEXTURE_2D);

  return std::make_unique<TextureAtlas>(texture, entries);
}

void TextureAtlas::Bind() const {
  glBindTexture(GL_TEXTURE_2D, mId);
}

TextureAtlas::TextureAtlas(const GLuint id, const std::vector<TextureAtlasEntry> &entries)
    : mId(id), mEntries(entries) {
}

TextureAtlas::~TextureAtlas() {
  glDeleteTextures(1, &mId);
}

std::optional<TextureAtlasEntry> TextureAtlas::GetTexture(const TextureType type) const {
  for (auto &entry : mEntries) {
    if (entry.type == type) {
      return entry;
    }
  }

  return std::nullopt;
}

glm::vec2 TextureAtlasEntry::BottomLeft() const {
  return start;
}

glm::vec2 TextureAtlasEntry::BottomRight() const {
  return {end.x, start.y};
}

glm::vec2 TextureAtlasEntry::TopLeft() const {
  return {start.x, end.y};
}

glm::vec2 TextureAtlasEntry::TopRight() const {
  return end;
}
