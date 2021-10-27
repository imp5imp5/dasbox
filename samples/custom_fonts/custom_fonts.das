require daslib/media

[export]
def initialize
    set_window_title("Custom Fonts")

[export]
def act(dt: float)
    if get_key(VK_ESCAPE)
        schedule_quit_game()

[export]
def draw
    fill_rect(0, 0, get_screen_width(), get_screen_height(), make_color(0.2))
    fill_rect(0, 100, get_screen_width(), 100, make_color(0.8, 0.8, 0.75))
    fill_rect(0, 300, get_screen_width(), 100, make_color(0.8, 0.8, 0.75))
    fill_rect(0, 500, get_screen_width(), 100, make_color(0.8, 0.8, 0.75))

    set_font_name("mono")
    set_font_size(15)
    text_out(100, 64, "set_font_name(\"Aqum.ttf\")", make_color(1.0))

    set_font_name("Aqum.ttf")
    set_font_size(50)
    text_out(100, 120, "Aqum.ttf", make_color(0.2, 0.4, 0.6))


    set_font_name("mono")
    set_font_size(15)

    text_out(100, 264, "set_font_name(\"Shadower.ttf\")", make_color(1.0))
    set_font_name("Shadower.ttf")
    set_font_size(50)
    text_out(100, 320, "Shadower.ttf", make_color(0.4, 0.2, 0.1))


    set_font_name("mono")
    set_font_size(15)

    text_out(100, 464, "set_font_name(\"sans\")  // built-in font", make_color(1.0))
    set_font_name("sans")
    set_font_size(50)
    text_out(100, 520, "Open Sans", make_color(0.2, 0.3, 0.1))

