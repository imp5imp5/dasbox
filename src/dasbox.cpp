#include <daScript/daScript.h>
#include <daScript/daScript.h>
#include <daScript/simulate/interop.h>
#include <daScript/simulate/simulate_visit_op.h>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/BlendMode.hpp>
#include <SFML/System.hpp>
#include <algorithm>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <setjmp.h>
#include "input.h"
#include "graphics.h"
#include "logger.h"
#include "globals.h"
#include "fileSystem.h"
#include "sound.h"

#ifdef _WIN32
#include <../SFML/extlibs/headers/glad/include/glad/gl.h>
#endif

using namespace std;
using namespace das;

string root_dir = "";
string main_das_file_name = "";
string return_to_file_name = "";
string return_to_root = "";
bool has_errors = false;
bool recreate_window = false;
bool inside_initialization = false;
bool exec_script_scheduled = false;

locale * g_locale;
static jmp_buf eval_buf;
static bool use_separate_render_target = false;
static bool vsync_enabled = false;

int screen_global_scale = 0;
int screen_width = 1280;
int screen_height = 720;

void set_font_name(const char *);
void set_font_size_i(int);


//------------------------------- logger ----------------------------------------------

void os_debug_break()
{
//  freopen( stdout);
  print_error("Script break\n");
  longjmp(eval_buf, 1);
}

//-------------------------------------------------------------------------------------
bool is_quit_scheduled = false;
bool window_is_active = true;
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

void set_window_title(const char * title)
{
  if (!title)
    title = "";
  if (g_window && !inside_initialization)
  {
    g_window->setTitle(sf::String::fromUtf8(title, title + strlen(title)));
    delayed_window_title.first = false;
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
  delayed_resolution_set = (width != 1280 && height != 720);

  sf::Vector2i res = sf::Vector2i(width, height);
  if (res != delayed_resolution.second)
  {
    delayed_resolution = make_pair(true, res);
    screen_width = width;
    screen_height = height;
    recreate_window = true;
  }
}

void set_rendering_upscale(int upscale)
{
  delayed_upscale_set = (upscale != 0);

  if (upscale != delayed_upscale.second)
  {
    delayed_upscale = make_pair(true, upscale);
    recreate_window = true;
  }
}

void disable_auto_upscale()
{
  set_rendering_upscale(0);
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
    set_rendering_upscale(0);
}
//-------------------------------------------------------------------------------------

struct PlaygroundContext final : das::Context
{
  PlaygroundContext(uint32_t stackSize) : das::Context(stackSize) {}

  void to_out(const char * message)
  {
    logger << message;
  }

  void to_err(const char * message)
  {
    logger.setTopErrorLine();
    uint32_t savedColor = logger.setLogColor(ERROR_LINE_COLOR);
    logger << message << "\n";
    logger.setLogColor(savedColor);
  }
};

struct DasFile
{
  PlaygroundContext * ctx;
  ModuleGroup dummyLibGroup;
  das::smart_ptr<fs::DasboxFsFileAccess> fAccess;
  ProgramPtr program;

  DasFile()
  {
    ctx = nullptr;
    fAccess = make_smart<fs::DasboxFsFileAccess>();
  }

  ~DasFile()
  {
    delete ctx;
    ctx = nullptr;
  }
};


static DasFile * das_file = new DasFile();
static float time_to_check = 1.0f;

SimFunction * fn_initialize = nullptr;
SimFunction * fn_act = nullptr;
SimFunction * fn_draw = nullptr;
SimFunction * fn_finalize = nullptr;


static void find_function(SimFunction ** fn, const char * fn_name, bool required)
{
  *fn = das_file->ctx->findFunction(fn_name);
  if (!*fn)
  {
    if (required)
      print_error("Function '%s' not found", fn_name);
    else
      print_note("Function '%s' not found", fn_name);
  }
}

void exec_function(SimFunction * fn, vec4f * args)
{
  if (!das_file->ctx || !fn)
    return;

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
    }
  }
}


