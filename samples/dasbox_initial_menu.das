include daslib/internal_menu_background.das
require fio

let
    SELECT_Y = 328.0
    LIST_LINE_HEIGHT = 60.0
    DESCRIPTION_LINE_HEIGHT = 23


let minimal_sample = "require daslib/media
// options debugger            // uncomment for debug in VS Code
// require daslib/debug        // uncomment for debug in VS Code


// 'initialize' runs once when game starts and every hot-reload
[export]
def initialize
    set_window_title(\"__TITLE__\")
    return

// this function is called to update game data,
// dt - time elapsed since the previous update (in seconds)
[export]
def act(dt: float)
    if get_key(VK_ESCAPE)
        schedule_quit_game()
    return

// this function will be called every frame after 'act',
// you can draw anything in this function
[export]
def draw
    return
"

let vscode_settings = "\{
   // path to dasbox.exe
   \"dascript.compiler\": \"{get_dasbox_exe_path()}\",

   \"dascript.server.args\": [
        \"$\{file\}\",
        \"x\",
        \"-main\",
        \"main\",
        \"--trust\",
        \"--dasbox-console\",
        \"--\",
        \"--port\",
        \"$\{port\}\",
        \"--project\",
        \"plugin.das_project\"
    ],
    \"dascript.project.roots\": [
    ],
    \"dascript.policies.no_global_variables\": false,
    \"dascript.policies.fail_on_lack_of_aot_export\": false,
    \"dascript.project.scanWorkspace\": true,
 
    // additional settings
    \"editor.semanticHighlighting.enabled\": true,
    \"editor.semanticTokenColorCustomizations\": \{
        \"rules\": \{
            \"modifier\": \"#58a16f\"
        \}
    \},
\}
"



struct MenuItem
    name: string
    source: string
    editorDir: string
    clipboardText: string
    newApplication: bool
    image: ImageHandle const
    description: array<string>


var
    menuIndex: int
    nextIndex: int
    scrollAccum = 0.0
    itemAlpha = 0.0
    itemAlphaTrend = 1.0
    scrollPos = 0.0
    wishScrollPos = 0.0
    selectBoxPos = SELECT_Y
    wishSelectBoxPos = SELECT_Y
    fadeout = 0.0
    fadeoutTrend = -1.0
    isMouseOnItem = false

    enteringName = false
    enteredName: array<uint8>
    cursorBlinkTime = 0.0
    enteredSymbol: uint = 0u
    isNameValid = false
    jumpToName: string = ""

    menuItems: array<MenuItem>


def find_projects_dir(): string
    var d = get_dasbox_initial_dir() + "/projects"
    if is_file_exists(d)
        return d
    d = get_dasbox_initial_dir() + "/../projects"
    if is_file_exists(d)
        return d
    return ""


def find_samples_dir(): string
    var d = get_dasbox_initial_dir() + "/samples"
    if is_file_exists(d)
        return d
    d = get_dasbox_initial_dir() + "/../samples"
    if is_file_exists(d)
        return d
    return ""


def check_and_create_vs_code_settings(editor_dir: string)
    if !is_file_exists(editor_dir + "/.vscode")
        mkdir(editor_dir + "/.vscode")
    if !is_file_exists(editor_dir + "/.vscode/settings.json")
        fopen(editor_dir + "/.vscode/settings.json", "wt") <| $(f)
            if f != null
                fwrite(f, vscode_settings)


def create_project(name: string): bool
    var result = false
    var d = find_projects_dir()
    if empty(d)
        mkdir(get_dasbox_initial_dir() + "/projects")
    d = find_projects_dir()

    let fsName = replace(name, " ", "_")

    if empty(d)
        panic("Cannot create direcory 'projects'")
    if is_file_exists(d + "/" + fsName)
        panic("Cannot create project '{name}'. Directory is already exists")
    mkdir(d + "/" + fsName)
    fopen(d + "/" + fsName + "/" + to_lower(fsName) + "_main.das", "wt") <| $(f)
        if f != null
            fwrite(f, replace(minimal_sample, "__TITLE__", name))
            result = true

    check_and_create_vs_code_settings(d + "/" + fsName)

    return result


