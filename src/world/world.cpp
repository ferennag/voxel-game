#include "world.h"
#include "core/profiler.h"
#include "glm/ext/matrix_transform.hpp"
#include <memory>
#include <utility>

World::World(const int seed, const glm::ivec3 &chunkDimensions) : mSeed(seed), mChunkDimensions(chunkDimensions) {
  auto profiler = Profiler::Create();
  TextureAtlasBuilder atlasBuilder(16);
  atlasBuilder.AddTexture(TextureType::Dirt, "assets/textures/dirt.png");
  atlasBuilder.AddTexture(TextureType::Sand, "assets/textures/sand.png");
  mTextureAtlas = atlasBuilder.Build();
  profiler.LogSnapshot("TextureAtlas build");

  glm::ivec3 dim = {2, 1, 2};

  for (int x = -dim.x; x < dim.x; ++x) {
    for (int y = -dim.y; y < dim.y; ++y) {
      for (int z = -dim.z; z < dim.z; ++z) {
        EnsureChunkExists({x, y, z});
        profiler.LogSnapshot("Chunk generation");
      }
    }
  }

  profiler.LogEnd("World building completed");
}

void World::EnsureChunkExists(const glm::ivec3 &chunkPosition) {
  if (mChunks.contains(chunkPosition)) {
    return;
  }

  mChunks.insert(
      std::make_pair(chunkPosition, std::make_unique<Chunk>(*mTextureAtlas, chunkPosition, mChunkDimensions, mSeed)));
}

void World::Render(const Shader &shader) {
  mTextureAtlas->Bind();

  for (auto &pair : mChunks) {
    const glm::vec3 translationVector = pair.second->mPosition * mChunkDimensions;
    const glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), translationVector);

    shader.UniformMat4("model", model);
    pair.second->Render();
  }
}
