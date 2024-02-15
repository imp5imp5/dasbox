#include "input.h"
#include "sound.h"
#include "logger.h"
#include "fileSystem.h"
#include "localStorage.h"
#include "buildDate.h"
#include <daScript/daScript.h>
#include <daScript/ast/ast.h>
#include <daScript/simulate/interop.h>
#include <daScript/simulate/simulate_visit_op.h>
#include <daScript/simulate/aot_builtin_fio.h>
#include <SFML/System.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Clipboard.hpp>
#include <unordered_map>
#include <string>
#include <fstream>
#include <time.h>
#define _USE_MATH_DEFINES
#include <math.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif


using namespace das;
using namespace std;

static unordered_map<int, const char *> code_to_key_name;
static unordered_map<string, int> key_name_to_code;

bool das_get_key(int key_code) { return input::get_key(key_code - KEY_CODE_OFFSET); }
bool das_get_key_down(int key_code) { return input::get_key_down(key_code - KEY_CODE_OFFSET); }
bool das_get_key_up(int key_code) { return input::get_key_up(key_code - KEY_CODE_OFFSET); }
bool das_get_key_press(int key_code) { return input::get_key_press(key_code - KEY_CODE_OFFSET); }
bool das_get_mouse_button(int button_code) { return input::get_mouse_button(button_code - MOUSE_CODE_OFFSET); }
bool das_get_mouse_button_up(int button_code) { return input::get_mouse_button_up(button_code - MOUSE_CODE_OFFSET); }
bool das_get_mouse_button_down(int button_code) { return input::get_mouse_button_down(button_code - MOUSE_CODE_OFFSET); }
float das_get_axis(int axis_code) { return input::get_axis(axis_code); }

int das_get_pressed_key_index()
{
  int i = input::get_pressed_key_index();
  if (i < 0)
    return 0;
  else
    return i + KEY_CODE_OFFSET;
}


void set_window_title(const char * title);
void set_antialiasing(int a);
void set_resolution(int width, int height);
void set_rendering_upscale(int upscale);
void disable_auto_upscale();

void schedule_pause();
void schedule_quit_game();
void reset_time_after_start();
float get_time_after_start();
float get_delta_time();
bool is_window_active();
void set_vsync_enabled(bool enalbe);
void set_mouse_cursor_visible(bool visible);
void set_mouse_cursor_grabbed(bool grabbed);
void dasbox_execute(const char * file_name);


const char * das_get_key_name(int key_code)
{
  auto it = code_to_key_name.find(key_code);
  if (it == code_to_key_name.end())
    return "";
  else
    return it->second;
}

int das_get_key_code(const char * key_name)
{
  auto it = key_name_to_code.find(string(key_name));
  if (it == key_name_to_code.end())
    return -1;
  else
    return it->second;
}


float normalize_angle(float a)
{
  a = fmodf(a, 2 * M_PI);
  return a > M_PI ? a - 2 * M_PI : (a < -M_PI ? a + 2 * M_PI : a);
}

float angle_diff(float source, float target)
{
  return normalize_angle(target - source);
}

float angle_move_to(float from, float to, float dt, float vel)
{
  float d = vel * dt;
  float angleD = angle_diff(from, to);
  if (fabsf(angleD) < d)
    return to;

  if (angleD < 0)
    return from - d;
  else
    return from + d;
}

float move_to(float from, float to, float dt, float vel)
{
  float d = vel * dt;
  if (fabsf(from - to) < d)
    return to;

  if (to < from)
    return from - d;
  else
    return from + d;
}

float approach(float from, float to, float dt, float viscosity)
{
  if (viscosity < 1e-9f)
    return to;
  else
    return from + (1.0f - expf(-dt / viscosity)) * (to - from);
}

float angle_approach(float from, float to, float dt, float viscosity)
{
  if (viscosity < 1e-9f)
    return to;
  else
    return from + (1.0f - expf(-dt / viscosity)) * angle_diff(from, to);
}

template <typename T> inline T approach_vec(T from, T to, float dt, float viscosity)
{
  if (viscosity < 1e-9f)
    return to;
  else
  {
    float d = 1.0f - expf(-dt / viscosity);
    return v_madd(v_splat4(&d), v_sub(to, from), from);
  }
}