def add_projects_to_menu()
    let atdir = find_projects_dir()
    if empty(atdir)
        return

    dir(atdir) <| $(fname)
        let dirName = atdir + "/" + fname
        var st : FStat
        if stat(dirName, st)
            if st.is_dir && fname != "." && fname != ".."
                menuItems |> emplace(
                    [[MenuItem
                          name = replace(fname, "_", " "),
                          clipboardText = dirName,
                          editorDir = dirName,
                          source = "[find]",
                          description <- [{string[]
                              "{dirName}";
                              "Enter - Execute.";
                              "Ctrl+Enter - Open in 'Visual Studio Code' and execute."
                              }]
                    ]])


def fill_menu_items()
    let samples_dir = find_samples_dir()

    menuItems |> clear()
    menuItems |> emplace(
        [[MenuItem name = "About",
                  clipboardText = "https://github.com/imp5imp5/dasbox",
              description <- [{string[]
                  "Dasbox {get_dasbox_version()}";
                  "Build date: {get_dasbox_build_date()}";
                  "https://github.com/imp5imp5/dasbox"
                  }]
        ]])

    menuItems |> emplace(
        [[MenuItem name = "New Application",
          clipboardText = minimal_sample,
          newApplication = true,
          description <- [{string[]
              "Press Enter to create new project.";
              "Press Ctrl+C to copy code of the minimal sample to clipboard.";
              "Execution: run from command line \"dasbox.exe <your_app.das>\""
              }]
        ]])

    add_projects_to_menu()

    menuItems |> emplace(
        [[MenuItem name = "Flappy Box",
          image <- create_managed_image("flappy_box/_preview.png"),
          source = "flappy_box/flappy_box.das",
          editorDir = empty(samples_dir) ? "" : samples_dir + "/flappy_box",
          description <- [{string[]
              "This example shows how to display graphical primitives";
              "and how to handle the simplest keyboard input.";
              "Enter - Execute.";
              "Ctrl+Enter - Open sample folder in 'Visual Studio Code' and execute.";
              "";
              "Controls:";
              "Space - Jump";
              "Esc - Return to this menu";
              "";
              "Source: samples/flappy_box/flappy_box.das"
              }]
        ]])

    menuItems |> emplace(
        [[MenuItem name = "Space, Rocks and Lasers",
          image <- create_managed_image("space_rocks_and_lasers/_preview.png"),
          source = "space_rocks_and_lasers/rocks_and_lasers.das",
          editorDir = empty(samples_dir) ? "" : samples_dir + "/space_rocks_and_lasers",
          description <- [{string[]
              "This example shows how to display graphical primitives";
              "and the simplest keyboard input.";
              "Enter - Execute.";
              "Ctrl+Enter - Open sample folder in 'Visual Studio Code' and execute.";
              "";
              "Controls:";
              "Arrow Keys - Move";
              "Space - Fire";
              "Esc - Return to this menu";
              "";
              "Source: samples/space_rocks_and_lasers/rocks_and_lasers.das"
              }]
        ]])

    menuItems |> emplace(
        [[MenuItem name = "Keyboard Input",
          image <- create_managed_image("controls/_preview_keyboard.png"),
          source = "controls/keyboard_input_demo.das",
          editorDir = empty(samples_dir) ? "" : samples_dir + "/controls",
          description <- [{string[]
              "Example of functions for processing keyboard input.";
              "Enter - Execute.";
              "Ctrl+Enter - Open sample folder in 'Visual Studio Code' and execute.";
              "";
              "Controls:";
              "Esc - Return to this menu";
              "";
              "Source: samples/controls/keyboard_input_demo.das"
              }]
        ]])

    menuItems |> emplace(
        [[MenuItem name = "Mouse Input",
          image <- create_managed_image("controls/_preview_mouse.png"),
          source = "controls/mouse_input_demo.das",
          editorDir = empty(samples_dir) ? "" : samples_dir + "/controls",
          description <- [{string[]
              "Example of functions for processing mouse input.";
              "Enter - Execute.";
              "Ctrl+Enter - Open sample folder in 'Visual Studio Code' and execute.";
              "";
              "Controls:";
              "Esc - Return to this menu";
              "";
              "Source: samples/controls/mouse_input_demo.das"
              }]
        ]])

    menuItems |> emplace(
        [[MenuItem name = "Gamepad Input",
          image <- create_managed_image("controls/_preview_gamepad.png"),
          source = "controls/gamepad_input_demo.das",
          editorDir = empty(samples_dir) ? "" : samples_dir + "/controls",
          description <- [{string[]
              "Example of functions for processing gampad input.";
              "Enter - Execute.";
              "Ctrl+Enter - Open sample folder in 'Visual Studio Code' and execute.";
              "";
              "Controls:";
              "Esc - Return to this menu";
              "";
              "Source: samples/controls/gamepad_input_demo.das"
              }]
        ]])

    menuItems |> emplace(
        [[MenuItem name = "Graphics",
          image <- create_managed_image("graphics/_preview_graphics.png"),
          source = "graphics/graphics_demo.das",
          editorDir = empty(samples_dir) ? "" : samples_dir + "/graphics",
          description <- [{string[]
              "Simple graphics primitives and text.";
              "Enter - Execute.";
              "Ctrl+Enter - Open sample folder in 'Visual Studio Code' and execute.";
              "";
              "Controls:";
              "Esc - Return to this menu";
              "";
              "Source: samples/graphics/graphics_demo.das"
              }]
        ]])

    menuItems |> emplace(
        [[MenuItem name = "Images",
          image <- create_managed_image("images/_preview_images.png"),
          source = "images/images_demo.das",
          editorDir = empty(samples_dir) ? "" : samples_dir + "/images",
          description <- [{string[]
              "How to work with 'Image' and 'ImageHandle'.";
              "Enter - Execute.";
              "Ctrl+Enter - Open sample folder in 'Visual Studio Code' and execute.";
              "";
              "Controls:";
              "Esc - Return to this menu";
              "";
              "Source: samples/images/images_demo.das"
              }]
        ]])

    menuItems |> emplace(
        [[MenuItem name = "Custom Fonts",
          image <- create_managed_image("custom_fonts/_preview_custom_fonts.png"),
          source = "custom_fonts/custom_fonts.das",
          editorDir = empty(samples_dir) ? "" : samples_dir + "/custom_fonts",
          description <- [{string[]
              "How to load custom fonts.";
              "Enter - Execute.";
              "Ctrl+Enter - Open sample folder in 'Visual Studio Code' and execute.";
              "";
              "Controls:";
              "Esc - Return to this menu";
              "";
              "Source: samples/custom_fonts/custom_fonts.das"
              }]
        ]])

    menuItems |> emplace(
        [[MenuItem name = "Will-o'-Wisp",
          image <- create_managed_image("will_o_wisp/_preview.png"),
          source = "will_o_wisp/will_o_wisp.das",
          editorDir = empty(samples_dir) ? "" : samples_dir + "/will_o_wisp",
          description <- [{string[]
              "Alpha blend, gradients, triangle strip.";
              "Enter - Execute.";
              "Ctrl+Enter - Open sample folder in 'Visual Studio Code' and execute.";
              "";
              "Controls:";
              "Esc - Return to this menu";
              "";
              "Source: samples/will_o_wisp/will_o_wisp.das"
              }]
        ]])

    menuItems |> emplace(
        [[MenuItem name = "Sound",
          image <- create_managed_image("sound/_preview_sound.png"),
          source = "sound/sound_demo.das",
          editorDir = empty(samples_dir) ? "" : samples_dir + "/sound",
          description <- [{string[]
              "How to work with 'PcmSound'.";
              "Enter - Execute.";
              "Ctrl+Enter - Open sample folder in 'Visual Studio Code' and execute.";
              "";
              "Controls:";
              "Esc - Return to this menu";
              "";
              "Source: samples/sound/sound_demo.das"
              }]
        ]])

    menuItems |> emplace(
        [[MenuItem name = "Basic ECS Demo",
          image <- create_managed_image("ecs/_preview_basic.png"),
          source = "ecs/decs_demo_basic.das",
          editorDir = empty(samples_dir) ? "" : samples_dir + "/ecs",
          description <- [{string[]
              "Entity Component System 'DECS'.";
              "Enter - Execute.";
              "Ctrl+Enter - Open sample folder in 'Visual Studio Code' and execute.";
              "";
              "Controls:";
              "Esc - Return to this menu";
              "Click - Create new particle";
              "";
              "Source: samples/ecs/decs_demo_basic.das"
              }]
        ]])


