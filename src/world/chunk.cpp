#include "chunk.h"
#include "SDL3/SDL_log.h"
#include <SDL3_image/SDL_image.h>
#include <cstdlib>

Chunk::Chunk(const TextureAtlas &atlas, const glm::ivec3 &position, const glm::ivec3 &dimensions, int seed)
    : mPosition(position), mDimensions(dimensions), mSeed(seed), mTextureAtlas(atlas) {
}

void Chunk::GenerateVertices() {
  mTiles = new Tile **[mDimensions.x];
  for (int i = 0; i < mDimensions.x; i++) {
    mTiles[i] = new Tile *[mDimensions.y];
    for (int j = 0; j < mDimensions.y; j++) {
      mTiles[i][j] = new Tile[mDimensions.z];
    }
  }

  for (int z = 0; z < mDimensions.z; z++) {
    for (int y = 0; y < mDimensions.y; y++) {
      for (int x = 0; x < mDimensions.x; x++) {
        mTiles[x][y][z] = static_cast<Tile>(rand() % static_cast<int>(Tile::ALL));
      }
    }
  }

  // TODO: As a next step:
  // - implement an algorithm that joins adjacent vertexes
  // - currently there are duplicated vertices, we should use indexing to reduce the number of vertices
  for (int z = 0; z < mDimensions.z; z++) {
    for (int y = 0; y < mDimensions.y; y++) {
      for (int x = 0; x < mDimensions.x; x++) {
        const auto tile = mTiles[x][y][z];
        if (tile == Tile::Empty) {
          continue;
        }

        if (y == (mDimensions.y - 1) || mTiles[x][y + 1][z] == Tile::Empty) {
          AddCubeFace(tile, CubeFace::Top, x, y, z);
        }

        if (y == 0 || mTiles[x][y - 1][z] == Tile::Empty) {
          AddCubeFace(tile, CubeFace::Bottom, x, y, z);
        }

        if (x == 0 || mTiles[x - 1][y][z] == Tile::Empty) {
          AddCubeFace(tile, CubeFace::Left, x, y, z);
        }

        if (x == mDimensions.x - 1 || mTiles[x + 1][y][z] == Tile::Empty) {
          AddCubeFace(tile, CubeFace::Right, x, y, z);
        }

        if (z == 0 || mTiles[x][y][z - 1] == Tile::Empty) {
          AddCubeFace(tile, CubeFace::Back, x, y, z);
        }

        if (z == mDimensions.z - 1 || mTiles[x][y][z + 1] == Tile::Empty) {
          AddCubeFace(tile, CubeFace::Front, x, y, z);
        }
      }
    }
  }

  SDL_Log("Chunk (%d, %d, %d): %zu vertices", mPosition.x, mPosition.y, mPosition.z, mVertices.size());
}

void Chunk::SetupVAO() {
  glCreateVertexArrays(1, &mVao);
  glCreateBuffers(1, &mVbo);
  glBindVertexArray(mVao);
  glBindBuffer(GL_ARRAY_BUFFER, mVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * mVertices.size(), mVertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, position)));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, textureCoords)));
  glEnableVertexAttribArray(1);
}

Chunk::~Chunk() {
  for (int x = 0; x < mDimensions.x; x++) {
    for (int y = 0; y < mDimensions.y; y++) {
      delete[] mTiles[x][y];
    }
    delete[] mTiles[x];
  }

  delete[] mTiles;
  glDeleteVertexArrays(1, &mVao);
  glDeleteBuffers(1, &mVbo);
}

void Chunk::Render() {
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glBindVertexArray(mVao);
  glDrawArrays(GL_TRIANGLES, 0, mVertices.size());
  glBindVertexArray(0);
}

