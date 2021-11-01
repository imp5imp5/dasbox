#pragma once

#include <vector>

struct Profiler
{
  static const int collectFrames = 50;
  int frames;
  std::vector<double> actTime; // usec
  std::vector<double> drawTime; // usec
  std::vector<double> textureUpdate;
  std::vector<double> playingSounds;
  std::vector<double> renderPrimitives;

  Profiler();
  void reset();
  void update();
  double getAverage(const std::vector<double> & a);
  double getSum(const std::vector<double> & a);
  double getMin(const std::vector<double> & a);
  double getMax(const std::vector<double> & a);
  void add(std::vector<double> & a, double t);
  void print();
};

extern Profiler profiler;