def recreate_menu()

    fill_menu_items()
    let saved = local_storage_get("dasbox_initial_menu/menuIndex")
    var s = empty(saved) ? 1 : int(saved)

    if !empty(jumpToName)
        for i, item in range(9999), menuItems
            if item.name == jumpToName && i > 0
                jumpToName = ""
                s = i
                break

    menuIndex = clamp(s, 0, length(menuItems) - 1)
    nextIndex = menuIndex
    local_storage_set("dasbox_initial_menu/menuIndex", string(nextIndex))
    scrollPos = float(menuIndex)
    wishScrollPos = scrollPos
    if menuIndex != 1
        fadeout = 1.0
        fadeoutTrend = -1.0



[export]
def initialize()
    background_initialize()
    recreate_menu()


def update_select_item(dt: float)
    let ctrl = get_key(VK_LCONTROL) || get_key(VK_RCONTROL)
    scrollAccum -= get_mouse_scroll_delta()
    if get_key_press(VK_DOWN)
        scrollAccum += 1.0
    if get_key_press(VK_UP)
        scrollAccum -= 1.0
    if get_key_press(VK_NEXT)
        scrollAccum += get_key(VK_LCONTROL) || get_key(VK_RCONTROL) ? 99999.0 : 5.0
    if get_key_press(VK_PRIOR)
        scrollAccum -= get_key(VK_LCONTROL) || get_key(VK_RCONTROL) ? 99999.0 : 5.0
    if get_key_press(VK_END)
        scrollAccum += 99999.0
    if get_key_press(VK_HOME)
        scrollAccum -= 99999.0

    if ctrl && get_key_press(VK_C) && length(menuItems[menuIndex].clipboardText) > 0
        set_clipboard_text(menuItems[menuIndex].clipboardText)

    if abs(scrollAccum) >= 1.0 && fadeout <= 0.0
        let next = clamp(nextIndex + int(scrollAccum), 0, length(menuItems) - 1)
        scrollAccum -= float(int(scrollAccum))
        if next != menuIndex
            nextIndex = next
            local_storage_set("dasbox_initial_menu/menuIndex", string(nextIndex))
            itemAlphaTrend = -1.0
            wishScrollPos = float(nextIndex)

    if (get_key_press(VK_RETURN) || get_key_press(VK_SPACE) ||
            (get_mouse_button_up(MB_LEFT) && isMouseOnItem)) && fadeout <= 0.0
        let editorDir = menuItems[nextIndex].editorDir
        var src = menuItems[nextIndex].source
        if !empty(src) && (src == "[find]" || !is_file_exists(src))
            menuItems[nextIndex].source = dasbox_find_main_das_file_in_directory(editorDir)

        if ctrl && !empty(editorDir)
            check_and_create_vs_code_settings(editorDir)
            dasbox_execute_editor(editorDir, menuItems[nextIndex].source)

        if menuItems[nextIndex].newApplication
            enteringName = true
            clear(enteredName)
        if is_file_exists(menuItems[nextIndex].source)
            fadeoutTrend = 1.0


