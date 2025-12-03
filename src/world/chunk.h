#pragma once

#include "core/texture.h"
#include "cube.h"
#include "texture_atlas.h"
#include "tile.h"
#include <GL/glew.h>
#include <cstdint>
#include <glm/glm.hpp>

typedef uint64_t u64;

struct VoxelGrid {
  static const int SIZE = 64;
  u64 columns[SIZE * SIZE];

  bool operator()(int x, int y, int z) const {
    return columns[x * SIZE + z] & (1 << y);
  }
};

struct Vertex {
  glm::vec3 position;
  glm::vec2 textureCoords;
};

struct Chunk {
  VoxelGrid mGrid;
  glm::ivec3 mPosition, mDimensions;
  int mSeed;
  std::vector<Vertex> mVertices;
  const TextureAtlas &mTextureAtlas;

  GLuint mVao, mVbo;

  Chunk(const TextureAtlas &atlas, const glm::ivec3 &postion, const glm::ivec3 &dimensions, int seed);
  ~Chunk();

  void GenerateVertices();
  void SetupVAO();

  void Render();

private:
  void AddCubeFace(Tile tile, CubeFace face, int x, int y, int z);
};
