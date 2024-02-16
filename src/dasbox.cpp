#include <daScript/daScript.h>
#include <daScript/simulate/interop.h>
#include <daScript/simulate/simulate_visit_op.h>
#include <daScript/das_project_specific.h>
#include <daScript/misc/performance_time.h>
#include <daScript/misc/job_que.h>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/BlendMode.hpp>
#include <SFML/System.hpp>
#include <algorithm>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <setjmp.h>
#include "input.h"
#include "graphics.h"
#include "logger.h"
#include "globals.h"
#include "fileSystem.h"
#include "localStorage.h"
#include "sound.h"
#include "profiler.h"

#ifdef _WIN32
#include <../SFML/extlibs/headers/glad/include/glad/gl.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#else // POSIX
#include <signal.h>
#endif

namespace das
{
  extern mutex g_jobQueMutex;
  extern shared_ptr<JobQue> g_jobQue;
};


using namespace std;
using namespace das;

string root_dir = "";
string main_das_file_name = "";
string return_to_file_name = "";
string return_to_root = "";
string scheduled_screenshot_file_name = "";
int exit_code_on_error = 1;
bool exit_on_error = false;
bool return_to_trust_mode = false;
const char * initial_dir = ".";
bool has_fatal_errors = false;
bool recreate_window = false;
bool inside_initialization = false;
bool inside_draw_fn = true;
bool exec_script_scheduled = false;
bool trust_mode = false;
bool run_for_plugin = false;
bool log_to_console = false;
bool use_debug_trap = false;
bool use_garbage_collector = true;
string plugin_main_function = "main";

const char * exception_pos = "unknown";
locale * g_locale;
static jmp_buf eval_buf;
static bool use_separate_render_target = false;
static bool vsync_enabled = false;
static bool disable_auto_upscale_arg = false;

int screen_global_scale = 0;
int screen_width = 1280;
int screen_height = 720;

sf::Vector2i window_pos = sf::Vector2i(0, 0);

void set_font_name(const char *);
void set_font_size_i(int);


int current_frame = 0;

bool is_quit_scheduled = false;
bool window_is_active = true;
bool is_first_frame = true;
bool is_in_debug_mode = false;
bool program_has_unsafe_operations = false;
float cur_dt = 0.001f;
double time_after_start = 0.0;

sf::RenderTarget * g_render_target = nullptr;
sf::RenderWindow * g_window = nullptr;
sf::RenderTexture * render_texture = nullptr;
sf::Sprite * render_texture_sprite = nullptr;

pair<bool, sf::String> delayed_window_title = make_pair(false, sf::String("dasbox"));
pair<bool, bool> delayed_cursor_visible = make_pair(true, true);
pair<bool, bool> delayed_cursor_grabbed = make_pair(true, false);
pair<bool, bool> delayed_vsync = make_pair(true, true);
pair<bool, sf::Vector2i> delayed_resolution = make_pair(false, sf::Vector2i(1280, 720));
pair<bool, int> delayed_upscale = make_pair(false, 0);
pair<bool, int> delayed_window_antialiasing = make_pair(false, 0);

static bool delayed_window_antialiasing_set = false;
static bool delayed_resolution_set = false;
static bool delayed_upscale_set = false;


void schedule_pause()
{
  screen_mode = SM_LOG;
  on_switch_to_log_screen();
}

void schedule_quit_game()
{
  is_quit_scheduled = true;
}

void reset_time_after_start()
{
  time_after_start = 0.0;
}

float get_time_after_start()
{
  return float(time_after_start);
}

float get_delta_time()
{
  return cur_dt;
}

bool is_window_active()
{
  return window_is_active;
}

void update_window_title()
{
  if (!g_window)
    return;
  sf::String t;
  if (is_in_debug_mode)
    t += "[DEBUG]  ";
  if (program_has_unsafe_operations)
    t += "[UNSAFE]  ";
  t += delayed_window_title.second;
  g_window->setTitle(t);
}

void set_window_title(const char * title)
{
  if (!title)
    title = "";
  if (g_window && !inside_initialization)
  {
    delayed_window_title = make_pair(false, sf::String(title));
    update_window_title();
  }
  else
    delayed_window_title = make_pair(true, sf::String(title));
}

void set_antialiasing(int a)
{
  if (a < 0)
    a = 0;
  if (a > 8)
    a = 8;

  delayed_window_antialiasing_set = (a != 0);

  if (a != delayed_window_antialiasing.second)
  {
    delayed_window_antialiasing = make_pair(true, a);
    recreate_window = true;
  }
}

void set_resolution(int width, int height)
{
  delayed_resolution_set = (width != 1280 || height != 720);

  screen_width = width;
  screen_height = height;

  sf::Vector2i res = sf::Vector2i(width, height);
  if (res != delayed_resolution.second)
  {
    delayed_resolution = make_pair(true, res);
    recreate_window = true;
  }
}

void set_rendering_upscale(int upscale)
{
  delayed_upscale_set = (upscale != screen_global_scale);

  if (upscale != delayed_upscale.second)
  {
    delayed_upscale = make_pair(true, upscale);
    recreate_window = true;
  }
}

