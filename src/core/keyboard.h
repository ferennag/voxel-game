#pragma once

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
