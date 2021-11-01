#include "profiler.h"

#include "globals.h"
#include "graphics.h"
#include "sound.h"

using namespace std;

Profiler profiler;

int get_string_heap_memory_usage();
int get_heap_memory_usage();

Profiler::Profiler()
{
  reset();
}

void Profiler::reset()
{
  frames = 0;
  actTime.clear();
  drawTime.clear();
  textureUpdate.clear();
}

void Profiler::update()
{
  frames++;
}

double Profiler::getAverage(const vector<double> & a)
{
  if (a.size() == 0)
    return 0.0;
  double res = 0.0;
  for (double t : a)
    res += t;
  return res / a.size();
}

double Profiler::getSum(const vector<double> & a)
{
  double res = 0.0;
  for (double t : a)
    res += t;
  return res;
}

void Profiler::add(vector<double> & a, double t)
{
  if (a.size() < collectFrames)
    a.push_back(t);
  a[frames % collectFrames] = t;
}

void Profiler::print()
{
  print_text("\n"
    "_____________________________________________________\n"
    "Profiler log, based on last %d frame(s)\n"
    "\n"
    "act time:   %0.2f msec\n"
    "draw time:  %0.2f msec\n"
    "\n"
    "textures updated: %d (%0.2f per frame)\n"
    "\n"
    "total images: %d\n"
    "total sounds: %d\n"
    "\n"
    "heap:        %0.6f MB\n"
    "string heap: %0.6f MB\n"
    "\n"
    ,
    int(actTime.size()),
    getAverage(actTime) / 1000.0,
    getAverage(drawTime) / 1000.0,
    int(getSum(textureUpdate) + 0.5), getAverage(textureUpdate),
    graphics::get_image_count(),
    sound::get_sound_count(),
    get_heap_memory_usage() / 1.0e6,
    get_string_heap_memory_usage() / 1.0e6
  );

  print_text("New resources for this period:");
  graphics::print_debug_infos(current_frame - collectFrames);
  print_text("\n\n");
}
