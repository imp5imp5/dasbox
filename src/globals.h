#pragma once

#define DASBOX_VERSION "0.1.67"

#include <string>

namespace sf
{
  class RenderTarget;
  class RenderWindow;
}

enum ScreenMode
{
  SM_USER_APPLICATION = 0,
  SM_LOG
};

extern ScreenMode screen_mode;
extern int screen_global_scale;

extern sf::RenderTarget * g_render_target;
extern sf::RenderWindow * g_window;
extern int screen_width;
extern int screen_height;
extern bool has_fatal_errors;
extern bool trust_mode;
extern bool is_in_debug_mode;
extern bool program_has_unsafe_operations;
extern bool log_to_console;
extern bool inside_draw_fn;
extern const char * initial_dir;
extern int current_frame;
extern bool window_is_active;
extern int exit_code_on_error;
extern bool exit_on_error;
extern const char * exception_pos;
extern std::string scheduled_screenshot_file_name;


void print_error(const char * format, ...);
void print_exception(const char * format, ...);
void print_note(const char * format, ...);
void print_warning(const char * format, ...);
void print_text(const char * format, ...);
void dasbox_log(int level, const char * message);
void reinterpret_error_as_note(bool is_note);
void os_debug_break();
void fetch_cerr();

const char * das_str_dup(const char * s);

#define VERY_BIG_NUMBER 2147440000
#define countof(x) (sizeof(x) / sizeof((x)[0]))
#define G_UNUSED(x) ((void)(x))

#ifndef _MSC_VER
#  define stricmp strcasecmp
#endif

#define EXCEPTION_POS(x) exception_pos=x;

inline float ssmooth(float t, float min = 0.f, float max = 1.f)
{
  return min + t * t * (3 - 2 * t) * (max - min);
}

template <class T> inline T sqr(T x)
{
  return x * x;
}

template <class T> inline T sign(T x)
{
  return (x < 0) ? T(-1) : ((x > 0) ? T(1) : 0);
}

template <typename T> inline T clamp(T t, const T min_val, const T max_val)
{
  if (t < min_val)
    t = min_val;
  if (t <= max_val)
    return t;
  return max_val;
}

template <typename T> inline T lerp(T a, T b, float t)
{
  return (b - a) * t + a;
}

struct DasboxDebugInfo
{
  char name[40];
  int creationFrame;

  DasboxDebugInfo()
  {
    reset();
  }

  void reset()
  {
    name[0] = 0;
    name[sizeof(name) - 1] = 0;
    creationFrame = current_frame;
  }
};