void disable_auto_upscale()
{
  set_rendering_upscale(1);
  delayed_upscale_set = true;
}

void set_mouse_cursor_visible(bool visible)
{
  if (g_window && !inside_initialization)
  {
    fetch_cerr();
    reinterpret_error_as_note(true);
    g_window->setMouseCursorVisible(visible);
    fetch_cerr();
    reinterpret_error_as_note(false);
    delayed_cursor_visible.first = false;
  }
  else
    delayed_cursor_visible = make_pair(true, visible);
}

void set_mouse_cursor_grabbed(bool grabbed)
{
  if (g_window && !inside_initialization)
  {
    fetch_cerr();
    reinterpret_error_as_note(true);
    g_window->setMouseCursorGrabbed(grabbed);
    fetch_cerr();
    reinterpret_error_as_note(false);
    delayed_cursor_grabbed.first = false;
  }
  else
    delayed_cursor_grabbed = make_pair(true, grabbed);
}

void set_vsync_enabled(bool enable)
{
  if (g_window && !inside_initialization)
  {
    fetch_cerr();
    reinterpret_error_as_note(true);
    g_window->setVerticalSyncEnabled(enable);
    vsync_enabled = enable;
    fetch_cerr();
    reinterpret_error_as_note(false);
    delayed_vsync.first = false;
  }
  else
    delayed_vsync = make_pair(true, enable);
}

void prepare_delayed_variables()
{
  set_window_title("dasbox");
  delayed_window_antialiasing_set = false;
  delayed_resolution_set = false;
  delayed_upscale_set = false;
}

void check_delayed_variables()
{
  if (!delayed_window_antialiasing_set)
    set_antialiasing(0);

  if (!delayed_resolution_set)
    set_resolution(1280, 720);

  if (!delayed_upscale_set)
  {
    set_rendering_upscale(0);
    delayed_upscale.first = true;
  }
}
//-------------------------------------------------------------------------------------


struct PlaygroundContext final : das::Context
{
  PlaygroundContext(uint32_t stackSize) : das::Context(stackSize) {};
  PlaygroundContext(Context & ctx, uint32_t category) : das::Context(ctx, category) {};

  void to_out(const char * message)
  {
    dasbox_logger << message;
  }

  void to_err(const char * message)
  {
    dasbox_logger.setTopErrorLine();
    dasbox_logger.setState(LOGGER_ERROR);
    dasbox_logger << message << "\n";
    dasbox_logger.setState(LOGGER_NORMAL);
  }
};

struct DasFile
{
  das::smart_ptr<PlaygroundContext> ctx;
  ModuleGroup dummyLibGroup;
  das::smart_ptr<fs::DasboxFsFileAccess> fAccess;
  ProgramPtr program;

  DasFile() : fAccess(make_smart<fs::DasboxFsFileAccess>()) { }
};


smart_ptr<das::FileAccess> dasbox_get_file_access(char * pak) {
  if ( pak ) {
    return make_smart<fs::DasboxFsFileAccess>(pak, make_smart<fs::DasboxFsFileAccess>());
  } else {
    return make_smart<fs::DasboxFsFileAccess>();
  }
}


Context * dasbox_get_new_context(int stackSize) {
  return new PlaygroundContext(stackSize);
}


Context * dasbox_get_clone_context(Context * ctx, uint32_t category) {
  return new PlaygroundContext(*ctx, category);
}


static DasFile * das_file = new DasFile();
static DasFile * das_live_file = new DasFile();
static float time_to_check = 1.0f;

SimFunction * fn_before_gc = nullptr;
SimFunction * fn_after_gc = nullptr;
SimFunction * fn_act = nullptr;
SimFunction * fn_draw = nullptr;
SimFunction * fn_live_set_new_context = nullptr;

int get_string_heap_memory_usage()
{
  return int(das_file->ctx->stringHeap->bytesAllocated());
}

int get_heap_memory_usage()
{
  return int(das_file->ctx->heap->bytesAllocated());
}

static void find_live_function(SimFunction ** fn, const char * fn_name)
{
  if (!das_live_file->ctx.get())
    return;

  *fn = das_live_file->ctx->findFunction(fn_name);
  if (!*fn)
    print_error("Function '%s' not found (live)", fn_name);
}

static void set_new_live_context(Context * ctx, bool full_reload)
{
  if (!das_live_file->ctx.get())
    return;

  das_live_file->ctx->runWithCatch([&](){
      das_invoke_function<void>::invoke<smart_ptr<Context>, bool>(das_live_file->ctx.get(), nullptr, fn_live_set_new_context, ctx, full_reload);
    });

  if (const char * exception_text = das_live_file->ctx->getException())
  {
    string s;
    s += exception_text;
    s += "\n";
    s += das_live_file->ctx->getStackWalk(nullptr, true, true);
    print_exception("%s", s.c_str());
  }
}

