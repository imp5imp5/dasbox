include daslib/internal_menu_background.das
require fio

let
    SELECT_Y = 328.0
    LIST_LINE_HEIGHT = 60.0
    DESCRIPTION_LINE_HEIGHT = 23


let minimal_sample = "include daslib/media

[export]
def initialize
    return

[export]
def act(dt: float)
    return

[export]
def draw
    return
"


struct MenuItem
    name: string
    source: string
    clipboardText: string
    image: Image const
    description: array<string>


var
    menuIndex:int
    nextIndex:int
    scrollAccum = 0.0
    itemAlpha = 0.0
    itemAlphaTrend = 1.0
    scrollPos = 0.0
    wishScrollPos = 0.0
    selectBoxPos = SELECT_Y
    wishSelectBoxPos = SELECT_Y
    fadeout = 0.0
    fadeoutTrend = -1.0

    menuItems: array<MenuItem?> <- [{ auto
    new [[MenuItem name = "About",
          description <- [{string[]
              "Dasbox 0.1.1";   // TODO: get from .exe
              "Build date: 04.10.2021";
              "https://github.com/imp5imp5/dasbox"
              }]
        ]];

    new [[MenuItem name = "New Application",
          clipboardText = minimal_sample,
          description <- [{string[]
              "Press Ctrl+C to copy code of the minimal sample to clipboard.";
              "Execution: run from command line \"dasbox.exe <your_app.das>\"";
              "Source: samples/minimal.das"
              }]
        ]];

    new [[MenuItem name = "Flappy Box",
          image <- create_image("flappy_box/_preview.png"),
          source = "flappy_box/flappy_box.das",
          description <- [{string[]
              "This example shows how to display graphical primitives";
              "and how to handle the simplest keyboard input.";
              "";
              "Controls:";
              "Space - Jump";
              "Esc - Return to this menu";
              "";
              "Source: samples/flappy_box/flappy_box.das"
              }]
        ]];

    new [[MenuItem name = "Space, Rocks and Lasers",
          image <- create_image("space_rocks_and_lasers/_preview.png"),
          source = "space_rocks_and_lasers/rocks_and_lasers.das",
          description <- [{string[]
              "This example shows how to display graphical primitives";
              "and the simplest keyboard input.";
              "";
              "Controls:";
              "Arrow Keys - Move";
              "Space - Fire";
              "Esc - Return to this menu";
              "";
              "Source: samples/space_rocks_and_lasers/rocks_and_lasers.das"
              }]
        ]];

    new [[MenuItem name = "Keyboard Input",
          image <- create_image("controls/_preview_keyboard.png"),
          source = "controls/keyboard_input_demo.das",
          description <- [{string[]
              "Example of functions for processing keyboard input.";
              "";
              "Controls:";
              "Esc - Return to this menu";
              "";
              "Source: controls/keyboard_input_demo.das"
              }]
        ]];

    new [[MenuItem name = "Mouse Input",
          image <- create_image("controls/_preview_mouse.png"),
          source = "controls/mouse_input_demo.das",
          description <- [{string[]
              "Example of functions for processing mouse input.";
              "";
              "Controls:";
              "Esc - Return to this menu";
              "";
              "Source: controls/mouse_input_demo.das"
              }]
        ]];

    new [[MenuItem name = "Gamepad Input",
          image <- create_image("controls/_preview_gamepad.png"),
          source = "controls/gamepad_input_demo.das",
          description <- [{string[]
              "Example of functions for processing gampad input.";
              "";
              "Controls:";
              "Esc - Return to this menu";
              "";
              "Source: controls/gamepad_input_demo.das"
              }]
        ]]
}]



[export]
def initialize()
    background_initialize()
    set_font_name("sans")

    let s = local_storage_get("dasbox_initial_menu/menuIndex");
    menuIndex = length(s) == 0 ? 1 : clamp(int(s), 0, length(menuItems) - 1)
    nextIndex = menuIndex
    scrollPos = float(menuIndex)
    wishScrollPos = scrollPos
    if menuIndex != 1
        fadeout = 1.0
        fadeoutTrend = -1.0


[export]
def act(dt: float)
    if (get_time_after_start() > 3600.0)
        reset_time_after_start()

    scrollAccum -= get_mouse_scroll_delta()
    if get_key_press(VK_DOWN)
        scrollAccum += 1.0
    if get_key_press(VK_UP)
        scrollAccum -= 1.0

    if (get_key(VK_LCONTROL) || get_key(VK_RCONTROL)) && get_key_press(VK_C) && length(deref(menuItems[menuIndex]).clipboardText) > 0
        set_clipboard_text(deref(menuItems[menuIndex]).clipboardText)

    if abs(scrollAccum) >= 1.0 && fadeout <= 0.0
        let next = clamp(nextIndex + int(scrollAccum), 0, length(menuItems) - 1)
        scrollAccum -= float(int(scrollAccum))
        if next != menuIndex
            nextIndex = next
            local_storage_set("dasbox_initial_menu/menuIndex", string(nextIndex))
            itemAlphaTrend = -1.0
            wishScrollPos = float(nextIndex)

    itemAlpha += dt * itemAlphaTrend * 14.0
    if itemAlpha < 0.0
        itemAlphaTrend = 1.0
        menuIndex = nextIndex
    itemAlpha = clamp(itemAlpha, -1.0, 1.0)

    scrollPos = move_to(scrollPos, wishScrollPos, dt, 0.2)
    scrollPos = approach(scrollPos, wishScrollPos, dt, 0.06)

    wishSelectBoxPos = SELECT_Y + (float(nextIndex) - scrollPos) * LIST_LINE_HEIGHT
    selectBoxPos = move_to(selectBoxPos, wishSelectBoxPos, dt, 100.0)
    selectBoxPos = approach(selectBoxPos, wishSelectBoxPos, dt, 0.06)

    if (get_key_press(VK_RETURN) || get_key_press(VK_SPACE)) && fadeout <= 0.0
        if is_file_exists(deref(menuItems[nextIndex]).source)
            fadeoutTrend = 1.0

    fadeout = saturate(fadeout + dt * fadeoutTrend * 8.0)

    if fadeout >= 1.0 && fadeoutTrend > 0.0
        dasbox_execute(deref(menuItems[nextIndex]).source)


[export]
def draw()
    background_draw()
    set_font_size(get_screen_height() / 22)

    let selectorAlpha = 1.0 - clamp(abs((selectBoxPos - SELECT_Y) * 0.02), 0.0, 1.0)
    fill_rect(30.0, SELECT_Y, 640.0, LIST_LINE_HEIGHT, make_color(1.0, 0.1 * selectorAlpha))

    for s, i in menuItems, range(999)
        text_out(60.0, SELECT_Y + 9.0 + (float(i) - scrollPos) * LIST_LINE_HEIGHT,
                deref(s).name, make_color(1.0, i == nextIndex ? 1.0 : 0.4))

    set_font_size(get_screen_height() / 42)

    for s, i in deref(menuItems[menuIndex]).description, range(999)
        text_out(700, 324 + i * DESCRIPTION_LINE_HEIGHT, s, make_color(1.0, 0.7 * itemAlpha))

    deref(menuItems[menuIndex]).image |> draw_image(700, 27, make_color(1.0, itemAlpha))

    if (fadeout > 0.0)
        fill_rect(0, 0, get_screen_width(), get_screen_height(), make_color(0.0, fadeout))