def update_enter_name(dt: float)
    cursorBlinkTime -= dt
    if cursorBlinkTime < 0.0
        cursorBlinkTime += 0.5

    if get_key_press(VK_ESCAPE) || (get_key_press(VK_RETURN) && empty(enteredName))
        enteringName = false
        return

    if get_key_press(VK_RETURN) && isNameValid
        enteringName = false
        if create_project(string(enteredName))
            jumpToName = replace(string(enteredName), "_", " ")
            recreate_menu()
        return


    let code = enteredSymbol
    if code != 0u
        cursorBlinkTime = 0.5
        if code == 0x08
            if length(enteredName) > 0
                enteredName |> resize(length(enteredName) - 1)
        elif length(enteredName) < 32
            if ((code >= uint('A') && code <= uint('Z')) || (code >= uint('a') && code <= uint('z')) ||
                (code >= uint('0') && code <= uint('9')) || code == uint('-') || code == uint('_') || code == uint(' '))
                enteredName |> push(uint8(code))
            elif code > 0x20
                enteredName |> push(uint8('?'))

    isNameValid = enteredName |> find_index(uint8('?')) < 0



[export]
def act(dt: float)
    if (get_time_after_start() > 3600.0)
        reset_time_after_start()

    enteredSymbol = fetch_entered_symbol()

    if enteringName
        update_enter_name(dt)
    else
        update_select_item(dt)

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


    fadeout = saturate(fadeout + dt * fadeoutTrend * 8.0)

    if fadeout >= 1.0 && fadeoutTrend > 0.0
        dasbox_execute(menuItems[nextIndex].source)


