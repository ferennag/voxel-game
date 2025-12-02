#pragma once

#include "core/texture.h"
#include <GL/glew.h>
#include <glm/glm.hpp>

enum class Tile {
  Empty,
  Dirt,
  ALL
};

enum class CubeFace {
  Front,
  Back,
  Left,
  Right,
  Top,
  Bottom
};

struct Vertex {
  float position[3];
  float textureCoords[2];
};

struct Chunk {
  glm::ivec3 mPosition, mDimensions;
  int mSeed;
  Tile ***mTiles;
  std::vector<Vertex> mVertices;
  std::shared_ptr<Texture> mTexture;

  GLuint mVao, mVbo;

  Chunk(const glm::ivec3 &postion, const glm::ivec3 &dimensions, int seed);
  ~Chunk();

  void Render();

private:
  void AddCubeFace(CubeFace face, int x, int y, int z);
};
