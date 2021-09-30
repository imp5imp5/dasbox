#include "input.h"
#include "sound.h"
#include "logger.h"
#include <daScript/daScript.h>
#include <daScript/ast/ast.h>
#include <daScript/simulate/interop.h>
#include <daScript/simulate/simulate_visit_op.h>
#include <SFML/System.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Joystick.hpp>
#include <unordered_map>
#include <string>



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


void set_window_title(const char * title);
void set_antialiasing(int a);
void set_resolution(int width, int height);
void set_rendering_upscale(int upscale);
void disable_auto_upscale();

void schedule_pause();
void schedule_quit_game();
float get_time_after_start();
float get_delta_time();
bool is_window_active();
void set_vsync_enabled(bool enalbe);
void set_mouse_cursor_visible(bool visible);
void set_mouse_cursor_grabbed(bool grabbed);


const char * das_get_key_name(int key_code)
{
  auto it = code_to_key_name.find(key_code);
  if (it == code_to_key_name.end())
    return "unknown";
  else
    return it->second;
}

int das_get_key_code(const string & key_name)
{
  auto it = key_name_to_code.find(key_name);
  if (it == key_name_to_code.end())
    return -1;
  else
    return it->second;
}


static char utils_das[] =
#include "utils.das.inl"
;



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
    addConstant(*this, "AXIS_SECONDARY_Y", 5 + AXIS_CODE_OFFSET + 5);
    addConstant(*this, "AXIS_POV_X", AXIS_CODE_OFFSET + 6);
    addConstant(*this, "AXIS_POV_Y", AXIS_CODE_OFFSET + 7);


    addConstant(*this, "MB_LEFT", MOUSE_CODE_OFFSET + 0);
    addConstant(*this, "MB_RIGHT", MOUSE_CODE_OFFSET + 1);
    addConstant(*this, "MB_MIDDLE", MOUSE_CODE_OFFSET + 2);


    addExtern<DAS_BIND_FUN(das_get_key)>(*this, lib, "get_key", SideEffects::accessExternal, "das_get_key");
    addExtern<DAS_BIND_FUN(das_get_key_down)>(*this, lib, "get_key_down", SideEffects::accessExternal, "das_get_key_down");
    addExtern<DAS_BIND_FUN(das_get_key_up)>(*this, lib, "get_key_up", SideEffects::accessExternal, "das_get_key_up");
    addExtern<DAS_BIND_FUN(input::fetch_entered_symbol)>
      (*this, lib, "fetch_entered_symbol", SideEffects::accessExternal, "input::fetch_entered_symbol");

    addExtern<DAS_BIND_FUN(das_get_key_name)>
      (*this, lib, "get_key_name", SideEffects::accessExternal, "das_get_key_name");
    addExtern<DAS_BIND_FUN(das_get_key_press)>
      (*this, lib, "get_key_press", SideEffects::accessExternal, "das_get_key_press");
    addExtern<DAS_BIND_FUN(das_get_mouse_button)>
      (*this, lib, "get_mouse_button", SideEffects::accessExternal, "das_get_mouse_button");
    addExtern<DAS_BIND_FUN(das_get_mouse_button_up)>
      (*this, lib, "get_mouse_button_up", SideEffects::accessExternal, "das_get_mouse_button_up");
    addExtern<DAS_BIND_FUN(das_get_mouse_button_down)>
      (*this, lib, "get_mouse_button_down", SideEffects::accessExternal, "das_get_mouse_button_down");
    addExtern<DAS_BIND_FUN(input::get_mouse_scroll_delta)>
      (*this, lib, "get_mouse_scroll_delta", SideEffects::accessExternal, "input::get_mouse_scroll_delta");
    addExtern<DAS_BIND_FUN(input::get_mouse_position)>
      (*this, lib, "get_mouse_position", SideEffects::accessExternal, "input::get_mouse_position");
    addExtern<DAS_BIND_FUN(input::get_mouse_position_delta)>
      (*this, lib, "get_mouse_position_delta", SideEffects::accessExternal, "input::get_mouse_position_delta");
    addExtern<DAS_BIND_FUN(input::get_mouse_velocity)>
      (*this, lib, "get_mouse_velocity", SideEffects::accessExternal, "input::get_mouse_velocity");
    addExtern<DAS_BIND_FUN(input::set_relative_mouse_movement)>
      (*this, lib, "set_relative_mouse_movement", SideEffects::modifyExternal, "input::set_relative_mouse_movement");
    addExtern<DAS_BIND_FUN(das_get_axis)>
      (*this, lib, "get_axis", SideEffects::accessExternal, "das_get_axis");


    addExtern<DAS_BIND_FUN(set_window_title)>
      (*this, lib, "set_window_title", SideEffects::modifyExternal, "set_window_title");
    addExtern<DAS_BIND_FUN(set_antialiasing)>
      (*this, lib, "set_antialiasing", SideEffects::modifyExternal, "set_antialiasing");
    addExtern<DAS_BIND_FUN(set_resolution)>
      (*this, lib, "set_resolution", SideEffects::modifyExternal, "set_resolution");
    addExtern<DAS_BIND_FUN(set_rendering_upscale)>
      (*this, lib, "set_rendering_upscale", SideEffects::modifyExternal, "set_rendering_upscale");
    addExtern<DAS_BIND_FUN(disable_auto_upscale)>
      (*this, lib, "disable_auto_upscale", SideEffects::modifyExternal, "disable_auto_upscale");

    addExtern<DAS_BIND_FUN(schedule_pause)>
      (*this, lib, "schedule_pause", SideEffects::modifyExternal, "schedule_pause");
    addExtern<DAS_BIND_FUN(schedule_quit_game)>
      (*this, lib, "schedule_quit_game", SideEffects::modifyExternal, "schedule_quit_game");
    addExtern<DAS_BIND_FUN(get_time_after_start)>
      (*this, lib, "get_time_after_start", SideEffects::modifyExternal, "get_time_after_start");
    addExtern<DAS_BIND_FUN(get_delta_time)>
      (*this, lib, "get_delta_time", SideEffects::modifyExternal, "get_delta_time");
    addExtern<DAS_BIND_FUN(is_window_active)>
      (*this, lib, "is_window_active", SideEffects::modifyExternal, "is_window_active");
    addExtern<DAS_BIND_FUN(set_vsync_enabled)>
      (*this, lib, "set_vsync_enabled", SideEffects::modifyExternal, "set_vsync_enabled");
    addExtern<DAS_BIND_FUN(set_mouse_cursor_visible)>
      (*this, lib, "set_mouse_cursor_visible", SideEffects::modifyExternal, "set_mouse_cursor_visible");
    addExtern<DAS_BIND_FUN(set_mouse_cursor_grabbed)>
      (*this, lib, "set_mouse_cursor_grabbed", SideEffects::modifyExternal, "set_mouse_cursor_grabbed");

    compileBuiltinModule("utils.das", (unsigned char *)utils_das, sizeof(utils_das));

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



