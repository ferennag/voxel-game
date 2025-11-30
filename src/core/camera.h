#pragma once

#include "keyboard.h"
#include "mouse.h"
#include <SDL3/SDL.h>
#include <glm/glm.hpp>

class Camera {
public:
  Camera(const glm::vec3 &position);

  void HandleMouseEvent(const MouseState &mouse);
  void HandleKeyboardEvent(const KeyboardState &keyboard);

  glm::mat4 GetView() const;

private:
  glm::vec3 mPosition, mFront, mUp;
  float mYaw, mPitch;
  glm::mat4 mView;
};