static void find_function(SimFunction ** fn, const char * fn_name, bool required, bool has_float_arg)
{
  if (!das_file->ctx.get())
    return;

  SimFunction * res = nullptr;
  vector<SimFunction *> functions = das_file->ctx->findFunctions(fn_name);
  for (auto f : functions)
  {
    if (has_float_arg)
    {
      if (verifyCall<void, float>(f->debugInfo, das_file->dummyLibGroup))
      {
        if (res)
        {
          print_error("Too many functions '%s(float)' found. Expected only one.", fn_name);
          break;
        }
        res = f;
      }
    }
    else
    {
      if (verifyCall<void>(f->debugInfo, das_file->dummyLibGroup))
      {
        if (res)
        {
          print_error("Too many functions '%s()' found. Expected only one.", fn_name);
          break;
        }
        res = f;
      }
    }
  }

  *fn = res;

  if (!*fn)
  {
    if (required)
      print_error("Function '%s' not found", fn_name);
  }
}

void exec_function(SimFunction * fn, vec4f * args)
{
  if (!das_file->ctx || !fn || has_fatal_errors)
    return;

  EXCEPTION_POS(fn->mangledName);

  if (setjmp(eval_buf) != 1)
  {
    das_file->ctx->eval(fn, args);
    if (const char * exception_text = das_file->ctx->getException())
    {
      string s;
      s += exception_text;
      s += "\n";
      s += das_file->ctx->getStackWalk(nullptr, true, true);
      print_exception("%s", s.c_str());
      has_fatal_errors = true;
    }
  }
  else
  {
    print_error("Failed to execute function '%s'", fn->name);
    has_fatal_errors = true;
  }

  EXCEPTION_POS("unknown");
}


const char * das_str_dup(const char * s)
{
  if (!s)
    return s;

  if (!das_file->ctx.get() || !das_file->ctx)
    return strdup(s); // memory leak is better than crash

  int len = strlen(s);
  char * res = das_file->ctx->stringHeap->allocate(len + 1);
  memcpy(res, s, len + 1);
  return res;
}


DasFile * load_module(const string & file_name, DasFile ** das_file, bool hard_reload)
{
  DasFile * oldFile = *das_file;
  *das_file = new DasFile;

  if (hard_reload)
  {
    delete oldFile;
    oldFile = nullptr;
  }

  fn_act = nullptr;
  fn_draw = nullptr;
  fn_before_gc = nullptr;
  fn_after_gc = nullptr;

  if (!fs::is_file_exists(file_name.c_str()))
  {
    print_error("File not found: '%s'", file_name.c_str());
    delete oldFile;
    return nullptr;
  }

  print_note("Executing file '%s'", file_name.c_str());
  EXCEPTION_POS("compile");


  CodeOfPolicies policies;
  policies.ignore_shared_modules = hard_reload;
  policies.threadlock_context = true;

  (*das_file)->program = compileDaScript(file_name, (*das_file)->fAccess, dasbox_logger, (*das_file)->dummyLibGroup, policies);
  ProgramPtr program = (*das_file)->program;
  if (program->failed())
  {
    string s;
    s += "Failed to compile: '";
    s += file_name;
    s += "'\n";

    for (Error & e : program->errors)
    {
      s += "\n";
      s += reportError(e.at, e.what, e.extra, e.fixme, e.cerr);
      s += "\n";
    }
    print_error("%s\n", s.c_str());
    delete oldFile;
    return nullptr;
  }

  program->policies.persistent_heap = true; // must be set after compileDaScript()
  if (!program->options.find("gc", Type::tBool))
    program->options.emplace_back("gc", true);


  EXCEPTION_POS("simulate");

  if (setjmp(eval_buf) != 1)
  {
    // create daScript context
    (*das_file)->ctx = make_smart<PlaygroundContext>(program->getContextStackSize());
    if (!program->simulate(*(*das_file)->ctx, dasbox_logger))
    {
      has_fatal_errors = true;
      string s;
      s += "Failed to simulate '";
      s += file_name;
      s += "'\n";

      for (Error & e : program->errors)
      {
        s += "\n";
        s += reportError(e.at, e.what, e.extra, e.fixme, e.cerr);
        s += "\n";
      }

      s += (*das_file)->ctx->getStackWalk(nullptr, true, true);

      print_error("%s\n", s.c_str());
      delete oldFile;
      return nullptr;
    }
  }
  else
  {
    print_exception("Failed to simulate");
    has_fatal_errors = true;
    delete oldFile;
    return nullptr;
  }

  EXCEPTION_POS("");

  return oldFile;
}


void find_dasbox_api_functions(bool hard_reload)
{
  find_function(&fn_act, "act", true, true);
  find_function(&fn_draw, "draw", true, false);
  find_function(&fn_before_gc, "before_gc", false, false);
  find_function(&fn_after_gc, "after_gc", false, false);
}


void find_dasbox_live_api_fnctions()
{
  find_live_function(&fn_live_set_new_context, "set_new_context");
}

void set_application_screen();

void initialize_das_file(bool hard_reload)
{
  if (das_file && das_file->ctx.get())
  {
    inside_initialization = true;
    if (input::is_relative_mouse_mode())
      input::set_relative_mouse_mode(false);
    prepare_delayed_variables();
    if (!has_fatal_errors)
      set_new_live_context(das_file->ctx.get(), hard_reload);
    find_dasbox_api_functions(hard_reload);
    inside_initialization = false;
    check_delayed_variables();

    if (is_in_debug_mode || program_has_unsafe_operations)
      update_window_title();
  }
}

