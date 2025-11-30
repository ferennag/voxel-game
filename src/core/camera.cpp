#include "camera.h"

#include "glm/ext/matrix_transform.hpp"

Camera::Camera(const glm::vec3 &position) : mPosition(position) {
  mFront = glm::vec3{0.0f, 0.0f, -1.0f};
  mUp = glm::vec3{0.0f, 1.0f, 0.0f};

  mView = glm::lookAt(mPosition, mFront, mUp);
  mYaw = -90.0f;
  mPitch = 0.0f;
}

void Camera::HandleKeyboardEvent(const KeyboardState &keyboard) {
  float speed = 0.2f;
  if (keyboard.pressed[static_cast<int>(Key::Left)]) {
    auto left = glm::normalize(glm::cross(mFront, mUp));
    mPosition -= speed * left;
  }

  if (keyboard.pressed[static_cast<int>(Key::Right)]) {
    auto left = glm::normalize(glm::cross(mFront, mUp));
    mPosition += speed * left;
  }

  if (keyboard.pressed[static_cast<int>(Key::Up)]) {
    mPosition += speed * mFront;
  }

  if (keyboard.pressed[static_cast<int>(Key::Down)]) {
    mPosition -= speed * mFront;
  }

  if (keyboard.pressed[static_cast<int>(Key::Jump)]) {
    mPosition += speed * mUp;
  }

  if (keyboard.pressed[static_cast<int>(Key::Crouch)]) {
    mPosition -= speed * mUp;
  }

  mView = glm::lookAt(mPosition, mPosition + mFront, mUp);
}

void Camera::HandleMouseEvent(const MouseState &mouse) {
  float turnSpeed = 0.1f;
  mYaw += mouse.relX * turnSpeed;
  mPitch -= mouse.relY * turnSpeed;

  if (mPitch > 89.0f) {
    mPitch = 89.0f;
  }
  if (mPitch < -89.0f) {
    mPitch = -89.0f;
  }

  glm::vec3 direction{};
  direction.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
  direction.y = sin(glm::radians(mPitch));
  direction.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
  mFront = glm::normalize(direction);

  mView = glm::lookAt(mPosition, mPosition + mFront, mUp);
}

glm::mat4 Camera::GetView() const {
  return mView;
}
