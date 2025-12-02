#include "profiler.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_timer.h"

Profiler::Profiler() {
  mStarted = false;
}

Profiler Profiler::Create() {
  Profiler profiler;
  profiler.Start();
  return profiler;
}

float Profiler::ToMs(unsigned long time) {
  return static_cast<float>(time) / SDL_GetPerformanceFrequency() * 1000.0f;
}

void Profiler::Start() {
  mStarted = true;
  mStart = SDL_GetPerformanceCounter();
  mPrevious = mStart;
}

void Profiler::LogSnapshot(const std::string &name) {
  auto current = SDL_GetPerformanceCounter();
  auto diff = current - mPrevious;
  mPrevious = current;

  float milliseconds = ToMs(diff);
  SDL_Log("%s: %.2f ms", name.c_str(), milliseconds);
}

void Profiler::LogEnd(const std::string &name) {
  auto current = SDL_GetPerformanceCounter();
  auto diff = current - mStart;
  mStarted = false;

  float milliseconds = ToMs(diff);
  SDL_Log("%s: %.2f ms", name.c_str(), milliseconds);
}