void das_file_manual_reload(bool hard_reload)
{
  if (has_fatal_errors)
  {
    hard_reload = true;
    if (das_file->ctx.get())
      das_file->ctx->shutdown = true;
    delete das_file;
    das_file = nullptr;
  }

  sound::stop_all_sounds();
  builtin_sleep(50);
  graphics::delete_allocated_images();
  graphics::reset_transform();
  sound::delete_allocated_sounds();
  set_font_name(nullptr);
  set_font_size_i(16);
  profiler.reset();

  is_in_debug_mode = false;
  program_has_unsafe_operations = false;

  set_application_screen();
  dasbox_logger.clear();
  input::reset_input();
  reset_time_after_start();
  is_first_frame = true;
  fs::flush_local_storage();

  has_fatal_errors = false;

  fs::initialize_local_storage(main_das_file_name.c_str());
  DasFile * oldFile = load_module(main_das_file_name, &das_file, hard_reload);
  initialize_das_file(hard_reload);
  delete oldFile;
}

void das_file_reload_update(float dt)
{
  bool reload = false;
  bool hardReload = false;
  bool ctrl = input::get_key(sf::Keyboard::LControl) || input::get_key(sf::Keyboard::RControl);
  bool alt = input::get_key(sf::Keyboard::LAlt) || input::get_key(sf::Keyboard::RAlt);


  if (input::get_key_down(sf::Keyboard::F5) || (ctrl && input::get_key_down(sf::Keyboard::R)))
    reload = true;

  if ((ctrl && input::get_key_down(sf::Keyboard::F5)) || (ctrl && alt && input::get_key_down(sf::Keyboard::R)))
  {
    reload = true;
    hardReload = true;
  }

  if (reload)
  {
    if (!main_das_file_name.empty())
    {
      das_file_manual_reload(hardReload);
      time_to_check += 0.1f;
      return;
    }
  }

  time_to_check -= dt;
  if (time_to_check < 0)
  {
    time_to_check = is_window_active() ? 999999.0f : 0.4f;
    reload = false;
    for (auto f : das_file->fAccess->filesOpened)
      if (f.second != fs::get_file_time(f.first.c_str()))
      {
        reload = true;
        break;
      }

    if (reload)
      das_file_manual_reload(false);
  }
}


//-------------------------------- log ------------------------------------------------

ScreenMode screen_mode = SM_USER_APPLICATION;

void set_application_screen()
{
  dasbox_logger.topErrorLine = -1;
  screen_mode = SM_USER_APPLICATION;
}

void update_switch_screens()
{
  if ((input::get_key_down(sf::Keyboard::Tab) && !input::get_key(sf::Keyboard::LAlt) &&
      !input::get_key(sf::Keyboard::RAlt))
      /*|| (input::get_key_down(sf::Keyboard::Escape) && screen_mode == SM_LOG*)*/
     )
  {
    if (screen_mode == SM_LOG && !has_fatal_errors)
    {
      on_return_from_log_screen();
      set_application_screen();
    }
    else if (screen_mode == SM_USER_APPLICATION)
    {
      screen_mode = SM_LOG;
      on_switch_to_log_screen();
    }
  }
}



//-------------------------------------------------------------------------------------

void process_args(int argc, char **argv)
{
  for (int i = 1; i < argc; i++)
  {
    string arg = argv[i];
    if (!arg.empty())
    {
      if (arg[0] != '-')
      {
        const char * dot = strrchr(arg.c_str(), '.');
        if (dot && !strcmp(dot, ".das"))
        {
          if (main_das_file_name.empty())
          {
            main_das_file_name = fs::extract_file_name(arg);
            root_dir = fs::extract_dir(arg);
          }
          else
            print_error("Main .das file is already set ('%s')\n", main_das_file_name.c_str());
        }
        else
        {
          if (arg.back() == '\\' || arg.back() == '/')
            arg.pop_back();

          if (!arg.empty())
          {
            // temorary "trust mode"
            bool t = trust_mode;
            trust_mode = true;
            string fn = fs::find_main_das_file_in_directory(arg.c_str());
            trust_mode = t;

            if (!fn.empty())
            {
              main_das_file_name = fs::extract_file_name(fn);
              root_dir = arg;
            }
          }
        }
      }

      if (!strncmp(arg.c_str(), "--exit-on-error:", sizeof("--exit-on-error:") - 1))
      {
        exit_on_error = true;
        exit_code_on_error = atoi(arg.c_str() + sizeof("--exit-on-error:") - 1);
      }

      if (arg == "--disable-gc")
        use_garbage_collector = false;

      if (arg == "--trust")
        trust_mode = true;

      if (arg == "--use-debug-trap")
        use_debug_trap = true;

      if (arg == "--disable-auto-upscale")
        disable_auto_upscale_arg = true;

      if ((arg == "-main" || arg == "--main") && i < argc - 1)
      {
        plugin_main_function = argv[i + 1];
        run_for_plugin = true;
        trust_mode = true;
        i++;
      }

      if (arg == "--dasbox-console" || arg == "--")
        log_to_console = true;

      if (arg == "--")
        break;
    }
  }
}