bool load_module(const string & file_name)
{
  delete das_file;
  das_file = new DasFile;

  fn_initialize = nullptr;
  fn_act = nullptr;
  fn_draw = nullptr;
  fn_finalize = nullptr;

  if (!fs::is_file_exists(file_name.c_str()))
  {
    print_error("File not found: '%s'", file_name.c_str());
    return false;
  }

  print_note("Executing file '%s'", file_name.c_str());

  das_file->program = compileDaScript(file_name, das_file->fAccess, logger, das_file->dummyLibGroup);
  if (das_file->program->failed())
  {
    string s;
    s += "Failed to compile: '";
    s += file_name;
    s += "'\n";

    for (Error & e : das_file->program->errors)
    {
      s += "\n";
      s += reportError(e.at, e.what, e.extra, e.fixme, e.cerr);
      s += "\n";
    }
    print_error("%s\n", s.c_str());
    return false;
  }

  // create daScript context
  das_file->ctx = new PlaygroundContext(das_file->program->getContextStackSize());
  if (!das_file->program->simulate(*das_file->ctx, logger))
  {
    string s;
    s += "Failed to simulate '";
    s += file_name;
    s += "'\n";

    for (Error & e : das_file->program->errors)
    {
      s += "\n";
      s += reportError(e.at, e.what, e.extra, e.fixme, e.cerr);
      s += "\n";
    }

    print_error("%s\n", s.c_str());
    return false;
  }

  find_function(&fn_initialize, "initialize", false);
  find_function(&fn_act, "act", true);
  find_function(&fn_draw, "draw", true);
  find_function(&fn_finalize, "finalize", false);
  //  find_function(&fn_initialize_ecs, "initialize_ecs", false);

  inside_initialization = true;
  prepare_delayed_variables();
  exec_function(fn_initialize, nullptr);
  inside_initialization = false;
  check_delayed_variables();

  return true;
}

void set_application_screen();

void das_file_manual_reload()
{
  sound::stop_all_sounds();
  builtin_sleep(50);
  graphics::delete_allocated_images();
  sound::delete_allocated_sounds();
  set_font_name(nullptr);
  set_font_size_i(16);

  set_application_screen();
  logger.clear();
  input::reset_input();
  reset_time_after_start();
  load_module(main_das_file_name);
}

void das_file_reload_update(float dt)
{
  if (input::get_key_down(sf::Keyboard::F5) ||
    (input::get_key_down(sf::Keyboard::R) &&
    (input::get_key(sf::Keyboard::LControl) || input::get_key(sf::Keyboard::RControl))))
  {
    das_file_manual_reload();
    time_to_check += 0.1f;
    return;
  }

  time_to_check -= dt;
  if (time_to_check < 0)
  {
    time_to_check = is_window_active() ? 999999.0f : 0.4f;
    bool reload = false;
    for (auto f : das_file->fAccess->filesOpened)
      if (f.second != fs::get_file_time(f.first.c_str()))
      {
        reload = true;
        break;
      }

    if (reload)
      das_file_manual_reload();
  }
}


//-------------------------------- log ------------------------------------------------

ScreenMode screen_mode = SM_USER_APPLICATION;

void set_application_screen()
{
  logger.topErrorLine = -1;
  screen_mode = SM_USER_APPLICATION;
}

  void update_switch_screens()
{
  if (input::get_key_down(sf::Keyboard::Tab) && !input::get_key(sf::Keyboard::LAlt) &&
      !input::get_key(sf::Keyboard::RAlt))
  {
    if (screen_mode == SM_LOG)
    {
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
          if (root_dir.empty())
            root_dir = arg;
          else
            print_error("Root directory is already set ('%s')\n", root_dir.c_str());
        }
      }
    }
  }
}

void print_usage()
{
  cout << "\nUsage:\n  dasbox [directory] <start-file.das>\n";
}


