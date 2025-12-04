#include "chunk.h"
#include "SDL3/SDL_log.h"
#include <FastNoiseLite.h>
#include <SDL3_image/SDL_image.h>

Chunk::Chunk(const TextureAtlas &atlas, const glm::ivec3 &position, const glm::ivec3 &dimensions, int seed)
    : mPosition(position), mDimensions(dimensions), mSeed(seed), mTextureAtlas(atlas) {
}

void Chunk::GenerateVertices() {
  VoxelGrid grid;

  FastNoiseLite noise;
  noise.SetSeed(mSeed);
  noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
  noise.SetFrequency(0.010);
  noise.SetFractalType(FastNoiseLite::FractalType_FBm);

  for (int z = 0; z < VoxelGrid::SIZE; ++z) {
    for (int x = 0; x < VoxelGrid::SIZE; ++x) {
      float nX = static_cast<float>(mPosition.x) * VoxelGrid::SIZE + static_cast<float>(x);
      float nY = static_cast<float>(mPosition.y) * VoxelGrid::SIZE;
      float nZ = static_cast<float>(mPosition.z) * VoxelGrid::SIZE + static_cast<float>(z);
      float value = noise.GetNoise(nX, nY, nZ);
      // Normalize the noise value between 0.0f, and 1.0f
      value = (value + 1.0f / 2.0f);

      int height = std::floor(value * VoxelGrid::SIZE);
      u64 column = (1 << height) - 1;
      grid.columns[x * VoxelGrid::SIZE + z] = column;
    }
  }

  mGrid = grid;

  // TODO: As a next step:
  // - implement Greedy Meshing algorithm
  // - currently there are duplicated vertices, we should use indexing to reduce the number of vertices
  for (int z = 0; z < mDimensions.z; z++) {
    for (int y = 0; y < mDimensions.y; y++) {
      for (int x = 0; x < mDimensions.x; x++) {
        // TODO: hardcode the tile to dirt for now. I need to separate the vertex information from the texture
        // information
        const auto tile = Tile::Dirt;
        if (!grid(x, y, z)) {
          continue;
        }

        if (y == (mDimensions.y - 1) || !grid(x, y + 1, z)) {
          AddCubeFace(tile, CubeFace::Top, x, y, z);
        }

        if (y == 0 || !grid(x, y - 1, z)) {
          AddCubeFace(tile, CubeFace::Bottom, x, y, z);
        }

        if (x == 0 || !grid(x - 1, y, z)) {
          AddCubeFace(tile, CubeFace::Left, x, y, z);
        }

        if (x == mDimensions.x - 1 || !grid(x + 1, y, z)) {
          AddCubeFace(tile, CubeFace::Right, x, y, z);
        }

        if (z == 0 || !grid(x, y, z - 1)) {
          AddCubeFace(tile, CubeFace::Back, x, y, z);
        }

        if (z == mDimensions.z - 1 || !grid(x, y, z + 1)) {
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
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, normal)));
  glEnableVertexAttribArray(2);
}

Chunk::~Chunk() {
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

  glm::vec3 normalTop = {0.0, 1.0, 0.0};
  glm::vec3 normalBottom = {0.0, -1.0, 0.0};
  glm::vec3 normalLeft = {-1.0, 0.0, 0.0};
  glm::vec3 normalRight = {1.0, 0.0, 0.0};
  glm::vec3 normalFront = {0.0, 0.0, -1.0};
  glm::vec3 normalBack = {0.0, 0.0, 1.0};

  switch (face) {
    case CubeFace::Front: {
      mVertices.push_back(Vertex{frontBottomLeft, texture.BottomLeft(), normalFront});
      mVertices.push_back(Vertex{frontBottomRight, texture.BottomRight(), normalFront});
      mVertices.push_back(Vertex{frontTopLeft, texture.TopLeft(), normalFront});

      mVertices.push_back(Vertex{frontBottomRight, texture.BottomRight(), normalFront});
      mVertices.push_back(Vertex{frontTopRight, texture.TopRight(), normalFront});
      mVertices.push_back(Vertex{frontTopLeft, texture.TopLeft(), normalFront});
      break;
    }
    case CubeFace::Back: {
      mVertices.push_back(Vertex{backBottomLeft, texture.BottomRight(), normalBack});
      mVertices.push_back(Vertex{backTopLeft, texture.TopRight(), normalBack});
      mVertices.push_back(Vertex{backTopRight, texture.TopLeft(), normalBack});

      mVertices.push_back(Vertex{backTopRight, texture.TopLeft(), normalBack});
      mVertices.push_back(Vertex{backBottomRight, texture.BottomLeft(), normalBack});
      mVertices.push_back(Vertex{backBottomLeft, texture.BottomRight(), normalBack});
      break;
    }

    case CubeFace::Left: {
      mVertices.push_back(Vertex{backBottomLeft, texture.BottomLeft(), normalLeft});
      mVertices.push_back(Vertex{frontBottomLeft, texture.BottomRight(), normalLeft});
      mVertices.push_back(Vertex{backTopLeft, texture.TopLeft(), normalLeft});

      mVertices.push_back(Vertex{frontBottomLeft, texture.BottomRight(), normalLeft});
      mVertices.push_back(Vertex{frontTopLeft, texture.TopRight(), normalLeft});
      mVertices.push_back(Vertex{backTopLeft, texture.TopLeft(), normalLeft});
      break;
    }

    case CubeFace::Right: {
      mVertices.push_back(Vertex{frontBottomRight, texture.BottomLeft(), normalRight});
      mVertices.push_back(Vertex{backBottomRight, texture.BottomRight(), normalRight});
      mVertices.push_back(Vertex{frontTopRight, texture.TopLeft(), normalRight});

      mVertices.push_back(Vertex{frontTopRight, texture.TopLeft(), normalRight});
      mVertices.push_back(Vertex{backBottomRight, texture.BottomRight(), normalRight});
      mVertices.push_back(Vertex{backTopRight, texture.TopRight(), normalRight});
      break;
    }

    case CubeFace::Top: {
      mVertices.push_back(Vertex{frontTopLeft, texture.BottomLeft(), normalTop});
      mVertices.push_back(Vertex{frontTopRight, texture.BottomRight(), normalTop});
      mVertices.push_back(Vertex{backTopLeft, texture.TopLeft(), normalTop});

      mVertices.push_back(Vertex{backTopLeft, texture.TopLeft(), normalTop});
      mVertices.push_back(Vertex{frontTopRight, texture.BottomRight(), normalTop});
      mVertices.push_back(Vertex{backTopRight, texture.TopRight(), normalTop});

      break;
    }

    case CubeFace::Bottom: {
      mVertices.push_back(Vertex{frontBottomLeft, texture.BottomRight(), normalBottom});
      mVertices.push_back(Vertex{backBottomLeft, texture.TopRight(), normalBottom});
      mVertices.push_back(Vertex{backBottomRight, texture.TopLeft(), normalBottom});

      mVertices.push_back(Vertex{backBottomRight, texture.TopLeft(), normalBottom});
      mVertices.push_back(Vertex{frontBottomRight, texture.BottomLeft(), normalBottom});
      mVertices.push_back(Vertex{frontBottomLeft, texture.BottomRight(), normalBottom});
      break;
    }
  }
}
