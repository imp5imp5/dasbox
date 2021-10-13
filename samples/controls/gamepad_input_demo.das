require daslib/media


var
    button = 0.0
    button_down = 0.0
    button_up = 0.0
    pos = 0.0
    scroll = 0.0
    cursor: Image
    mouse_pos: float2


[export]
def initialize()
    set_window_title("Gamepad Input Demo")
    return


[export]
def act(dt: float)
    if get_key(VK_ESCAPE)
        schedule_quit_game()

    button -= dt * 10.0
    button_down -= dt * 10.0
    button_up -= dt * 10.0

    if get_key(GP_A)
        button = 1.0

    if get_key_down(GP_A)
        button_down = 1.0

    if get_key_up(GP_A)
        button_up = 1.0

    scroll += get_mouse_scroll_delta()
    mouse_pos = get_mouse_position()



def draw_stick(at_x, at_y: int; stick_x, stick_y: float)
    circle(at_x, at_y, 100, 0x808080)
    fill_rect(at_x - 1, at_y - 1, 3, 3, 0x808080)
    line(float(at_x), float(at_y), float(at_x) + stick_x * 100.0, float(at_y) + stick_y * 100.0, 0xFFFFFF)


def draw_trigger(at_x, at_y: int; trigger: float)
    rect(at_x - 10, at_y, 20, 100, 0x808080)
    fill_rect(at_x - 10, at_y, 20, int(100.0 * trigger), 0xFFFFFF)


def draw_dirpad(at_x, at_y: int; dirpad_x, dirpad_y: float)
    rect(at_x - 20, at_y - 100, 40, 200, 0x808080)
    rect(at_x - 100, at_y - 20, 200, 40, 0x808080)
    fill_rect(at_x - 19, at_y - 99, 38, 198, 0x0)
    fill_rect(at_x - 99, at_y - 19, 198, 38, 0x0)
    line(float(at_x), float(at_y), float(at_x) + dirpad_x * 100.0, float(at_y), 0xFFFFFF)
    line(float(at_x), float(at_y), float(at_x), float(at_y) + dirpad_y * 100.0, 0xFFFFFF)


[export]
def draw()
    text_out(80, 300, "get_axis(AXIS_PRIMARY_Y) = {get_axis(AXIS_PRIMARY_Y)}", 0xF0F0F0)
    draw_stick(200, 150, get_axis(AXIS_PRIMARY_X), get_axis(AXIS_PRIMARY_Y))
    draw_stick(500, 150, get_axis(AXIS_SECONDARY_X), get_axis(AXIS_SECONDARY_Y))
    draw_trigger(50, 50, max(get_axis(AXIS_TRIGGERS), 0.0))
    draw_trigger(650, 50, max(-get_axis(AXIS_TRIGGERS), 0.0))
    draw_dirpad(850, 150, get_axis(AXIS_POV_X), get_axis(AXIS_POV_Y))

    fill_rect(50, 350, 20, 18, make_color(button))
    text_out(80, 350, "get_key(GP_A)", 0xF0F0F0)

    fill_rect(50, 400, 20, 18, make_color(button_down))
    text_out(80, 400, "get_key_down(GP_A)", 0xF0F0F0)

    fill_rect(50, 450, 20, 18, make_color(button_up))
    text_out(80, 450, "get_key_up(GP_A)", 0xF0F0F0)