void create_window()
{
  input::reset_input();

  sf::Vector2i pos(INT_MAX, INT_MAX);
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
  windowSettings.majorVersion = 3;
  windowSettings.minorVersion = 0;

  windowSettings.antialiasingLevel = 0;

  sf::Vector2i resolution = delayed_resolution.second;


  sf::Vector2i window_size = resolution;
  sf::String windowTitle = delayed_window_title.second;

  if (delayed_upscale.first)
    screen_global_scale = clamp(delayed_upscale.second, 0, 16);

  screen_width = resolution.x;
  screen_height = resolution.y;

  if (screen_global_scale <= 0) // auto upscale
  {
    if (resolution.x < sf::VideoMode::getDesktopMode().width / 2.5f && resolution.y < sf::VideoMode::getDesktopMode().height / 2.5f)
      screen_global_scale = 2;
    else
      screen_global_scale = 1;
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
  das_file_manual_reload();
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


  root_dir = fs::extract_dir(scheduled_script_name.c_str());
  if (!fs::change_dir(root_dir))
  {
    print_error("Cannot change directory to '%s'\n", root_dir.c_str());
    return;
  }

  main_das_file_name = fs::extract_file_name(scheduled_script_name.c_str());
  das_file_manual_reload();
}

void dasbox_execute(const char * file_name)
{
  scheduled_script_name = std::string(file_name);
  exec_script_scheduled = true;
}


void fetch_cerr()
{
  string s = logger.cerrStream.str();
  if (!s.empty())
  {
    print_error("%s", s.c_str());
    stringstream clearStream;
    logger.cerrStream.swap(clearStream);
  }
}


int main(int argc, char **argv)
{
  //setlocale(LC_ALL, "C");
  locale utf8Locale("en_US.UTF-8");
  g_locale = &utf8Locale;

  sf::err().rdbuf(logger.cerrStream.rdbuf());

  const char * default_menu_script_path = nullptr;
  if (fs::is_file_exists("samples/dasbox_initial_menu.das"))
    default_menu_script_path = "samples/dasbox_initial_menu.das";
  if (fs::is_file_exists("../samples/dasbox_initial_menu.das"))
    default_menu_script_path = "../samples/dasbox_initial_menu.das";


  if (argc == 1 && !default_menu_script_path)
  {
    print_usage();
    return 0;
  }

  process_args(argc, argv);
  if (has_errors)
    return 1;

  if (root_dir.empty() && main_das_file_name.empty())
  {
    main_das_file_name = "dasbox_initial_menu.das";
    root_dir = fs::extract_dir(default_menu_script_path);
  }

  fs::initialize();
  graphics::initialize();
  sound::initialize();

  if (!fs::change_dir(root_dir))
  {
    print_error("Cannot change directory to '%s'\n", root_dir.c_str());
    return 1;
  }

  if (!fs::is_file_exists(main_das_file_name.c_str()))
  {
    print_error("File does not exists '%s'\n", main_das_file_name.c_str());
    return 1;
  }

  NEED_MODULE(Module_BuiltIn);
  NEED_MODULE(Module_Math);
  NEED_MODULE(Module_Strings);
  NEED_MODULE(Module_Rtti);
  NEED_MODULE(Module_Ast);
  NEED_MODULE(Module_Debugger);
  NEED_MODULE(ModuleFio);
  NEED_MODULE(ModuleGraphics);
  NEED_MODULE(ModuleDasbox);
  NEED_MODULE(ModuleSound);

  load_module(main_das_file_name);
  fetch_cerr();

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

    float dt = deltaClock.restart().asSeconds();
    time_after_start += double(dt);

    dt = min(dt, 0.1f);
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
      vec4f arg = v_make_vec4f(dt, 0, 0, 0);
      exec_function(fn_act, &arg);
      fetch_cerr();
    }

    if (logger.topErrorLine >= 0)
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


    // render

    graphics::on_graphics_frame_start();

    if (screen_mode == SM_USER_APPLICATION)
      exec_function(fn_draw, nullptr);
    else
      draw_log_screen();

    if (use_separate_render_target)
    {
      render_texture->display();
      g_window->draw(*render_texture_sprite, sf::BlendNone);
    }

#ifdef _WIN32
    // Workaround unstable vsync on Windows 10
    if (vsync_enabled)
      glFinish();
#endif
    g_window->display();

    input::post_update_input();

    if (is_quit_scheduled && !return_to_file_name.empty())
      return_to_previous_script();

    if (exec_script_scheduled)
      exec_script_impl();

    if (recreate_window)
    {
      print_note("Window recreated");
      create_window();
    }

    if (is_quit_scheduled)
      g_window->close();
  }

  exec_function(fn_finalize, nullptr);

  sound::finalize();
  graphics::finalize();

  delete render_texture;
  delete render_texture_sprite;
  delete g_window;

  return 0;
}
