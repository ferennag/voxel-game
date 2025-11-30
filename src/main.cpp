#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/trigonometric.hpp"
#include <GL/glew.h>

#define SDL_MAIN_USE_CALLBACKS
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_projection.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <memory>

#include "core/shader.h"

GLfloat vertices[] = {
    -0.5f, -0.5f, 0.0f,

    0.5f,  -0.5f, 0.0f,

    0.0f,  0.5f,  0.0f,
};

struct GameState {
  SDL_Window *window;
  SDL_GLContext glContext;
  std::unique_ptr<Shader> shader;
  GLuint vao, vbo;
  glm::mat4 projection, model, view;
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

  SDL_Window *window =
      SDL_CreateWindow("Voxel Game", 1024, 768, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

  if (!window) {
    SDL_Log("Failed to create SDL window: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  auto glContext = SDL_GL_CreateContext(window);
  if (!glContext) {
    SDL_Log("Failed to create OpenGL context: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_GL_MakeCurrent(window, glContext);
  SDL_GL_SetSwapInterval(1);

  glewInit();

  GameState *state = new GameState();
  *appstate = state;

  state->window = window;
  state->glContext = glContext;
  state->shader = Shader::Load("assets/shaders/basic.vert", "assets/shaders/basic.frag");

  glCreateVertexArrays(1, &state->vao);
  glCreateBuffers(1, &state->vbo);
  glBindVertexArray(state->vao);
  glBindBuffer(GL_ARRAY_BUFFER, state->vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
  glEnableVertexAttribArray(0);

  state->model = glm::identity<glm::mat4>();
  state->view = glm::lookAt(glm::vec3{0.0f, 1.0f, -5.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f});

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  GameState *state = static_cast<GameState *>(appstate);
  switch (event->type) {
    case SDL_EVENT_QUIT: {
      return SDL_APP_SUCCESS;
    }
    case SDL_EVENT_WINDOW_RESIZED: {
      auto width = event->window.data1;
      auto height = event->window.data2;
      glViewport(0, 0, width, height);
      state->projection = glm::perspectiveFov(glm::radians(45.0f), static_cast<float>(width),
                                              static_cast<float>(height), 0.1f, 1000.0f);
      break;
    }
    case SDL_EVENT_KEY_DOWN: {
      switch (event->key.key) {
        case SDLK_ESCAPE: {
          return SDL_APP_SUCCESS;
        }
      }

      break;
    }
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  GameState *state = static_cast<GameState *>(appstate);
  glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  state->shader->Bind();
  state->shader->UniformMat4("projection", state->projection);
  state->shader->UniformMat4("view", state->view);
  state->shader->UniformMat4("model", state->model);
  glBindVertexArray(state->vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  state->shader->Unbind();

  SDL_GL_SwapWindow(state->window);
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  if (appstate) {
    GameState *state = static_cast<GameState *>(appstate);
    glDeleteVertexArrays(1, &state->vao);
    glDeleteBuffers(1, &state->vbo);
    delete state;
  }
}