template <typename T> T cvt(float v, float i0, float i1, T o0, T o1)
{
  if (i0 < i1)
    return ::lerp(o0, o1, ::clamp((v - i0) / max(i1 - i0, 1e-6f), 0.0f, 1.0f));
  else
    return ::lerp(o1, o0, ::clamp((v - i1) / max(i0 - i1, 1e-6f), 0.0f, 1.0f));
}

template <typename T> inline T lerp_vec(T a, T b, float t)
{
  return v_madd(v_splat4(&t), v_sub(b, a), a);
}

template <typename T> T cvt_vec(float v, float i0, float i1, T o0, T o1)
{
  if (i0 < i1)
    return lerp_vec(o0, o1, ::clamp((v - i0) / max(i1 - i0, 1e-6f), 0.0f, 1.0f));
  else
    return lerp_vec(o1, o0, ::clamp((v - i1) / max(i0 - i1, 1e-6f), 0.0f, 1.0f));
}


void set_clipboard_text(const char * text)
{
  if (input::get_key_press(sf::Keyboard::C) &&
    (input::get_key(sf::Keyboard::LControl) || input::get_key(sf::Keyboard::RControl)))
  {
    sf::Clipboard::setString(text);
  }
  else
    print_error("set_clipboard_text can be called only if Ctrl+C is pressed");
}

const char * get_clipboard_text()
{
/*  if (input::get_key_press(sf::Keyboard::V) &&
    (input::get_key(sf::Keyboard::LControl) || input::get_key(sf::Keyboard::RControl)))
  {
  ...
  }
  else */
    print_error("get_clipboard_text is not implemented");
  return das_str_dup("");
}

const char * get_dasbox_exe_path();

void patch_vscode_project(const char * dir)
{
  string projFileName = fs::combine_path(dir, ".vscode/settings.json");
  if (!fs::is_file_exists(projFileName.c_str()))
    return;

  string s = fs::read_whole_file(projFileName.c_str());

  const char * n = strstr(s.c_str(), "/dasbox.exe\",");
  if (n)
  {
    const char * nEnd = strchr(n, '"');
    const char * nStart = n;
    while (nStart >= s.c_str() && *nStart != '"')
      nStart--;

    nStart++;

    if (!strncmp(nStart, get_dasbox_exe_path(), nEnd - nStart))
      return;

    s.replace(nStart - s.c_str(), nEnd - nStart, get_dasbox_exe_path());
    fs::write_string_to_file(projFileName.c_str(), s.c_str());
  }
}

void dasbox_execute_editor(const char * dir, const char * file_name)
{
  if (!trust_mode)
    return;
  if (!dir)
    dir = "";
  if (!file_name)
    return;

#if _WIN32
  int res = system("where code.cmd");
  if (!res)
  {
    patch_vscode_project(dir);
    char buf[512] = { 0 };
    snprintf(buf, sizeof(buf), "code \"%s\"", dir, file_name);
    system(buf);
  }
#endif
}

const char * get_dasbox_exe_path()
{
  static string path;
  if (!path.empty())
    return path.c_str();
#if defined(PLATFORM_POSIX) || defined(__linux__)
  std::ifstream("/proc/self/comm") >> path;
  return path.c_str();
#elif defined(__APPLE__)
  char buf[512] = { 0 };
  uint32_t size = sizeof(buf);
  _NSGetExecutablePath(buf, &size);
  path = string(buf);
  return path.c_str();
#elif defined(_WIN32)
  char buf[512] = { 0 };
  GetModuleFileNameA(nullptr, buf, sizeof(buf));
  char * p = buf;
  while (*p)
  {
    if (*p == '\\')
      *p = '/';
    p++;
  }
  path = string(buf);
  return path.c_str();
#else
  static_assert(false, "unrecognized platform");
#endif
}

static das::LogLevel log_verbosity = LogLevel::warning;

