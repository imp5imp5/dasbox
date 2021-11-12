#include "input.h"
#include "globals.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <vector>

using namespace das;
using namespace std;

namespace input
{

static float min_mouse_step = 0.02f; //. 0.005f
static float time_between_mouse_update = 0.0f;
bool is_any_key_down = false;
static vector<uint32_t> entered_symbols;

#define K_NOT_PRESSED 0
#define K_PRESSED_THIS_FRAME 1
#define K_PRESSED 2
#define K_RELEASED_THIS_FRAME 3
#define K_CLICKED_THIS_FRAME 4

#define DKEY__MAX_BUTTONS (sf::Keyboard::KeyCount + 1)

static char key[KEY_COUNT] = { 0 };
static char mouse_button[MOUSE_BUTTONS_COUNT] = { 0 };
//static char gamepad_button[GAMEPAD_BUTTONS_COUNT] = { 0 };
//static HumanInput::JoystickRawState cur_joystick_raw_state;
static float mouse_scroll_accum = 0;
static int last_key_index = -1;
static bool is_last_key_repeat = false;
static float gamepad_axes[GAMEPAD_AXES] = { 0 };
static bool delayed_relative_mode = false;
static bool relative_mouse_mode = false;
static bool prev_window_is_active = false;

static float mouse_vel_x = 0.f;
static float mouse_vel_y = 0.f;
static float mouse_dx = 0.f;
static float mouse_dy = 0.f;
static float mouse_x = 0.f;
static float mouse_y = 0.f;
static float virtual_mouse_x = 0.f;
static float virtual_mouse_y = 0.f;

void reset_input()
{
  memset(mouse_button, 0, sizeof(mouse_button));
//  memset(gamepad_button, 0, sizeof(gamepad_button));
  memset(key, 0, sizeof(key));
  memset(gamepad_axes, 0, sizeof(gamepad_axes));
  mouse_scroll_accum = 0;
  last_key_index = -1;
  is_last_key_repeat = true;
  is_any_key_down = false;

  mouse_vel_x = 0.f;
  mouse_vel_y = 0.f;
  mouse_dx = 0.f;
  mouse_dy = 0.f;

  float invScale = 1.f / screen_global_scale;
  if (g_window)
  {
    mouse_x = sf::Mouse::getPosition(*g_window).x * invScale;
    mouse_y = sf::Mouse::getPosition(*g_window).y * invScale;
  }

  entered_symbols.clear();
}

static void post_update_keys(char * keys, int count)
{
  entered_symbols.clear();

  for (int i = 0; i < count; i++)
    if (keys[i] != K_NOT_PRESSED)
    {
      if (keys[i] == K_PRESSED_THIS_FRAME)
        keys[i] = K_PRESSED;
      else if (keys[i] == K_CLICKED_THIS_FRAME || keys[i] == K_RELEASED_THIS_FRAME)
        keys[i] = K_NOT_PRESSED;
    }
}


inline char next_state_after_press(char s)
{
  if (s == K_CLICKED_THIS_FRAME || s == K_NOT_PRESSED)
    return K_PRESSED_THIS_FRAME;
  else if (s == K_RELEASED_THIS_FRAME)
    return K_PRESSED;
  return s;
}

inline char next_state_after_release(char s)
{
  if (s == K_PRESSED_THIS_FRAME)
    return K_CLICKED_THIS_FRAME;
  else if (s == K_RELEASED_THIS_FRAME)
    return K_NOT_PRESSED;
  else if (s == K_PRESSED)
    return K_RELEASED_THIS_FRAME;
  return s;
}

static void release_input(char * keys, int count)
{
  for (int i = 0; i < count; i++)
    if (keys[i] != K_NOT_PRESSED)
    {
      keys[i] = next_state_after_release(keys[i]);
      if (keys[i] == K_CLICKED_THIS_FRAME)
        keys[i] = K_NOT_PRESSED;
    }
}

bool get_key_down(int key_code)
{
  if (key_code < 0 || key_code >= KEY_COUNT)
    return false;
  return (key[key_code] == K_PRESSED_THIS_FRAME || key[key_code] == K_CLICKED_THIS_FRAME);
}

bool get_key(int key_code)
{
  if (key_code < 0 || key_code >= KEY_COUNT)
    return false;
  return (key[key_code] == K_PRESSED || get_key_down(key_code));
}

bool get_key_up(int key_code)
{
  if (key_code < 0 || key_code >= KEY_COUNT)
    return false;
  return (key[key_code] == K_RELEASED_THIS_FRAME || key[key_code] == K_CLICKED_THIS_FRAME);
}

bool get_key_press(int key_code)
{
  if (key_code < 0 || key_code >= KEY_COUNT)
    return false;
  return get_key_down(key_code) || (is_last_key_repeat && key_code == last_key_index);
}

uint32_t fetch_entered_symbol()
{
  if (entered_symbols.empty())
    return 0;
  else
  {
    uint32_t code = entered_symbols[0];
    entered_symbols.erase(entered_symbols.begin());
    return code;
  }
}

int get_pressed_key_index()
{
  return last_key_index;
}


bool get_mouse_button_down(int button_code)
{
  if (button_code < 0 || button_code >= MOUSE_BUTTONS_COUNT)
    return false;
  return (mouse_button[button_code] == K_PRESSED_THIS_FRAME || mouse_button[button_code] == K_CLICKED_THIS_FRAME);
}

bool get_mouse_button(int button_code)
{
  if (button_code < 0 || button_code >= MOUSE_BUTTONS_COUNT)
    return false;
  return (mouse_button[button_code] == K_PRESSED || get_mouse_button_down(button_code));
}

bool get_mouse_button_up(int button_code)
{
  if (button_code < 0 || button_code >= MOUSE_BUTTONS_COUNT)
    return false;
  return (mouse_button[button_code] == K_RELEASED_THIS_FRAME || mouse_button[button_code] == K_CLICKED_THIS_FRAME);
}


float get_mouse_scroll_delta()
{
  return mouse_scroll_accum;
}

das::float2 get_mouse_position()
{
  return relative_mouse_mode ? das::float2(virtual_mouse_x, virtual_mouse_y) : das::float2(mouse_x, mouse_y);
}

das::float2 get_mouse_position_delta()
{
  return das::float2(mouse_dx, mouse_dy);
}

das::float2 get_mouse_velocity()
{
  return das::float2(mouse_vel_x, mouse_vel_y);
}



float get_axis(int axis)
{
  if (axis < AXIS_CODE_OFFSET || axis >= AXIS_CODE_OFFSET + GAMEPAD_AXES)
    return 0.0f;
  return gamepad_axes[axis - AXIS_CODE_OFFSET];
}


void release_input()
{
  prev_window_is_active = false;
  release_input(key, countof(key));
  release_input(mouse_button, countof(mouse_button));
//  release_input(gamepad_button, countof(gamepad_button));
}

static sf::Cursor * empty_cursor = nullptr;
static sf::Cursor * arrow_cursor = nullptr;

static void create_cursors()
{
  if (empty_cursor)
    return;

  empty_cursor = new sf::Cursor();
  vector<uint8_t> buf(4 * 32 * 32, 0);
  buf[0] = 0x7f;
  buf[1] = 0x7f;
  buf[2] = 0x7f;
  buf[3] = 0x01;

  empty_cursor->loadFromPixels(&buf[0], sf::Vector2u(32, 32), sf::Vector2u(1, 1));

  arrow_cursor = new sf::Cursor();
  arrow_cursor->loadFromSystem(sf::Cursor::Arrow);
}



static void hide_cursor()
{
#ifdef _WIN32
  create_cursors();
  g_window->setMouseCursor(*empty_cursor);
#else
  g_window->setMouseCursorVisible(false);
#endif
}

static void show_cursor()
{
#ifdef _WIN32
  create_cursors();
  g_window->setMouseCursor(*arrow_cursor);
#else
  g_window->setMouseCursorVisible(true);
#endif
}


void update_mouse_input(float dt, bool active)
{
  if (!active)
  {
    mouse_scroll_accum = 0;
    last_key_index = -1;
    is_last_key_repeat = false;
    mouse_dx = 0.f;
    mouse_dy = 0.f;
  }

  float invScale = 1.f / screen_global_scale;

  float prevX = mouse_x;
  float prevY = mouse_y;
  mouse_x = sf::Mouse::getPosition(*g_window).x * invScale;
  mouse_y = sf::Mouse::getPosition(*g_window).y * invScale;
  mouse_dx = (mouse_x - prevX);
  mouse_dy = (mouse_y - prevY);

  if (delayed_relative_mode)
    set_relative_mouse_mode(true);

  if (relative_mouse_mode && window_is_active)
  {
    if (!prev_window_is_active)
    {
      virtual_mouse_x = sf::Mouse::getPosition(*g_window).x * invScale;
      virtual_mouse_y = sf::Mouse::getPosition(*g_window).y * invScale;
    }

    sf::Vector2i center = sf::Vector2i(screen_width * screen_global_scale / 2, screen_height * screen_global_scale / 2);
    sf::Mouse::setPosition(center, *g_window);
    mouse_x = screen_width / 2;
    mouse_y = screen_height / 2;
    virtual_mouse_x = ::clamp(virtual_mouse_x + mouse_dx, 0.0f, screen_width - 1.0f);
    virtual_mouse_y = ::clamp(virtual_mouse_y + mouse_dy, 0.0f, screen_height - 1.0f);
  }

  float k = 1.5f;//fabsf(mouse_dx) < 3 && fabsf(mouse_dx) < 3 ? 1.1f : 1.5f;

  bool moved = (mouse_dx != 0 || mouse_dy != 0);
  if (moved)
  {
    if (mouse_vel_x == 0 && mouse_vel_y == 0)
    {
      float invT = 1.f / std::max(dt, min_mouse_step * k);
      mouse_vel_x = mouse_dx * invT;
      mouse_vel_y = mouse_dy * invT;
    }
    else
    {
      float invT = 1.f / std::max(time_between_mouse_update + dt, min_mouse_step);
      mouse_vel_x = mouse_dx * invT;
      mouse_vel_y = mouse_dy * invT;
    }

    if (time_between_mouse_update < min_mouse_step)
      min_mouse_step = std::max(time_between_mouse_update, 0.005f);

    min_mouse_step += dt * 0.0001f;
    min_mouse_step = std::min(min_mouse_step, 0.02f);

    time_between_mouse_update = 0;
  }
  else
  {
    if (time_between_mouse_update > min_mouse_step * k)
    {
      mouse_vel_x = 0;
      mouse_vel_y = 0;
    }
    time_between_mouse_update += dt;
  }
}


void post_update_input()
{
  prev_window_is_active = window_is_active;
  is_last_key_repeat = false;
  is_any_key_down = false;
  mouse_dx = 0.f;
  mouse_dy = 0.f;
  mouse_scroll_accum = 0;
  post_update_keys(key, countof(key));
  post_update_keys(mouse_button, countof(mouse_button));
//  post_update_keys(gamepad_button, countof(gamepad_button));
}

void gmc_mouse_button_down(int btn_idx)
{
  if (btn_idx >= 0 && btn_idx< MOUSE_BUTTONS_COUNT)
    mouse_button[btn_idx] = next_state_after_press(mouse_button[btn_idx]);
}

void gmc_mouse_button_up(int btn_idx)
{
  if (btn_idx >= 0 && btn_idx < MOUSE_BUTTONS_COUNT)
    mouse_button[btn_idx] = next_state_after_release(mouse_button[btn_idx]);
}

void gmc_mouse_wheel(float scroll)
{
  mouse_scroll_accum += scroll;
}

void gkc_button_down(int btn_idx)
{
  if (get_key(btn_idx))
    is_last_key_repeat = true;
  is_any_key_down = true;
  last_key_index = btn_idx;
  if (btn_idx >= 0 && btn_idx < DKEY__MAX_BUTTONS)
    key[btn_idx] = next_state_after_press(key[btn_idx]);
}

void gkc_symbol_entered(uint32_t code)
{
  entered_symbols.push_back(code);
}


void gkc_button_up(int btn_idx)
{
  if (btn_idx == last_key_index)
    last_key_index = -1;
  if (btn_idx >= 0 && btn_idx < DKEY__MAX_BUTTONS)
    key[btn_idx] = next_state_after_release(key[btn_idx]);
}

void joy_button_down(int btn_idx)
{
  if (btn_idx >= 0 && btn_idx < GAMEPAD_BUTTONS_COUNT)
    key[btn_idx + 256] = next_state_after_press(key[btn_idx + 256]);
}

void joy_button_up(int btn_idx)
{
  if (btn_idx >= 0 && btn_idx < GAMEPAD_BUTTONS_COUNT)
    key[btn_idx + 256] = next_state_after_release(key[btn_idx + 256]);
}

void joy_axis_position(int axis_idx, float axis_pos)
{
  if (axis_idx >= 0 && axis_idx < GAMEPAD_AXES)
  {
    if (fabs(axis_pos) < 1.0f)
      axis_pos = 0.0f;
    else
      axis_pos = sign(axis_pos) * (fabsf(axis_pos) - 1.0f) * (1.f / 99.f);
    gamepad_axes[axis_idx] = ::clamp(axis_pos, -1.0f, 1.0f);
  }
}


bool is_relative_mouse_mode()
{
  return delayed_relative_mode || relative_mouse_mode;
}

void set_relative_mouse_mode(bool relative)
{
  if (!relative)
  {
    delayed_relative_mode = false;
    relative_mouse_mode = false;
  }

  if (!g_window)
  {
    delayed_relative_mode = relative;
    return;
  }

  if (g_window && relative && !window_is_active)
  {
    delayed_relative_mode = relative;
    return;
  }

  bool mouseDown = sf::Mouse::isButtonPressed(sf::Mouse::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Right);

  if (relative)
  {
    if (mouseDown || !g_window->isOpen() || !window_is_active)
    {
      delayed_relative_mode = true;
      return;
    }

    virtual_mouse_x = mouse_x;
    virtual_mouse_y = mouse_y;

    g_window->setMouseCursorGrabbed(true);
    hide_cursor();
    sf::Vector2i center = sf::Vector2i(screen_width * screen_global_scale / 2, screen_height * screen_global_scale / 2);
    sf::Mouse::setPosition(center, *g_window);
    mouse_x = screen_width / 2;
    mouse_y = screen_height / 2;
    relative_mouse_mode = true;
    delayed_relative_mode = false;
  }
  else
  {
    show_cursor();
    g_window->setMouseCursorGrabbed(false);
  }
}

}