void set_application_icon()
{
#if _WIN32
  HICON hIcon = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(1), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
  unsigned res = SetClassLongPtr((HWND)g_window->getSystemHandle(), GCLP_HICON, (LONG_PTR)hIcon);
  G_UNUSED(res);
#endif
}


void create_window()
{
  fetch_cerr();
  input::reset_input();

  sf::Vector2i pos(INT_MAX, INT_MAX);

  if (fs::local_storage_has_key("dasbox_preferences/window_pos/x"))
  {
    pos.x = atoi(fs::local_storage_get("dasbox_preferences/window_pos/x"));
    pos.y = atoi(fs::local_storage_get("dasbox_preferences/window_pos/y"));
  }

  if (g_window)
  {
    pos = g_window->getPosition();
    g_window->close();
    delete g_window;
    g_window = nullptr;
  }

  delete render_texture;
  render_texture = nullptr;
  delete render_texture_sprite;
  render_texture_sprite = nullptr;


  sf::ContextSettings windowSettings;
  windowSettings.majorVersion = 2;
  windowSettings.minorVersion = 1;

  windowSettings.antialiasingLevel = 0;

  sf::Vector2i resolution = delayed_resolution.second;


  sf::Vector2i window_size = resolution;
  sf::String windowTitle = delayed_window_title.second;

  if (delayed_upscale.first)
    screen_global_scale = ::clamp(delayed_upscale.second, 0, 16);

  screen_width = resolution.x;
  screen_height = resolution.y;

  if (screen_global_scale <= 0) // auto upscale
  {
    float threshold = get_desktop_dpi() <= 100 ? 2.5f : 2.21f;

    if (resolution.x < get_desktop_width() / threshold && resolution.y < get_desktop_height() / threshold &&
        disable_auto_upscale_arg == false)
    {
      screen_global_scale = 2;
    }
    else
    {
      screen_global_scale = 1;
    }
  }

  use_separate_render_target = (screen_global_scale != 1);

  if (!use_separate_render_target)
    windowSettings.antialiasingLevel = delayed_window_antialiasing.second;

  window_size.x *= screen_global_scale;
  window_size.y *= screen_global_scale;

  sf::VideoMode videoMode(window_size.x, window_size.y);

  fetch_cerr();
  reinterpret_error_as_note(true);
  g_window = new sf::RenderWindow(videoMode, windowTitle, sf::Style::Close, windowSettings);
  set_application_icon();
  fetch_cerr();
  reinterpret_error_as_note(false);

  if (pos.x != INT_MAX)
  {
    if (pos.x + window_size.x + 2 < int(sf::VideoMode::getDesktopMode().width) &&
        pos.y + window_size.y + 2 < int(sf::VideoMode::getDesktopMode().height) &&
        pos.x > -100 && pos.y > -30)
    {
      g_window->setPosition(pos);
    }
  }

  render_texture = new sf::RenderTexture();
  render_texture_sprite = new sf::Sprite();

  if (use_separate_render_target)
  {
    sf::ContextSettings rtSettings = windowSettings;
    rtSettings.antialiasingLevel = delayed_window_antialiasing.second;
    rtSettings.majorVersion = 2;
    rtSettings.minorVersion = 1;

    render_texture->create(resolution.x, resolution.y, rtSettings);
    render_texture->setSmooth(false);
    render_texture->setRepeated(false);
    g_render_target = render_texture;
    render_texture_sprite->setTexture(render_texture->getTexture());
    render_texture_sprite->setScale(float(screen_global_scale), float(screen_global_scale));
  }
  else
  {
    g_render_target = g_window;
  }

  g_window->setVerticalSyncEnabled(true);

  if (input::is_relative_mouse_mode())
    input::set_relative_mouse_mode(true);

  if (input::is_cursor_hidden())
    input::hide_cursor();

  delayed_window_antialiasing.first = false;
  delayed_upscale.first = false;
  delayed_resolution.first = false;

  recreate_window = false;
}


void return_to_previous_script()
{
  is_quit_scheduled = false;
  root_dir = return_to_root;
  main_das_file_name = return_to_file_name;
  if (!fs::change_dir(root_dir))
  {
    print_error("Cannot change directory to '%s'\n", root_dir.c_str());
    return;
  }
  return_to_file_name.clear();
  return_to_root.clear();
  trust_mode = return_to_trust_mode;
  return_to_trust_mode = false;
  das_file_manual_reload(true);
}


static string scheduled_script_name = "";

void exec_script_impl()
{
  exec_script_scheduled = false;

  if (!fs::is_file_exists(scheduled_script_name.c_str()))
  {
    print_error("Cannot open file '%s'. File does not exist.", scheduled_script_name.c_str());
    return;
  }

  if (!fs::is_path_string_valid(scheduled_script_name.c_str()))
  {
    print_error("Cannot open file '%s'. Absolute paths or access to the parent directory is prohibited.",
      scheduled_script_name.c_str());
    return;
  }

  return_to_root = fs::get_current_dir();
  return_to_file_name = main_das_file_name;
  return_to_trust_mode = trust_mode;
  trust_mode = false;


  root_dir = fs::extract_dir(scheduled_script_name.c_str());
  if (!fs::change_dir(root_dir))
  {
    print_error("Cannot change directory to '%s'\n", root_dir.c_str());
    return;
  }

  main_das_file_name = fs::extract_file_name(scheduled_script_name.c_str());
  das_file_manual_reload(true);
}

