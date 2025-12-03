#pragma once

#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <vector>

enum class TextureType {
  Dirt,
  Sand,
};

struct TextureAtlasEntry {
  TextureType type;
  glm::vec2 start;
  glm::vec2 end;

  glm::vec2 BottomLeft() const;
  glm::vec2 BottomRight() const;
  glm::vec2 TopLeft() const;
  glm::vec2 TopRight() const;
};

class TextureAtlas {
  GLuint mId;
  std::vector<TextureAtlasEntry> mEntries;

public:
  TextureAtlas(const GLuint id, const std::vector<TextureAtlasEntry> &entries);
  ~TextureAtlas();

  void Bind(const unsigned int unit) const;

  std::optional<TextureAtlasEntry> GetTexture(const TextureType type) const;
};

class TextureAtlasBuilder {
  struct Entry {
    TextureType type;
    std::string texturePath;
    SDL_Surface *image;
  };

  std::vector<Entry> mEntries;
  int mTileSize;

public:
  TextureAtlasBuilder(const int tileSize);
  void AddTexture(const TextureType type, const std::string &texture);
  std::unique_ptr<TextureAtlas> Build();
};