void dasbox_log(int level, const char * message)
{
  if (level < log_verbosity && level < LogLevel::critical)
   return;

  if (!message)
    message = "";

  if (level >= LogLevel::critical)
    print_exception("%s", message);
  else if (level >= LogLevel::error)
    print_error("%s", message);
  else if (level >= LogLevel::warning)
    print_warning("%s", message);
  else if (level >= LogLevel::info)
    print_note("%s", message);
  else
    print_text("%s", message);

}

const char * builtin_find_main_das_file_in_directory(const char * path, Context * context, LineInfoArg * at)
{
  string fn = fs::find_main_das_file_in_directory(path);
  if (fn.empty())
    return nullptr;
  char * fname = context->stringHeap->allocateString(fn.c_str(), uint32_t(fn.size()));
  return fname;
}

const char * get_dasbox_version()
{
  return DASBOX_VERSION;
}

const char * get_dasbox_build_date()
{
  return DASBOX_BUILD_DATE;
}

const char * get_dasbox_initial_dir()
{
  return das_str_dup(initial_dir);
}

void randomize_seed(int4 & seed)
{
  static int x = 123;
  x *= 123;
  int s = time(nullptr) + x;
  seed = int4(s, s + 3, s + 12, s + 119);
  x += seed.w;
}

void schedule_screenshot(const char * file_name)
{
  scheduled_screenshot_file_name = file_name;
}


