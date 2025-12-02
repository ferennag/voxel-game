#pragma once

#include "core/texture.h"
#include "cube.h"
#include "texture_atlas.h"
#include "tile.h"
#include <GL/glew.h>
#include <glm/glm.hpp>

struct Vertex {
  glm::vec3 position;
  glm::vec2 textureCoords;
};

struct Chunk {
  glm::ivec3 mPosition, mDimensions;
  int mSeed;
  Tile ***mTiles;
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
