#include "world.h"
#include "SDL3/SDL_thread.h"
#include "core/profiler.h"
#include "glm/ext/matrix_transform.hpp"
#include <memory>
#include <utility>

int GenerateChunkVertices(void *data) {
  auto profiler = Profiler::Create();
  Chunk *chunk = static_cast<Chunk *>(data);
  chunk->GenerateVertices();
  profiler.LogEnd("Chunk generated");
  return 0;
}

World::World(const int seed, const glm::ivec3 &chunkDimensions) : mSeed(seed), mChunkDimensions(chunkDimensions) {
  auto profiler = Profiler::Create();
  TextureAtlasBuilder atlasBuilder(16);
  atlasBuilder.AddTexture(TextureType::Dirt, "assets/textures/dirt.png");
  atlasBuilder.AddTexture(TextureType::Sand, "assets/textures/sand.png");
  mTextureAtlas = atlasBuilder.Build();
  profiler.LogSnapshot("TextureAtlas build");

  glm::ivec3 dim = {2, 1, 2};

  std::vector<SDL_Thread *> threads;
  for (int x = -dim.x; x < dim.x; ++x) {
    for (int y = -dim.y; y < dim.y; ++y) {
      for (int z = -dim.z; z < dim.z; ++z) {
        auto *thread = EnsureChunkExists({x, y, z});
        if (thread != nullptr) {
          threads.push_back(thread);
        }
      }
    }
  }
  profiler.LogSnapshot("Chunk generation");

  for (auto *thread : threads) {
    int status;
    SDL_WaitThread(thread, &status);
  }

  for (auto &entry : mChunks) {
    entry.second->SetupVAO();
  }

  profiler.LogEnd("World building completed");
}

World::~World() {
  for (auto &entry : mChunks) {
    delete entry.second;
  }

  mChunks.clear();
}

SDL_Thread *World::EnsureChunkExists(const glm::ivec3 &chunkPosition) {
  if (mChunks.contains(chunkPosition)) {
    return nullptr;
  }

  auto *chunk = new Chunk(*mTextureAtlas, chunkPosition, mChunkDimensions, mSeed);

  mChunks.insert(std::make_pair(chunkPosition, chunk));

  return SDL_CreateThread(GenerateChunkVertices, "GenerateChunkVertices", (void *)chunk);
}

void World::Render(const Shader &shader) {
  mTextureAtlas->Bind();

  for (auto &pair : mChunks) {
    glm::vec3 translationVector = pair.second->mPosition * mChunkDimensions;
    const glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), translationVector);

    shader.UniformMat4("model", model);
    pair.second->Render();
  }
}
