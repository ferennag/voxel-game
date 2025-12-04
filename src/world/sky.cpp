#include "sky.h"
#include "SDL3/SDL_surface.h"
#include "core/shader.h"
#include <SDL3_image/SDL_image.h>
#include <string>
#include <vector>

float skyboxVertices[] = {
    // positions
    -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
    -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

    1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

Sky::Sky() {
  glGenTextures(1, &mTexture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);

  const int SIZE = 512;
  SDL_Surface *image = IMG_Load("assets/sky/sky.png");
  SDL_Surface *face = SDL_CreateSurface(SIZE, SIZE, image->format);

  std::vector<SDL_Rect> faceRects;
  faceRects.push_back({2 * SIZE, SIZE, SIZE, SIZE}); // right
  faceRects.push_back({0, SIZE, SIZE, SIZE});        // left
  faceRects.push_back({SIZE, 0, SIZE, SIZE});        // top
  faceRects.push_back({SIZE, 2 * SIZE, SIZE, SIZE}); // bottom
  faceRects.push_back({SIZE, SIZE, SIZE, SIZE});     // front
  faceRects.push_back({3 * SIZE, SIZE, SIZE, SIZE}); // back

  int i = 0;
  for (auto &faceRect : faceRects) {
    SDL_BlitSurface(image, &faceRect, face, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, SIZE, SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, face->pixels);
    ++i;
  }

  SDL_DestroySurface(image);
  SDL_DestroySurface(face);

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  glGenVertexArrays(1, &mVao);
  glGenBuffers(1, &mVbo);
  glBindVertexArray(mVao);
  glBindBuffer(GL_ARRAY_BUFFER, mVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  mShader = Shader::Load("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
}

Sky::~Sky() {
  glDeleteTextures(1, &mTexture);
  glDeleteVertexArrays(1, &mVao);
  glDeleteBuffers(1, &mVbo);
}

void Sky::Render(const glm::mat4 &projection, const glm::mat4 &view) const {
  glDepthMask(GL_FALSE);
  glBindVertexArray(mVao);
  glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);
  mShader->Bind();
  mShader->UniformMat4("projection", projection);
  mShader->UniformMat4("view", glm::mat4(glm::mat3(view)));
  glDrawArrays(GL_TRIANGLES, 0, sizeof(skyboxVertices) / sizeof(float));
  glDepthMask(GL_TRUE);
}
