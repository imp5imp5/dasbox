require daslib/media

    // Key Codes:

    // VK_ESCAPE
    // VK_RETURN        Enter on main keyboard
    // VK_SPACE

    // VK_LEFT          LeftArrow on arrow keypad
    // VK_RIGHT         RightArrow on arrow keypad
    // VK_UP            UpArrow on arrow keypad
    // VK_DOWN          DownArrow on arrow keypad

    // VK_LSHIFT        Left Shift
    // VK_RSHIFT        Right Shift
    // VK_LCONTROL      Left Control
    // VK_RCONTROL      Right Control
    // VK_LALT          Left Alt
    // VK_RALT          Right Alt

    // VK_LWIN          Left Windows key
    // VK_RWIN          Right Windows key
    // VK_TAB
    // VK_CAPITAL       Caps Lock

    // VK_HOME          Home on arrow keypad
    // VK_PRIOR         PgUp on arrow keypad
    // VK_END           End on arrow keypad
    // VK_NEXT          PgDn on arrow keypad
    // VK_INSERT        Insert on arrow keypad
    // VK_DELETE        Delete on arrow keypad

    // VK_GRAVE         Accent grave, ~
    // VK_MINUS         - On main keyboard
    // VK_EQUALS        =
    // VK_BACK          Backspace

    // VK_LBRACKET      [
    // VK_RBRACKET      ]
    // VK_SEMICOLON     ;
    // VK_APOSTROPHE    '
    // VK_BACKSLASH     \
    // VK_COMMA         ,
    // VK_PERIOD        . on main keyboard
    // VK_SLASH         / on main keyboard

    // VK_MULTIPLY      * on numeric keypad
    // VK_SUBTRACT      - On numeric keypad
    // VK_ADD           + On numeric keypad
    // VK_NUMPAD0, VK_NUMPAD1 ....
    // VK_DECIMAL       . On numeric keypad
    // VK_NUMPADENTER   Enter on numeric keypad
    // VK_DIVIDE        / On numeric keypad

    // VK_0, VK_1, VK_2 ...

    // VK_A, VK_B, VK_C ...

    // VK_F1, VK_F2, VK_F3 ...

    // VK_NUMLOCK
    // VK_SCROLL        Scroll Lock
    // VK_SYSRQ         Print Screen
    // VK_PAUSE         Pause
    // VK_APPS          AppMenu key



var
    space = 0.0
    space_down = 0.0
    space_up = 0.0
    space_press = 0.0
    entered_text: array<uint8>


[export]
def initialize()
    set_window_title("Keyboard Input Demo")


[export]
def act(dt: float)
    if get_key(VK_ESCAPE)
        schedule_quit_game()

    space -= dt * 10.0
    space_down -= dt * 10.0
    space_up -= dt * 10.0
    space_press -= dt * 10.0

    if get_key(VK_SPACE)
        space = 1.0

    if get_key_down(VK_SPACE)
        space_down = 1.0

    if get_key_up(VK_SPACE)
        space_up = 1.0

    if get_key_press(VK_SPACE)
        space_press = 1.0

    var code = fetch_entered_symbol()
    if code != 0u
        if code >= 0x20
            for c in convert_utf8_code_to_chars(code)
                entered_text |> push(c)
        if code == 0x08 && length(entered_text) > 0
            while !empty(entered_text) && !is_first_byte_of_utf8_char(entered_text[length(entered_text) - 1])
                entered_text |> resize(length(entered_text) - 1)
            entered_text |> resize(length(entered_text) - 1)


[export]
def draw()
    fill_rect(50, 50, 20, 18, make_color(space))
    text_out(80, 50, "get_key(VK_SPACE)", 0xF0F0F0)

    fill_rect(50, 100, 20, 18, make_color(space_down))
    text_out(80, 100, "get_key_down(VK_SPACE)", 0xF0F0F0)

    fill_rect(50, 150, 20, 18, make_color(space_up))
    text_out(80, 150, "get_key_up(VK_SPACE)", 0xF0F0F0)

    fill_rect(50, 200, 20, 18, make_color(space_press))
    text_out(80, 200, "get_key_press(VK_SPACE)", 0xF0F0F0)

    text_out(80, 250, "get_key_name(get_pressed_key_index()) = {get_key_name(get_pressed_key_index())}", 0xF0F0F0)

    text_out(80, 300, "Entered text: {string(entered_text)}_", 0xF0F0F0)


