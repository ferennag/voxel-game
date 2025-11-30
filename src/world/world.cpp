#include "world.h"
#include "glm/ext/matrix_transform.hpp"
#include <memory>
#include <utility>

World::World(const int seed, const glm::ivec3 &chunkDimensions) : mSeed(seed), mChunkDimensions(chunkDimensions) {
  for (int x = -1; x < 1; ++x) {
    for (int y = 0; y < 1; ++y) {
      for (int z = -1; z < 1; ++z) {
        EnsureChunkExists({x, y, z});
      }
    }
  }
}

void World::EnsureChunkExists(const glm::ivec3 &chunkPosition) {
  if (mChunks.contains(chunkPosition)) {
    return;
  }

  mChunks.insert(std::make_pair(chunkPosition, std::make_unique<Chunk>(chunkPosition, mChunkDimensions, mSeed)));
}

void World::Render(const Shader &shader) {
  for (auto &pair : mChunks) {
    const glm::vec3 translationVector = pair.second->mPosition * mChunkDimensions;
    const glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), translationVector);

    shader.UniformMat4("model", model);
    pair.second->Render();
  }
}
