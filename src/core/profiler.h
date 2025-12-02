#pragma once

#include <string>

class Profiler {
  bool mStarted;
  unsigned long mStart;
  unsigned long mPrevious;
  unsigned long mEnd;

  static float ToMs(unsigned long time);

public:
  static Profiler Create();

  Profiler();

  void Start();

  void LogSnapshot(const std::string &name);

  void LogEnd(const std::string &name);
};