namespace das {
  void builtin_sleep(uint32_t msec);
  void builtin_exit(int32_t ec);

  class ModuleFio : public Module {
  public:
    ModuleFio() : Module("fio") {
      ModuleLibrary lib;
      lib.addModule(this);
      lib.addBuiltInModule();
      lib.addModule(Module::require("strings"));

      addExtern<DAS_BIND_FUN(builtin_sleep)>(*this, lib, "sleep",
        SideEffects::modifyExternal, "builtin_sleep")
        ->arg("msec");

      addExtern<DAS_BIND_FUN(builtin_exit)>(*this, lib, "exit",
        SideEffects::modifyExternal, "builtin_exit")
        ->arg("exitCode")->unsafeOperation = true;

      uint32_t verifyFlags = uint32_t(VerifyBuiltinFlags::verifyAll);
      verifyFlags &= ~VerifyBuiltinFlags::verifyHandleTypes;
      verifyBuiltinNames(verifyFlags);
      verifyAotReady();
    }
    virtual ModuleAotType aotRequire ( TextWriter & tw ) const override {
      //tw << "#include \"daScript/misc/performance_time.h\"\n";
      //tw << "#include \"daScript/simulate/aot_builtin_fio.h\"\n";
      return ModuleAotType::cpp;
    }
  };
}

REGISTER_MODULE_IN_NAMESPACE(ModuleFio, das);
