require daslib/media

    //  Mouse Button Codes:

    //  MB_LEFT
    //  MB_RIGHT
    //  MB_MIDDLE


var
    button = 0.0
    button_down = 0.0
    button_up = 0.0
    pos = 0.0
    scroll = 0.0
    mouse_pos: float2
    relative_mode = false


[export]
def initialize()
    set_window_title("Mouse Input Demo")


[export]
def act(dt: float)
    if get_key(VK_ESCAPE)
        schedule_quit_game()

    button -= dt * 10.0
    button_down -= dt * 10.0
    button_up -= dt * 10.0

    if get_mouse_button(MB_LEFT)
        button = 1.0

    if get_mouse_button_down(MB_LEFT)
        button_down = 1.0

    if get_mouse_button_up(MB_LEFT)
        button_up = 1.0

    scroll += get_mouse_scroll_delta()
    mouse_pos = get_mouse_position()

    if get_key_down(VK_SPACE)
        relative_mode = !relative_mode
        set_relative_mouse_mode(relative_mode)


[export]
def draw()
    fill_rect(50, 50, 20, 18, make_color(button))
    text_out(80, 50, "get_mouse_button(MB_LEFT)", 0xF0F0F0)

    fill_rect(50, 100, 20, 18, make_color(button_down))
    text_out(80, 100, "get_mouse_button_down(MB_LEFT)", 0xF0F0F0)

    fill_rect(50, 150, 20, 18, make_color(button_up))
    text_out(80, 150, "get_mouse_button_up(MB_LEFT)", 0xF0F0F0)

    text_out(80, 200, "scroll += get_mouse_scroll_delta(),    scroll = {scroll}", 0xF0F0F0)

    text_out(80, 250, "get_mouse_position() = {get_mouse_position()}", 0xF0F0F0)
    text_out(80, 300, "get_mouse_position_delta() = {get_mouse_position_delta()}", 0xF0F0F0)
    text_out(80, 350, "get_mouse_velocity() = {get_mouse_velocity()}", 0xF0F0F0)
    text_out(80, 400, "set_relative_mouse_mode({relative_mode})  // Press SPACE to toggle mode", 0xFFD080)

