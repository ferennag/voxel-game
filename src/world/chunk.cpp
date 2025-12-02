#include "chunk.h"
#include <SDL3_image/SDL_image.h>
#include <cstdlib>

Chunk::Chunk(const glm::ivec3 &position, const glm::ivec3 &dimensions, int seed)
    : mPosition(position), mDimensions(dimensions), mSeed(seed) {
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

        if (y == 0 || mTiles[x][y - 1][z] == Tile::Empty) {
          AddCubeFace(CubeFace::Top, x, y, z);
        }

        if (y == mDimensions.y - 1 || mTiles[x][y + 1][z] == Tile::Empty) {
          AddCubeFace(CubeFace::Bottom, x, y, z);
        }

        if (x == 0 || mTiles[x - 1][y][z] == Tile::Empty) {
          AddCubeFace(CubeFace::Left, x, y, z);
        }

        if (x == mDimensions.x - 1 || mTiles[x + 1][y][z] == Tile::Empty) {
          AddCubeFace(CubeFace::Right, x, y, z);
        }

        if (z == 0 || mTiles[x][y][z - 1] == Tile::Empty) {
          AddCubeFace(CubeFace::Back, x, y, z);
        }

        if (z == mDimensions.z - 1 || mTiles[x][y][z + 1] == Tile::Empty) {
          AddCubeFace(CubeFace::Front, x, y, z);
        }
      }
    }
  }

  mTexture = Texture::Load("assets/textures/dirt.png");

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
  mTexture->Bind();
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glBindVertexArray(mVao);
  glDrawArrays(GL_TRIANGLES, 0, mVertices.size());
  glBindVertexArray(0);
}

void Chunk::AddCubeFace(CubeFace face, int x, int y, int z) {
  switch (face) {
    case CubeFace::Front: {
      mVertices.push_back(Vertex{{x - 0.5f, y - 0.5f, z + 0.5f}, {0.0f, 0.0f}});
      mVertices.push_back(Vertex{{x + 0.5f, y - 0.5f, z + 0.5f}, {1.0f, 0.0f}});
      mVertices.push_back(Vertex{{x - 0.5f, y + 0.5f, z + 0.5f}, {0.0f, 1.0f}});

      mVertices.push_back(Vertex{{x + 0.5f, y - 0.5f, z + 0.5f}, {1.0f, 0.0f}});
      mVertices.push_back(Vertex{{x + 0.5f, y + 0.5f, z + 0.5f}, {1.0f, 1.0f}});
      mVertices.push_back(Vertex{{x - 0.5f, y + 0.5f, z + 0.5f}, {0.0f, 1.0f}});
      break;
    }
    case CubeFace::Back: {
      mVertices.push_back(Vertex{{x + 0.5f, y - 0.5f, z - 0.5f}, {0.0f, 0.0f}});
      mVertices.push_back(Vertex{{x - 0.5f, y - 0.5f, z - 0.5f}, {1.0f, 0.0f}});
      mVertices.push_back(Vertex{{x + 0.5f, y + 0.5f, z - 0.5f}, {0.0f, 1.0f}});

      mVertices.push_back(Vertex{{x + 0.5f, y + 0.5f, z - 0.5f}, {0.0f, 1.0f}});
      mVertices.push_back(Vertex{{x - 0.5f, y - 0.5f, z - 0.5f}, {1.0f, 0.0f}});
      mVertices.push_back(Vertex{{x - 0.5f, y + 0.5f, z - 0.5f}, {1.0f, 1.0f}});
      break;
    }

    case CubeFace::Left: {
      mVertices.push_back(Vertex{{x - 0.5f, y - 0.5f, z - 0.5f}, {1.0f, 0.0f}});
      mVertices.push_back(Vertex{{x - 0.5f, y - 0.5f, z + 0.5f}, {0.0f, 0.0f}});
      mVertices.push_back(Vertex{{x - 0.5f, y + 0.5f, z - 0.5f}, {1.0f, 1.0f}});

      mVertices.push_back(Vertex{{x - 0.5f, y + 0.5f, z - 0.5f}, {1.0f, 1.0f}});
      mVertices.push_back(Vertex{{x - 0.5f, y - 0.5f, z + 0.5f}, {0.0f, 0.0f}});
      mVertices.push_back(Vertex{{x - 0.5f, y + 0.5f, z + 0.5f}, {0.0f, 1.0f}});
      break;
    }

    case CubeFace::Right: {
      mVertices.push_back(Vertex{{x + 0.5f, y - 0.5f, z + 0.5f}, {1.0f, 0.0f}});
      mVertices.push_back(Vertex{{x + 0.5f, y - 0.5f, z - 0.5f}, {0.0f, 0.0f}});
      mVertices.push_back(Vertex{{x + 0.5f, y + 0.5f, z + 0.5f}, {1.0f, 1.0f}});

      mVertices.push_back(Vertex{{x + 0.5f, y + 0.5f, z + 0.5f}, {1.0f, 1.0f}});
      mVertices.push_back(Vertex{{x + 0.5f, y - 0.5f, z - 0.5f}, {0.0f, 0.0f}});
      mVertices.push_back(Vertex{{x + 0.5f, y + 0.5f, z - 0.5f}, {0.0f, 1.0f}});
      break;
    }

    case CubeFace::Top: {
      mVertices.push_back(Vertex{{x - 0.5f, y + 0.5f, z + 0.5f}, {0.0f, 1.0f}});
      mVertices.push_back(Vertex{{x + 0.5f, y + 0.5f, z + 0.5f}, {1.0f, 1.0f}});
      mVertices.push_back(Vertex{{x - 0.5f, y + 0.5f, z - 0.5f}, {0.0f, 0.0f}});

      mVertices.push_back(Vertex{{x - 0.5f, y + 0.5f, z - 0.5f}, {0.0f, 0.0f}});
      mVertices.push_back(Vertex{{x + 0.5f, y + 0.5f, z + 0.5f}, {1.0f, 1.0f}});
      mVertices.push_back(Vertex{{x + 0.5f, y + 0.5f, z - 0.5f}, {1.0f, 0.0f}});
      break;
    }

    case CubeFace::Bottom: {
      mVertices.push_back(Vertex{{x - 0.5f, y - 0.5f, z + 0.5f}, {0.0f, 0.0f}});
      mVertices.push_back(Vertex{{x - 0.5f, y - 0.5f, z - 0.5f}, {0.0f, 1.0f}});
      mVertices.push_back(Vertex{{x + 0.5f, y - 0.5f, z - 0.5f}, {1.0f, 1.0f}});

      mVertices.push_back(Vertex{{x + 0.5f, y - 0.5f, z - 0.5f}, {1.0f, 1.0f}});
      mVertices.push_back(Vertex{{x + 0.5f, y - 0.5f, z + 0.5f}, {1.0f, 0.0f}});
      mVertices.push_back(Vertex{{x - 0.5f, y - 0.5f, z + 0.5f}, {0.0f, 0.0f}});
      break;
    }
  }
}