void Chunk::AddCubeFace(Tile tile, CubeFace face, int x, int y, int z) {
  TextureType textureType;
  switch (tile) {
    case Tile::Dirt:
      textureType = TextureType::Dirt;
      break;
    case Tile::Sand:
      textureType = TextureType::Sand;
      break;
    default:
      textureType = TextureType::Dirt;
      break;
  }

  auto texture = *mTextureAtlas.GetTexture(textureType);

  glm::vec3 frontTopLeft = {x - 0.5f, y + 0.5f, z + 0.5f};
  glm::vec3 frontTopRight = {x + 0.5f, y + 0.5f, z + 0.5f};
  glm::vec3 frontBottomLeft = {x - 0.5f, y - 0.5f, z + 0.5f};
  glm::vec3 frontBottomRight = {x + 0.5f, y - 0.5f, z + 0.5f};

  glm::vec3 backTopLeft = {x - 0.5f, y + 0.5f, z - 0.5f};
  glm::vec3 backTopRight = {x + 0.5f, y + 0.5f, z - 0.5f};
  glm::vec3 backBottomLeft = {x - 0.5f, y - 0.5f, z - 0.5f};
  glm::vec3 backBottomRight = {x + 0.5f, y - 0.5f, z - 0.5f};

  switch (face) {
    case CubeFace::Front: {
      mVertices.push_back(Vertex{frontBottomLeft, texture.BottomLeft()});
      mVertices.push_back(Vertex{frontBottomRight, texture.BottomRight()});
      mVertices.push_back(Vertex{frontTopLeft, texture.TopLeft()});

      mVertices.push_back(Vertex{frontBottomRight, texture.BottomRight()});
      mVertices.push_back(Vertex{frontTopRight, texture.TopRight()});
      mVertices.push_back(Vertex{frontTopLeft, texture.TopLeft()});
      break;
    }
    case CubeFace::Back: {
      mVertices.push_back(Vertex{backBottomLeft, texture.BottomRight()});
      mVertices.push_back(Vertex{backTopLeft, texture.TopRight()});
      mVertices.push_back(Vertex{backTopRight, texture.TopLeft()});

      mVertices.push_back(Vertex{backTopRight, texture.TopLeft()});
      mVertices.push_back(Vertex{backBottomRight, texture.BottomLeft()});
      mVertices.push_back(Vertex{backBottomLeft, texture.BottomRight()});
      break;
    }

    case CubeFace::Left: {
      mVertices.push_back(Vertex{backBottomLeft, texture.BottomLeft()});
      mVertices.push_back(Vertex{frontBottomLeft, texture.BottomRight()});
      mVertices.push_back(Vertex{backTopLeft, texture.TopLeft()});

      mVertices.push_back(Vertex{frontBottomLeft, texture.BottomRight()});
      mVertices.push_back(Vertex{frontTopLeft, texture.TopRight()});
      mVertices.push_back(Vertex{backTopLeft, texture.TopLeft()});
      break;
    }

    case CubeFace::Right: {
      mVertices.push_back(Vertex{frontBottomRight, texture.BottomLeft()});
      mVertices.push_back(Vertex{backBottomRight, texture.BottomRight()});
      mVertices.push_back(Vertex{frontTopRight, texture.TopLeft()});

      mVertices.push_back(Vertex{frontTopRight, texture.TopLeft()});
      mVertices.push_back(Vertex{backBottomRight, texture.BottomRight()});
      mVertices.push_back(Vertex{backTopRight, texture.TopRight()});
      break;
    }

    case CubeFace::Top: {
      mVertices.push_back(Vertex{frontTopLeft, texture.BottomLeft()});
      mVertices.push_back(Vertex{frontTopRight, texture.BottomRight()});
      mVertices.push_back(Vertex{backTopLeft, texture.TopLeft()});

      mVertices.push_back(Vertex{backTopLeft, texture.TopLeft()});
      mVertices.push_back(Vertex{frontTopRight, texture.BottomRight()});
      mVertices.push_back(Vertex{backTopRight, texture.TopRight()});

      break;
    }

    case CubeFace::Bottom: {
      mVertices.push_back(Vertex{frontBottomLeft, texture.BottomRight()});
      mVertices.push_back(Vertex{backBottomLeft, texture.TopRight()});
      mVertices.push_back(Vertex{backBottomRight, texture.TopLeft()});

      mVertices.push_back(Vertex{backBottomRight, texture.TopLeft()});
      mVertices.push_back(Vertex{frontBottomRight, texture.BottomLeft()});
      mVertices.push_back(Vertex{frontBottomLeft, texture.BottomRight()});
      break;
    }
  }
}
