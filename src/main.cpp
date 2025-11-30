#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_video.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include <GL/glew.h>
#include <cmath>

#define SDL_MAIN_USE_CALLBACKS
#include "glm/ext/matrix_float4x4.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <memory>

#include "core/shader.h"
#include "world/chunk.h"

enum class Key {
  Left,
  Right,
  Up,
  Down,
  Jump,
  Crouch,
  ALL,
};

struct KeyboardState {
  bool pressed[static_cast<long>(Key::ALL)];
};

struct GameState {
  SDL_Window *window;
  SDL_GLContext glContext;
  KeyboardState keyboard;
  std::unique_ptr<Shader> shader;
  glm::mat4 projection, model, view;
  std::unique_ptr<Chunk> chunk;
  glm::vec3 cameraPosition, cameraFront, cameraUp;

  float cameraYaw, cameraPitch;
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

  SDL_SetWindowRelativeMouseMode(window, true);

  glewInit();

  GameState *state = new GameState();
  *appstate = state;

  state->window = window;
  state->glContext = glContext;
  state->shader = Shader::Load("assets/shaders/basic.vert", "assets/shaders/basic.frag");
  state->chunk = std::make_unique<Chunk>(glm::ivec3{0, 0, 0}, glm::ivec3{64, 64, 64}, 0);

  state->model = glm::identity<glm::mat4>();
  state->cameraPosition = glm::vec3{0.0f, 50.0f, 100.0f};
  state->cameraFront = glm::vec3{0.0f, 0.0f, -1.0f};
  state->cameraUp = glm::vec3{0.0f, 1.0f, 0.0f};
  state->view = glm::lookAt(state->cameraPosition, state->cameraFront, glm::vec3{0.0f, 1.0f, 0.0f});
  state->cameraYaw = -90.0f;
  state->cameraPitch = 0.0f;

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
    case SDL_EVENT_KEY_UP:
    case SDL_EVENT_KEY_DOWN: {
      bool pressed = event->key.down;
      switch (event->key.key) {
        case SDLK_ESCAPE: {
          return SDL_APP_SUCCESS;
        }
        case SDLK_W:
        case SDLK_UP: {
          state->keyboard.pressed[static_cast<int>(Key::Up)] = pressed;
          break;
        }
        case SDLK_S:
        case SDLK_DOWN: {
          state->keyboard.pressed[static_cast<int>(Key::Down)] = pressed;
          break;
        }
        case SDLK_A:
        case SDLK_LEFT: {
          state->keyboard.pressed[static_cast<int>(Key::Left)] = pressed;
          break;
        }
        case SDLK_D:
        case SDLK_RIGHT: {
          state->keyboard.pressed[static_cast<int>(Key::Right)] = pressed;
          break;
        }
        case SDLK_SPACE: {
          state->keyboard.pressed[static_cast<int>(Key::Jump)] = pressed;
          break;
        }
      }

      state->keyboard.pressed[static_cast<int>(Key::Crouch)] = pressed && (event->key.mod & SDL_KMOD_LCTRL);

      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      state->cameraYaw += event->motion.xrel * 0.1f;
      state->cameraPitch -= event->motion.yrel * 0.1f;

      if (state->cameraPitch > 89.0f) {
        state->cameraPitch = 89.0f;
      }
      if (state->cameraPitch < -89.0f) {
        state->cameraPitch = -89.0f;
      }
      break;
    }
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  GameState *state = static_cast<GameState *>(appstate);

  float speed = 0.2f;
  if (state->keyboard.pressed[static_cast<int>(Key::Left)]) {
    auto left = glm::normalize(glm::cross(state->cameraFront, state->cameraUp));
    state->cameraPosition -= speed * left;
  }

  if (state->keyboard.pressed[static_cast<int>(Key::Right)]) {
    auto left = glm::normalize(glm::cross(state->cameraFront, state->cameraUp));
    state->cameraPosition += speed * left;
  }

  if (state->keyboard.pressed[static_cast<int>(Key::Up)]) {
    state->cameraPosition += speed * state->cameraFront;
  }

  if (state->keyboard.pressed[static_cast<int>(Key::Down)]) {
    state->cameraPosition -= speed * state->cameraFront;
  }

  if (state->keyboard.pressed[static_cast<int>(Key::Jump)]) {
    state->cameraPosition += speed * state->cameraUp;
  }

  if (state->keyboard.pressed[static_cast<int>(Key::Crouch)]) {
    state->cameraPosition -= speed * state->cameraUp;
  }

  glm::vec3 direction{};
  direction.x = cos(glm::radians(state->cameraYaw)) * cos(glm::radians(state->cameraPitch));
  direction.y = sin(glm::radians(state->cameraPitch));
  direction.z = sin(glm::radians(state->cameraYaw)) * cos(glm::radians(state->cameraPitch));
  state->cameraFront = glm::normalize(direction);

  state->view = glm::lookAt(state->cameraPosition, state->cameraPosition + state->cameraFront, state->cameraUp);

  glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  state->shader->Bind();
  state->shader->UniformMat4("projection", state->projection);
  state->shader->UniformMat4("view", state->view);
  state->shader->UniformMat4("model", state->model);
  state->chunk->Render();
  state->shader->Unbind();

  SDL_GL_SwapWindow(state->window);
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  if (appstate) {
    GameState *state = static_cast<GameState *>(appstate);
    delete state;
  }
}