[export]
def draw()
    background_draw()
    set_font_name("sans")
    set_font_size(get_screen_height() / 22)

    var selectorAlpha = 1.0 - clamp(abs((selectBoxPos - SELECT_Y) * 0.02), 0.0, 1.0)
    isMouseOnItem = false
    if !enteringName
        let mouse = get_mouse_position()
        if mouse.x > 30.0 && mouse.y <= 30.0 + 640.0 && mouse.y > SELECT_Y && mouse.y < SELECT_Y + LIST_LINE_HEIGHT
            isMouseOnItem = true
            selectorAlpha *= 1.3

    fill_rect(30.0, SELECT_Y, 640.0, LIST_LINE_HEIGHT, make_color(1.0, 0.1 * selectorAlpha))

    for s, i in menuItems, range(99999)
        text_out(60.0, SELECT_Y + 9.0 + (float(i) - scrollPos) * LIST_LINE_HEIGHT,
                s.name, make_color(1.0, i == nextIndex ? 1.0 : 0.4))

    if enteringName
        set_font_size(get_screen_height() / 26)
        set_font_name("mono")
        fill_rect(700, 372, 560, 2, make_color(1.0, 0.1 * selectorAlpha))
        if empty(enteredName)
            text_out(700, 338, "Enter name", make_color(1.0, 0.3 * itemAlpha))
        text_out(700, 338, string(enteredName) + (cursorBlinkTime > 0.25 ? "_" : ""),
                isNameValid ? make_color(1.0, 0.95 * itemAlpha) : make_color(1.0, 0.4, 0.3, 0.95 * itemAlpha))
    else
        set_font_size(get_screen_height() / 42)
        for s, i in menuItems[menuIndex].description, range(99999)
            text_out(700, 324 + i * DESCRIPTION_LINE_HEIGHT, s, make_color(1.0, 0.7 * itemAlpha))

    if (menuItems[menuIndex].image.width > 1)
        menuItems[menuIndex].image |> draw_image(700, 27, make_color(1.0, itemAlpha))

    if (fadeout > 0.0)
        fill_rect(0, 0, get_screen_width(), get_screen_height(), make_color(0.0, fadeout))

