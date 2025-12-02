#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include "chunk.h"
#include "core/shader.h"
#include "texture_atlas.h"
#include <glm/glm.hpp>
#include <memory>

class World {
public:
  World(const int seed, const glm::ivec3 &chunkDimensions);
  ~World();

  void Render(const Shader &shader);

private:
  int mSeed;
  glm::ivec3 mChunkDimensions;
  std::unordered_map<glm::ivec3, Chunk *> mChunks;
  std::unique_ptr<TextureAtlas> mTextureAtlas;

  SDL_Thread *EnsureChunkExists(const glm::ivec3 &chunkPosition);
};
