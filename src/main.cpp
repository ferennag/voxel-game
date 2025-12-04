#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_video.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/trigonometric.hpp"
#include <GL/glew.h>

#define SDL_MAIN_USE_CALLBACKS
#include "glm/ext/matrix_float4x4.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <memory>

#include "core/camera.h"
#include "core/keyboard.h"
#include "core/shader.h"
#include "core/texture.h"
#include "world/world.h"

#ifdef _WIN32
#include <Wincon.h>

extern "C" {
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;       // Optimus: force switch to discrete GPU
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1; // AMD
}
#endif

struct GameState {
  SDL_Window *window;
  SDL_GLContext glContext;
  KeyboardState keyboard;

  std::unique_ptr<Shader> shader;
  std::unique_ptr<Camera> camera;
  glm::vec3 sunPosition;

  glm::mat4 projection, model;
  std::unique_ptr<World> world;
};

void GLAPIENTRY OpenGLOutputCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                     const GLchar *message, const void *userParam) {
  SDL_Log("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

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

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(OpenGLOutputCallback, 0);
  glEnable(GL_MULTISAMPLE);

  GameState *state = new GameState();
  *appstate = state;

  state->window = window;
  state->glContext = glContext;
  state->shader = Shader::Load("assets/shaders/basic.vert", "assets/shaders/basic.frag");
  if (!state->shader) {
    return SDL_APP_FAILURE;
  }

  state->world = std::make_unique<World>(0, glm::ivec3{64, 64, 64});

  state->model = glm::identity<glm::mat4>();
  state->camera = std::make_unique<Camera>(glm::vec3{0.0f, 100.0f, 100.0f});
  state->sunPosition = {20.0f, 200.0f, -20.0f};

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
      state->camera->HandleMouseEvent(glm::vec2(event->motion.xrel, event->motion.yrel));
      break;
    }
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  GameState *state = static_cast<GameState *>(appstate);

  state->camera->HandleKeyboardEvent(state->keyboard);

  glEnable(GL_DEPTH_TEST);
  glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  state->shader->Bind();
  state->shader->UniformMat4("projection", state->projection);
  state->shader->UniformMat4("view", state->camera->GetView());
  state->shader->UniformVec3("sunPosition", state->sunPosition);
  state->shader->UniformVec3("eye", state->camera->GetPosition());
  state->world->Render(*state->shader);
  state->shader->Unbind();

  SDL_GL_SwapWindow(state->window);

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  Texture::Cleanup();
  if (appstate) {
    GameState *state = static_cast<GameState *>(appstate);
    delete state;
  }
}
