#pragma once

#include "globals.h"
#include <daScript/daScript.h>


#define MOUSE_BUTTONS_COUNT 7
#define GAMEPAD_BUTTONS_COUNT 32
#define KEY_COUNT (256 + 32)
#define GAMEPAD_AXES 16

#define KEY_CODE_OFFSET 1000
#define MOUSE_CODE_OFFSET 2000
#define AXIS_CODE_OFFSET 3000

namespace input
{

void reset_input();
bool get_key(int key_code);
bool get_key_down(int key_code);
bool get_key_up(int key_code);
bool get_key_press(int key_code);
uint32_t fetch_entered_symbol();
bool get_mouse_button(int button_code);
bool get_mouse_button_up(int button_code);
bool get_mouse_button_down(int button_code);
float get_mouse_scroll_delta();
das::float2 get_mouse_position();
das::float2 get_mouse_position_delta();
das::float2 get_mouse_velocity();
void set_relative_mouse_mode(bool relative);
bool is_relative_mouse_mode();
bool is_cursor_hidden();
float get_axis(int axis);
int get_pressed_key_index();


void release_input();
void update_mouse_input(float dt, bool active);
void post_update_input();
void gmc_mouse_button_down(int btn_idx);
void gmc_mouse_button_up(int btn_idx);
void gmc_mouse_wheel(float scroll);
void gkc_button_down(int btn_idx);
void gkc_symbol_entered(uint32_t code);
void gkc_button_up(int btn_idx);
void joy_button_down(int btn_idx);
void joy_button_up(int btn_idx);
void joy_axis_position(int axis_idx, float axis_pos);


void hide_cursor();
void show_cursor();

}