void dasbox_execute(const char * file_name)
{
  if (!trust_mode)
    return;
  scheduled_script_name = std::string(file_name);
  exec_script_scheduled = true;
}


void fetch_cerr()
{
  string s = dasbox_logger.cerrStream.str();
  if (!s.empty())
  {
    print_error("%s", s.c_str());
    stringstream clearStream;
    dasbox_logger.cerrStream.swap(clearStream);
  }
}


void check_window_pos_changed()
{
  if (!g_window || !g_window->isOpen())
    return;

  if (window_pos != g_window->getPosition() && g_window->getSize().x > 0)
  {
    window_pos = g_window->getPosition();
    char buf[16] = { 0 };
    snprintf(buf, 16, "%d", window_pos.x);
    fs::local_storage_set("dasbox_preferences/window_pos/x", buf);
    snprintf(buf, 16, "%d", window_pos.y);
    fs::local_storage_set("dasbox_preferences/window_pos/y", buf);
  }
}


void run_das_for_plugin(const string & file_name, const string & main_func_name)
{
  auto access = make_smart<fs::DasboxFsFileAccess>();
  ModuleGroup dummyGroup;
  if (auto program = compileDaScript(file_name, access, dasbox_logger, dummyGroup))
  {
    if (program->failed())
    {
      for (auto & err : program->errors)
        dasbox_logger << reportError(err.at, err.what, err.extra, err.fixme, err.cerr);
    }
    else
    {
      Context ctx(program->getContextStackSize());
      if (!program->simulate(ctx, dasbox_logger))
      {
        dasbox_logger << "Failed to simulate\n";
        for (auto & err : program->errors)
          dasbox_logger << reportError(err.at, err.what, err.extra, err.fixme, err.cerr );
      }
      else
      {
        if (auto fnTest = ctx.findFunction(main_func_name.c_str()))
        {
          if (verifyCall<void>(fnTest->debugInfo, dummyGroup) || verifyCall<bool>(fnTest->debugInfo, dummyGroup))
          {
            ctx.restart();
            ctx.eval(fnTest, nullptr);
          }
          else
          {
            dasbox_logger << "function '"  << main_func_name << "' call arguments do not match, expecting 'main(): void' or 'main(): bool'\n";
          }
        }
        else
        {
          dasbox_logger << "function '"  << main_func_name << "' not found\n";
        }
      }
    }
  }
}


void take_window_screenshot(const string & file_name)
{
  if (!g_window)
    return;

  if (!fs::is_path_string_valid(file_name.c_str()))
  {
    print_error("Cannot write to '%s'. Absolute paths or access to the parent directory is prohibited.", file_name.c_str());
    return;
  }

  sf::Texture texture;
  texture.create(g_window->getSize().x, g_window->getSize().y);
  texture.update(*g_window);

  texture.copyToImage().saveToFile(file_name);
}


void win32_check_for_active_hack()
{
#ifdef _WIN32
  if (!g_window || !g_window->getSystemHandle())
    return;

  if (GetActiveWindow() == g_window->getSystemHandle() && GetForegroundWindow() == g_window->getSystemHandle())
    window_is_active = true;
#endif
}


