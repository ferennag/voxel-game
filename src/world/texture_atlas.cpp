#include "texture_atlas.h"
#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <cassert>
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

  // TODO: This is an optimization, if the atlas only has 1 texture we just create a texture of that size
  // Note: This is ugly and should be refactored in some way
  if (mEntries.size() == 1) {
    auto entry = *mEntries.begin();
    SDL_Surface *surface = IMG_Load(entry.texturePath.c_str());
    entry.image = surface;

    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    std::vector<TextureAtlasEntry> entries;
    entries.push_back({
        .type = entry.type,
        .start = {0.0f, 0.0f},
        .end = {1.0f, 1.0f},
    });
    SDL_DestroySurface(entry.image);
    glGenerateMipmap(GL_TEXTURE_2D);

    return std::make_unique<TextureAtlas>(texture, entries);
  } else {
    // If there are 2 or more entries in the list, we want to create a texture atlas with all textures copied into a
    // single, larger texture
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    std::vector<TextureAtlasEntry> entries;
    int offsetX = 0;
    int offsetY = 0;

    for (auto &entry : mEntries) {
      glTextureSubImage2D(texture, 0, offsetX, offsetY, mTileSize, mTileSize, GL_RGBA, GL_UNSIGNED_BYTE,
                          entry.image->pixels);

      // TODO: Here we are removing 1px from all sides of the texture to prevent a black artifact around the edges of
      // the texture There is probably a better way to do it, e.g. to create textures with +2 on both sides, repeating
      // the textures outer 1 pixel, essentially adding a border to the texture.
      entries.push_back({
          .type = entry.type,
          .start = {(offsetX + 1) / size, (offsetY + 1) / size},
          .end = {(offsetX + mTileSize - 1) / size, (offsetY + mTileSize - 1) / size},
      });

      offsetX += mTileSize;
      if (offsetX + mTileSize >= size) {
        offsetX = 0;
        offsetY += mTileSize;
      }

      SDL_DestroySurface(entry.image);
    }

    glGenerateMipmap(GL_TEXTURE_2D);

    return std::make_unique<TextureAtlas>(texture, entries);
  }
}

void TextureAtlas::Bind(const unsigned int unit) const {
  switch (unit) {
    case 0:
      glActiveTexture(GL_TEXTURE0);
      break;
    case 1:
      glActiveTexture(GL_TEXTURE1);
      break;
    case 2:
      glActiveTexture(GL_TEXTURE2);
      break;
    case 3:
      glActiveTexture(GL_TEXTURE3);
      break;
    case 4:
      glActiveTexture(GL_TEXTURE4);
      break;
    case 5:
      glActiveTexture(GL_TEXTURE5);
      break;
    default:
      // TODO: Not implemented for more than 5
      assert(unit <= 5);
      break;
  }
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
