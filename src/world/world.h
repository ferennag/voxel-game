#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include "chunk.h"
#include "core/shader.h"
#include <glm/glm.hpp>
#include <memory>

class World {
public:
  World(const int seed, const glm::ivec3 &chunkDimensions);
  ~World() = default;

  void Render(const Shader &shader);

private:
  int mSeed;
  glm::ivec3 mChunkDimensions;
  std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>> mChunks;

  void EnsureChunkExists(const glm::ivec3 &chunkPosition);
};