void run_das_for_ui()
{
  if (!main_das_file_name.empty())
  {
    fs::initialize_local_storage(main_das_file_name.c_str());
    is_first_frame = true;
    DasFile * oldFile = load_module(main_das_file_name, &das_file, true);
    initialize_das_file(true);
    delete oldFile;
  }

  /////////////////////////////////////////////////////////
  create_window();

  sf::Clock deltaClock;

  while (g_window->isOpen())
  {
    fetch_cerr();
    sf::Event event;
    while (g_window->pollEvent(event))
    {
      switch (event.type)
      {
      case sf::Event::Closed:
        g_window->close();
        break;

      case sf::Event::GainedFocus:
        window_is_active = true;
        break;

      case sf::Event::LostFocus:
        window_is_active = false;
        input::release_input();
        time_to_check = 0.4f;
        break;

      case sf::Event::MouseWheelScrolled:
        if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel)
          input::gmc_mouse_wheel(event.mouseWheelScroll.delta);
        break;

      case sf::Event::MouseButtonPressed:
        input::gmc_mouse_button_down(event.mouseButton.button);
        break;

      case sf::Event::MouseButtonReleased:
        input::gmc_mouse_button_up(event.mouseButton.button);
        break;

      case sf::Event::KeyPressed:
        input::gkc_button_down(event.key.code);
        break;

      case sf::Event::KeyReleased:
        input::gkc_button_up(event.key.code);
        break;

      case sf::Event::TextEntered:
        if (screen_mode == SM_USER_APPLICATION)
          input::gkc_symbol_entered(event.text.unicode);
        break;

      case sf::Event::JoystickButtonPressed:
        if (event.joystickButton.joystickId == 0)
          input::joy_button_down(event.joystickButton.button);
        break;

      case sf::Event::JoystickButtonReleased:
        if (event.joystickButton.joystickId == 0)
          input::joy_button_up(event.joystickButton.button);
        break;

      case sf::Event::JoystickMoved:
        if (event.joystickMove.joystickId == 0)
        {
          if (event.joystickMove.axis == sf::Joystick::Axis::PovX)
            event.joystickMove.position = sign(event.joystickMove.position) * (fabs(event.joystickMove.position) < 50.0f ? 0.0f : 100.0f);
          if (event.joystickMove.axis == sf::Joystick::Axis::PovY)
            event.joystickMove.position = sign(event.joystickMove.position) * (fabs(event.joystickMove.position) < 50.0f ? 0.0f : -100.0f);

          input::joy_axis_position(event.joystickMove.axis, event.joystickMove.position);
        }
        break;

      default: break;
      }
    }

    if (!window_is_active)
      win32_check_for_active_hack();

    float dt = deltaClock.restart().asSeconds();
    if (is_first_frame)
    {
      is_first_frame = false;
      dt = std::min(dt, 0.01f);
    }

    if (screen_mode == SM_USER_APPLICATION)
    {
      time_after_start += double(dt);
      current_frame++;
    }

    dt = std::min(dt, 0.1f);

    if (vsync_enabled)
    {
      float dtRate = dt / std::max(cur_dt, 0.0001f);
      if (dtRate > 0.8f && dtRate < 1.25f)
        dt = lerp(cur_dt, dt, 0.125f);
      float dtHighLimit = std::max(cur_dt, 0.01f) * 2.0f;
      if (dt > dtHighLimit)
        dt = dtHighLimit;
    }

    cur_dt = dt;

    input::update_mouse_input(dt, window_is_active);

    das_file_reload_update(dt);
    update_switch_screens();
    if (screen_mode == SM_LOG)
      update_log_screen(dt);

    if (!window_is_active)
      builtin_sleep(8);
    else
      builtin_sleep(0);

    if (screen_mode == SM_USER_APPLICATION)
    {
      uint64_t t0 = ref_time_ticks();
      vec4f arg = v_make_vec4f(dt, 0, 0, 0);
      exec_function(fn_act, &arg);
      fetch_cerr();
      profiler.add(profiler.actTime, double(get_time_usec(t0)));
    }

    if (dasbox_logger.topErrorLine >= 0)
    {
      screen_mode = SM_LOG;
      on_switch_to_log_screen();
    }


    if (delayed_window_title.first)
      set_window_title(delayed_window_title.second.toAnsiString().c_str());

    if (delayed_cursor_grabbed.first)
      set_mouse_cursor_grabbed(delayed_cursor_grabbed.second);

    if (delayed_cursor_visible.first)
      set_mouse_cursor_visible(delayed_cursor_visible.second);

    if (delayed_vsync.first)
      set_vsync_enabled(delayed_vsync.second);


    uint64_t drawT0 = ref_time_ticks();

    // render
    {
      inside_draw_fn = true;

      graphics::on_graphics_frame_start();

      if (screen_mode == SM_USER_APPLICATION)
        exec_function(fn_draw, nullptr);
      else
        draw_log_screen();

      inside_draw_fn = false;

      if (use_separate_render_target)
      {
        render_texture->display();
        g_window->draw(*render_texture_sprite, sf::BlendNone);
      }

      if (screen_mode == SM_USER_APPLICATION)
        if (!scheduled_screenshot_file_name.empty())
        {
          take_window_screenshot(scheduled_screenshot_file_name);
          scheduled_screenshot_file_name.clear();
        }
    }

    if (screen_mode == SM_USER_APPLICATION)
    {
      profiler.add(profiler.drawTime, double(get_time_usec(drawT0)));
      profiler.add(profiler.textureUpdate, double(graphics::get_updated_textures_count()));
      profiler.add(profiler.renderPrimitives, double(graphics::get_render_primitives_count()));
      profiler.add(profiler.playingSounds, double(sound::get_playing_sound_count()));
    }

#ifdef _WIN32
    // Workaround unstable vsync on Windows 10
 //   if (vsync_enabled)
//      glFinish();
#endif
    g_window->display();


    if (screen_mode == SM_USER_APPLICATION)
    {
      profiler.update();

      bool altPressed = (input::get_key(sf::Keyboard::LAlt) || input::get_key(sf::Keyboard::RAlt));
      bool ctrlPressed = (input::get_key(sf::Keyboard::LControl) || input::get_key(sf::Keyboard::LControl));

      if ((input::get_key_down(sf::Keyboard::F1) && altPressed) ||
          (input::get_key_down(sf::Keyboard::P) && altPressed && ctrlPressed))
      {
        dasbox_logger.setTopErrorLine();
        profiler.print();
        screen_mode = SM_LOG;
        on_switch_to_log_screen();
      }
    }


    input::post_update_input();

    fs::update_local_storage(dt);

    if (is_quit_scheduled && !return_to_file_name.empty())
    {
      g_window->clear();
      g_window->display();
      return_to_previous_script();
    }

    if (exec_script_scheduled)
      exec_script_impl();

    if (recreate_window)
    {
      print_note("Window recreated");
      create_window();
    }

    check_window_pos_changed();

    if (das_file && das_file->ctx && !has_fatal_errors && use_garbage_collector)
    {
      exec_function(fn_before_gc, nullptr);
      EXCEPTION_POS("ctx->collectHeap");
      das_file->ctx->collectHeap(nullptr, true, false);
      exec_function(fn_after_gc, nullptr);
    }

    if (is_quit_scheduled)
      g_window->close();
  }


  fs::flush_local_storage();
  sound::finalize();
  graphics::finalize();

  delete render_texture;
  delete render_texture_sprite;
  delete g_window;
}