class ModuleDasbox : public Module
{
public:
  ModuleDasbox() : Module("dasbox")
  {
    ModuleLibrary lib;
    lib.addModule(this);
    lib.addBuiltInModule();

#define DECL_KEYS() \
    DECL_KEY_CODE(VK_ESCAPE, Escape) \
    DECL_KEY_CODE(VK_RETURN, Enter) \
    DECL_KEY_CODE(VK_SPACE, Space) \
    DECL_KEY_CODE(VK_LEFT, Left) \
    DECL_KEY_CODE(VK_RIGHT, Right) \
    DECL_KEY_CODE(VK_UP, Up) \
    DECL_KEY_CODE(VK_DOWN, Down) \
    DECL_KEY_CODE(VK_LSHIFT, LShift) \
    DECL_KEY_CODE(VK_RSHIFT, RShift) \
    DECL_KEY_CODE(VK_LCONTROL, LControl) \
    DECL_KEY_CODE(VK_RCONTROL, RControl) \
    DECL_KEY_CODE(VK_LALT, LAlt) \
    DECL_KEY_CODE(VK_RALT, RAlt) \
    DECL_KEY_CODE(VK_LSYSTEM, LSystem) \
    DECL_KEY_CODE(VK_RSYSTEM, RSystem) \
    DECL_KEY_CODE(VK_MENU, Menu) \
    DECL_KEY_CODE(VK_TAB, Tab) \
    DECL_KEY_CODE(VK_HOME, Home) \
    DECL_KEY_CODE(VK_PRIOR, PageUp) \
    DECL_KEY_CODE(VK_END, End) \
    DECL_KEY_CODE(VK_NEXT, PageDown) \
    DECL_KEY_CODE(VK_INSERT, Insert) \
    DECL_KEY_CODE(VK_DELETE, Delete) \
    DECL_KEY_CODE(VK_1, Num1) \
    DECL_KEY_CODE(VK_2, Num2) \
    DECL_KEY_CODE(VK_3, Num3) \
    DECL_KEY_CODE(VK_4, Num4) \
    DECL_KEY_CODE(VK_5, Num5) \
    DECL_KEY_CODE(VK_6, Num6) \
    DECL_KEY_CODE(VK_7, Num7) \
    DECL_KEY_CODE(VK_8, Num8) \
    DECL_KEY_CODE(VK_9, Num9) \
    DECL_KEY_CODE(VK_0, Num0) \
    DECL_KEY_CODE(VK_TILDE, Tilde) \
    DECL_KEY_CODE(VK_MINUS, Hyphen) \
    DECL_KEY_CODE(VK_EQUALS, Equal) \
    DECL_KEY_CODE(VK_BACK, BackSpace) \
    DECL_KEY_CODE(VK_LBRACKET, LBracket) \
    DECL_KEY_CODE(VK_RBRACKET, RBracket) \
    DECL_KEY_CODE(VK_SEMICOLON, Semicolon) \
    DECL_KEY_CODE(VK_APOSTROPHE, Quote) \
    DECL_KEY_CODE(VK_BACKSLASH, Backslash) \
    DECL_KEY_CODE(VK_COMMA, Comma) \
    DECL_KEY_CODE(VK_PERIOD, Period) \
    DECL_KEY_CODE(VK_SLASH, Slash) \
    DECL_KEY_CODE(VK_DIVIDE, Divide) \
    DECL_KEY_CODE(VK_MULTIPLY, Multiply) \
    DECL_KEY_CODE(VK_SUBTRACT, Subtract) \
    DECL_KEY_CODE(VK_ADD, Add) \
    DECL_KEY_CODE(VK_NUMPAD0, Numpad0) \
    DECL_KEY_CODE(VK_NUMPAD1, Numpad1) \
    DECL_KEY_CODE(VK_NUMPAD2, Numpad2) \
    DECL_KEY_CODE(VK_NUMPAD3, Numpad3) \
    DECL_KEY_CODE(VK_NUMPAD4, Numpad4) \
    DECL_KEY_CODE(VK_NUMPAD5, Numpad5) \
    DECL_KEY_CODE(VK_NUMPAD6, Numpad6) \
    DECL_KEY_CODE(VK_NUMPAD7, Numpad7) \
    DECL_KEY_CODE(VK_NUMPAD8, Numpad8) \
    DECL_KEY_CODE(VK_NUMPAD9, Numpad9) \
    DECL_KEY_CODE(VK_Q, Q) \
    DECL_KEY_CODE(VK_W, W) \
    DECL_KEY_CODE(VK_E, E) \
    DECL_KEY_CODE(VK_R, R) \
    DECL_KEY_CODE(VK_T, T) \
    DECL_KEY_CODE(VK_Y, Y) \
    DECL_KEY_CODE(VK_U, U) \
    DECL_KEY_CODE(VK_I, I) \
    DECL_KEY_CODE(VK_O, O) \
    DECL_KEY_CODE(VK_P, P) \
    DECL_KEY_CODE(VK_A, A) \
    DECL_KEY_CODE(VK_S, S) \
    DECL_KEY_CODE(VK_D, D) \
    DECL_KEY_CODE(VK_F, F) \
    DECL_KEY_CODE(VK_G, G) \
    DECL_KEY_CODE(VK_H, H) \
    DECL_KEY_CODE(VK_J, J) \
    DECL_KEY_CODE(VK_K, K) \
    DECL_KEY_CODE(VK_L, L) \
    DECL_KEY_CODE(VK_Z, Z) \
    DECL_KEY_CODE(VK_X, X) \
    DECL_KEY_CODE(VK_C, C) \
    DECL_KEY_CODE(VK_V, V) \
    DECL_KEY_CODE(VK_B, B) \
    DECL_KEY_CODE(VK_N, N) \
    DECL_KEY_CODE(VK_M, M) \
    DECL_KEY_CODE(VK_F1, F1) \
    DECL_KEY_CODE(VK_F2, F2) \
    DECL_KEY_CODE(VK_F3, F3) \
    DECL_KEY_CODE(VK_F4, F4) \
    DECL_KEY_CODE(VK_F5, F5) \
    DECL_KEY_CODE(VK_F6, F6) \
    DECL_KEY_CODE(VK_F7, F7) \
    DECL_KEY_CODE(VK_F8, F8) \
    DECL_KEY_CODE(VK_F9, F9) \
    DECL_KEY_CODE(VK_F10, F10) \
    DECL_KEY_CODE(VK_F11, F11) \
    DECL_KEY_CODE(VK_F12, F12) \
    DECL_KEY_CODE(VK_PAUSE, Pause) \

#define DECL_KEY_CODE(k, m) addConstant(*this, #k, int(sf::Keyboard::m) + KEY_CODE_OFFSET);
    DECL_KEYS();
#undef DECL_KEY_CODE

#define DECL_KEY_CODE(k, m) code_to_key_name[int(sf::Keyboard::m) + KEY_CODE_OFFSET] = #k;
    DECL_KEYS();
#undef DECL_KEY_CODE

#define DECL_KEY_CODE(k, m) key_name_to_code[string(#k)] = int(sf::Keyboard::m) + KEY_CODE_OFFSET;
    DECL_KEYS();
#undef DECL_KEY_CODE


#define DECL_BUTTONS() \
    DECL_BUTTON("GP_A", 0 + KEY_CODE_OFFSET + 256) \
    DECL_BUTTON("GP_B", 1 + KEY_CODE_OFFSET + 256) \
    DECL_BUTTON("GP_X", 2 + KEY_CODE_OFFSET + 256) \
    DECL_BUTTON("GP_Y", 3 + KEY_CODE_OFFSET + 256) \
    DECL_BUTTON("GP_LEFT_SHOULDER", 4 + KEY_CODE_OFFSET + 256) \
    DECL_BUTTON("GP_RIGHT_SHOULDER", 5 + KEY_CODE_OFFSET + 256) \
    DECL_BUTTON("GP_BACK", 6 + KEY_CODE_OFFSET + 256) \
    DECL_BUTTON("GP_START", 7 + KEY_CODE_OFFSET + 256) \
    DECL_BUTTON("GP_LEFT_STICK", 8 + KEY_CODE_OFFSET + 256) \
    DECL_BUTTON("GP_RIGHT_STICK", 9 + KEY_CODE_OFFSET + 256) \


#define DECL_BUTTON(k, m) addConstant(*this, k, m);
    DECL_BUTTONS()
#undef DECL_BUTTON

#define DECL_BUTTON(k, m) code_to_key_name[m] = k;
      DECL_BUTTONS()
#undef DECL_BUTTON

#define DECL_BUTTON(k, m) key_name_to_code[string(k)] = m;
      DECL_BUTTONS()
#undef DECL_BUTTON

    addConstant(*this, "AXIS_PRIMARY_X", AXIS_CODE_OFFSET + 0);
    addConstant(*this, "AXIS_PRIMARY_Y", AXIS_CODE_OFFSET + 1);
    addConstant(*this, "AXIS_TRIGGERS", AXIS_CODE_OFFSET + 2);
    addConstant(*this, "AXIS_TRIGGERS_2", AXIS_CODE_OFFSET + 3);
    addConstant(*this, "AXIS_SECONDARY_X", AXIS_CODE_OFFSET + 4);
    addConstant(*this, "AXIS_SECONDARY_Y", AXIS_CODE_OFFSET + 5);
    addConstant(*this, "AXIS_POV_X", AXIS_CODE_OFFSET + 6);
    addConstant(*this, "AXIS_POV_Y", AXIS_CODE_OFFSET + 7);


    addConstant(*this, "MB_LEFT", MOUSE_CODE_OFFSET + 0);
    addConstant(*this, "MB_RIGHT", MOUSE_CODE_OFFSET + 1);
    addConstant(*this, "MB_MIDDLE", MOUSE_CODE_OFFSET + 2);


    addExtern<DAS_BIND_FUN(das_get_key)>(*this, lib, "get_key", SideEffects::accessExternal, "das_get_key")
      ->args({"key_code"});

    addExtern<DAS_BIND_FUN(das_get_key_down)>(*this, lib, "get_key_down", SideEffects::accessExternal, "das_get_key_down")
      ->args({"key_code"});

    addExtern<DAS_BIND_FUN(das_get_key_up)>(*this, lib, "get_key_up", SideEffects::accessExternal, "das_get_key_up")
      ->args({"key_code"});

    addExtern<DAS_BIND_FUN(input::fetch_entered_symbol)>
      (*this, lib, "fetch_entered_symbol", SideEffects::accessExternal, "input::fetch_entered_symbol");

    addExtern<DAS_BIND_FUN(das_get_pressed_key_index)>
      (*this, lib, "get_pressed_key_index", SideEffects::accessExternal, "das_get_pressed_key_index");

    addExtern<DAS_BIND_FUN(das_get_key_name)>
      (*this, lib, "get_key_name", SideEffects::accessExternal, "das_get_key_name")
      ->args({"key_code"});

    addExtern<DAS_BIND_FUN(das_get_key_code)>
      (*this, lib, "get_key_code", SideEffects::accessExternal, "das_get_key_code")
      ->args({"key_name"});

    addExtern<DAS_BIND_FUN(das_get_key_press)>
      (*this, lib, "get_key_press", SideEffects::accessExternal, "das_get_key_press")
      ->args({"key_code"});

    addExtern<DAS_BIND_FUN(das_get_mouse_button)>
      (*this, lib, "get_mouse_button", SideEffects::accessExternal, "das_get_mouse_button")
      ->args({"button_code"});

    addExtern<DAS_BIND_FUN(das_get_mouse_button_up)>
      (*this, lib, "get_mouse_button_up", SideEffects::accessExternal, "das_get_mouse_button_up")
      ->args({"button_code"});

    addExtern<DAS_BIND_FUN(das_get_mouse_button_down)>
      (*this, lib, "get_mouse_button_down", SideEffects::accessExternal, "das_get_mouse_button_down")
      ->args({"button_code"});

    addExtern<DAS_BIND_FUN(input::get_mouse_scroll_delta)>
      (*this, lib, "get_mouse_scroll_delta", SideEffects::accessExternal, "input::get_mouse_scroll_delta");
    addExtern<DAS_BIND_FUN(input::get_mouse_position)>
      (*this, lib, "get_mouse_position", SideEffects::accessExternal, "input::get_mouse_position");
    addExtern<DAS_BIND_FUN(input::get_mouse_position_delta)>
      (*this, lib, "get_mouse_position_delta", SideEffects::accessExternal, "input::get_mouse_position_delta");
    addExtern<DAS_BIND_FUN(input::get_mouse_velocity)>
      (*this, lib, "get_mouse_velocity", SideEffects::accessExternal, "input::get_mouse_velocity");
    addExtern<DAS_BIND_FUN(input::set_relative_mouse_mode)>
      (*this, lib, "set_relative_mouse_mode", SideEffects::modifyExternal, "input::set_relative_mouse_mode")
      ->args({"relative"});
    addExtern<DAS_BIND_FUN(input::is_relative_mouse_mode)>
      (*this, lib, "is_relative_mouse_mode", SideEffects::accessExternal, "input::is_relative_mouse_mode");

    addExtern<DAS_BIND_FUN(das_get_axis)>
      (*this, lib, "get_axis", SideEffects::accessExternal, "das_get_axis")
      ->args({"axis_code"});

    addExtern<DAS_BIND_FUN(set_clipboard_text)>
      (*this, lib, "set_clipboard_text", SideEffects::accessExternal, "set_clipboard_text")
      ->args({"text"});

    addExtern<DAS_BIND_FUN(sqr<float>)>
      (*this, lib, "sqr", SideEffects::accessExternal, "sqr")
      ->args({"x"});

    addExtern<DAS_BIND_FUN(sqr<double>)>
      (*this, lib, "sqr", SideEffects::accessExternal, "sqr")
      ->args({"x"});

    addExtern<DAS_BIND_FUN(sqr<int>)>
      (*this, lib, "sqr", SideEffects::accessExternal, "sqr")
      ->args({"x"});

    addExtern<DAS_BIND_FUN(normalize_angle)>
      (*this, lib, "normalize_angle", SideEffects::accessExternal, "normalize_angle")
      ->args({"angle"});

    addExtern<DAS_BIND_FUN(angle_diff)>
      (*this, lib, "angle_diff", SideEffects::accessExternal, "angle_diff")
      ->args({"source", "target"});

    addExtern<DAS_BIND_FUN(cvt<float>)>
      (*this, lib, "cvt", SideEffects::accessExternal, "cvt")
      ->args({"value", "from_range_1", "from_range_2", "to_range_1", "to_range_2"});

    addExtern<DAS_BIND_FUN(cvt_vec<das::float2>)>
      (*this, lib, "cvt", SideEffects::accessExternal, "cvt_vec<das::float2>")
      ->args({"value", "from_range_1", "from_range_2", "to_range_1", "to_range_2"});

    addExtern<DAS_BIND_FUN(cvt_vec<das::float3>)>
      (*this, lib, "cvt", SideEffects::accessExternal, "cvt_vec<das::float3>")
      ->args({"value", "from_range_1", "from_range_2", "to_range_1", "to_range_2"});

    addExtern<DAS_BIND_FUN(cvt_vec<das::float4>)>
      (*this, lib, "cvt", SideEffects::accessExternal, "cvt_vec<das::float4>")
      ->args({"value", "from_range_1", "from_range_2", "to_range_1", "to_range_2"});

    addExtern<DAS_BIND_FUN(move_to)>
      (*this, lib, "move_to", SideEffects::accessExternal, "move_to")
      ->args({"from", "to", "dt", "vel"});

    addExtern<DAS_BIND_FUN(angle_move_to)>
      (*this, lib, "angle_move_to", SideEffects::accessExternal, "angle_move_to")
      ->args({"from", "to", "dt", "vel"});

    addExtern<DAS_BIND_FUN(approach)>
      (*this, lib, "approach", SideEffects::accessExternal, "approach")
      ->args({"from", "to", "dt", "viscosity"});

    addExtern<DAS_BIND_FUN(angle_approach)>
      (*this, lib, "angle_approach", SideEffects::accessExternal, "angle_approach")
      ->args({"from", "to", "dt", "viscosity"});

    addExtern<DAS_BIND_FUN(approach_vec<das::float2>)>
      (*this, lib, "approach", SideEffects::accessExternal, "approach_vec<das::float2>")
      ->args({"from", "to", "dt", "viscosity"});

    addExtern<DAS_BIND_FUN(approach_vec<das::float3>)>
      (*this, lib, "approach", SideEffects::accessExternal, "approach_vec<das::float3>")
      ->args({"from", "to", "dt", "viscosity"});

    addExtern<DAS_BIND_FUN(approach_vec<das::float4>)>
      (*this, lib, "approach", SideEffects::accessExternal, "approach_vec<das::float4>")
      ->args({"from", "to", "dt", "viscosity"});


    addExtern<DAS_BIND_FUN(set_window_title)>
      (*this, lib, "set_window_title", SideEffects::modifyExternal, "set_window_title")
      ->args({"title"});

    addExtern<DAS_BIND_FUN(set_antialiasing)>
      (*this, lib, "set_antialiasing", SideEffects::modifyExternal, "set_antialiasing")
      ->args({"antialiasing_level"});

    addExtern<DAS_BIND_FUN(set_resolution)>
      (*this, lib, "set_resolution", SideEffects::modifyExternal, "set_resolution")
      ->args({"width", "height"});

    addExtern<DAS_BIND_FUN(set_rendering_upscale)>
      (*this, lib, "set_rendering_upscale", SideEffects::modifyExternal, "set_rendering_upscale")
      ->args({"upscale"});

    addExtern<DAS_BIND_FUN(disable_auto_upscale)>
      (*this, lib, "disable_auto_upscale", SideEffects::modifyExternal, "disable_auto_upscale");

    addExtern<DAS_BIND_FUN(fs::local_storage_set)>
      (*this, lib, "local_storage_set", SideEffects::modifyExternal, "fs::local_storage_set")
      ->args({"key", "value"});

    addExtern<DAS_BIND_FUN(fs::local_storage_get)>
      (*this, lib, "local_storage_get", SideEffects::modifyExternal, "fs::local_storage_get")
      ->args({"key"});

    addExtern<DAS_BIND_FUN(fs::local_storage_has_key)>
      (*this, lib, "local_storage_has_key", SideEffects::modifyExternal, "fs::local_storage_has_key")
      ->args({"key"});

    addExtern<DAS_BIND_FUN(schedule_pause)>
      (*this, lib, "schedule_pause", SideEffects::modifyExternal, "schedule_pause");
    addExtern<DAS_BIND_FUN(schedule_quit_game)>
      (*this, lib, "schedule_quit_game", SideEffects::modifyExternal, "schedule_quit_game");
    addExtern<DAS_BIND_FUN(get_time_after_start)>
      (*this, lib, "get_time_after_start", SideEffects::modifyExternal, "get_time_after_start");
    addExtern<DAS_BIND_FUN(reset_time_after_start)>
      (*this, lib, "reset_time_after_start", SideEffects::modifyExternal, "reset_time_after_start");
    addExtern<DAS_BIND_FUN(get_delta_time)>
      (*this, lib, "get_delta_time", SideEffects::modifyExternal, "get_delta_time");
    addExtern<DAS_BIND_FUN(is_window_active)>
      (*this, lib, "is_window_active", SideEffects::modifyExternal, "is_window_active");
    addExtern<DAS_BIND_FUN(set_vsync_enabled)>
      (*this, lib, "set_vsync_enabled", SideEffects::modifyExternal, "set_vsync_enabled")
      ->args({"vsync"});

    addExtern<DAS_BIND_FUN(set_mouse_cursor_visible)>
      (*this, lib, "set_mouse_cursor_visible", SideEffects::modifyExternal, "set_mouse_cursor_visible")
      ->args({"visible"});

    addExtern<DAS_BIND_FUN(set_mouse_cursor_grabbed)>
      (*this, lib, "set_mouse_cursor_grabbed", SideEffects::modifyExternal, "set_mouse_cursor_grabbed")
      ->args({"grabbed"});

    addExtern<DAS_BIND_FUN(fs::is_file_exists)>(*this, lib, "is_file_exists",
      SideEffects::modifyExternal, "fs::is_file_exists")
      ->arg("file_name");

    addExtern<DAS_BIND_FUN(dasbox_execute)>(*this, lib, "dasbox_execute",
      SideEffects::modifyExternal, "dasbox_execute")
      ->arg("file_name");

    addExtern<DAS_BIND_FUN(dasbox_log)>
      (*this, lib, "dasbox_log", SideEffects::modifyExternal, "dasbox_log");

    addExtern<DAS_BIND_FUN(get_dasbox_version)>
      (*this, lib, "get_dasbox_version", SideEffects::accessExternal, "get_dasbox_version");

    addExtern<DAS_BIND_FUN(get_dasbox_build_date)>
      (*this, lib, "get_dasbox_build_date", SideEffects::accessExternal, "get_dasbox_build_date");

    addExtern<DAS_BIND_FUN(get_dasbox_initial_dir)>
      (*this, lib, "get_dasbox_initial_dir", SideEffects::accessExternal, "get_dasbox_initial_dir");

    addExtern<DAS_BIND_FUN(get_dasbox_exe_path)>
      (*this, lib, "get_dasbox_exe_path", SideEffects::accessExternal, "get_dasbox_exe_path");

    addExtern<DAS_BIND_FUN(dasbox_execute_editor)>
      (*this, lib, "dasbox_execute_editor", SideEffects::modifyExternal, "dasbox_execute_editor");

    addExtern<DAS_BIND_FUN(builtin_find_main_das_file_in_directory)>
      (*this, lib, "dasbox_find_main_das_file_in_directory", SideEffects::modifyExternal, "builtin_find_main_das_file_in_directory");

    addExtern<DAS_BIND_FUN(randomize_seed)>
      (*this, lib, "randomize_seed", SideEffects::modifyArgument, "randomize_seed");

    addExtern<DAS_BIND_FUN(input::hide_cursor)>
      (*this, lib, "hide_mouse_cursor", SideEffects::modifyExternal, "input::hide_cursor");

    addExtern<DAS_BIND_FUN(input::show_cursor)>
      (*this, lib, "show_mouse_cursor", SideEffects::modifyExternal, "input::show_cursor");

    addExtern<DAS_BIND_FUN(schedule_screenshot)>
      (*this, lib, "schedule_screenshot", SideEffects::modifyExternal, "schedule_screenshot")
      ->args({"file_name"});

    // its AOT ready
    //verifyAotReady();
  }

  virtual ModuleAotType aotRequire(TextWriter & tw) const override
  {
    //tw << "#include \"test_profile.h\"\n";
    return ModuleAotType::cpp;
  }
};

REGISTER_MODULE(ModuleDasbox);