void hide_console()
{
#ifdef _WIN32
  DWORD consolePid = -1;
  GetWindowThreadProcessId(::GetConsoleWindow(), &consolePid);
  DWORD curPid = GetCurrentProcessId();

  if (consolePid == curPid)
    ShowWindow(::GetConsoleWindow(), SW_HIDE);
#endif
}


void run_das_for_ui_with_try_except()
{
#ifdef _WIN32
  __try
  {
    run_das_for_ui();
  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {
    if (exit_code_on_error != 1)
    {
      printf("\nNATIVE EXCEPTION code = 0x%X, at = \"%s\"\n", exception_code(), exception_pos);
      exit(exit_code_on_error);
    }

    if (!log_to_console)
      dasbox_logger.printAllLog();
    ShowWindow(::GetConsoleWindow(), SW_SHOW);
    SetForegroundWindow(::GetConsoleWindow());
    printf("\n\nNATIVE EXCEPTION code = 0x%X, at = \"%s\"\nPress any key to exit.\n", exception_code(), exception_pos);
    system("@pause > nul");
    exit(exit_code_on_error);
  }
#else
  log_to_console = true;
  run_das_for_ui();
#endif
}


int main(int argc, char **argv)
{
  sf::err().rdbuf(dasbox_logger.cerrStream.rdbuf());

  process_args(argc, argv);
  if (!log_to_console && !run_for_plugin)
    hide_console();

  locale utf8Locale("en_US.UTF-8");
  g_locale = &utf8Locale;


  string initialDirString = fs::get_current_dir();
  initial_dir = initialDirString.c_str();

  das::set_project_specific_fs_callbacks(dasbox_get_file_access);
  das::set_project_specific_ctx_callbacks(dasbox_get_new_context, dasbox_get_clone_context);

  setCommandLineArguments(argc, argv);

  if (root_dir.empty() && main_das_file_name.empty())
  {
    const char * default_menu_script_path = nullptr;
    if (fs::is_file_exists("samples/dasbox_initial_menu.das"))
      default_menu_script_path = "samples/dasbox_initial_menu.das";
    if (fs::is_file_exists("../samples/dasbox_initial_menu.das"))
      default_menu_script_path = "../samples/dasbox_initial_menu.das";

    if (default_menu_script_path)
    {
      main_das_file_name = "dasbox_initial_menu.das";
      root_dir = fs::extract_dir(default_menu_script_path);
      trust_mode = true;
    }
  }

  fs::initialize();
  graphics::initialize();
  sound::initialize();

  if (!run_for_plugin)
  {
    if (!fs::change_dir(root_dir))
    {
      print_error("Cannot change directory to '%s'\n", root_dir.c_str());
      return 1;
    }

    if (!fs::is_file_exists(main_das_file_name.c_str()))
    {
      if (main_das_file_name.empty())
        print_error("File name is not specified\n\nUsage:\n  dasbox.exe <your_application.das>", main_das_file_name.c_str());
      else
        print_error("File does not exists '%s'\n", main_das_file_name.c_str());
    }
  }
  
  NEED_MODULE(Module_BuiltIn);
  NEED_MODULE(Module_Math);
  NEED_MODULE(Module_Strings);
  NEED_MODULE(Module_Rtti);
  NEED_MODULE(Module_Ast);
  NEED_MODULE(Module_Debugger);
  NEED_MODULE(Module_Network);
  NEED_MODULE(Module_UriParser);
  NEED_MODULE(Module_JobQue);
  NEED_MODULE(Module_FIO);
  NEED_MODULE(ModuleGraphics);
  NEED_MODULE(ModuleDasbox);
  NEED_MODULE(ModuleSound);

  if (!das::g_jobQue)
  {
    lock_guard<mutex> guard(das::g_jobQueMutex);
    das::g_jobQue = make_shared<das::JobQue>();
  }


  if (run_for_plugin && trust_mode)
  {
    run_das_for_plugin(fs::combine_path(root_dir, main_das_file_name), plugin_main_function);
    return 0;
  }
  else
  {
    das_live_file = new DasFile();
    DasFile * oldFile = load_module("daslib/live.das", &das_live_file, false);
    find_dasbox_live_api_fnctions();
    delete oldFile;

    run_das_for_ui_with_try_except();
  }

  return 0;
}
