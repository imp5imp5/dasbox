require daslib/media

// The easiest way to play sound:
//
// var
//     snd: SoundHandle <- create_managed_sound("my_sound.wav")
// ...
//     play_sound(snd)


var
    gong: SoundHandle
    musicLoop: SoundHandle
    handle: PlayingSoundHandle


///////////////////////////////  UI Components  ///////////////////////////////////

struct Slider
    name: string
    pos: float2
    size: float2
    minVal: float = 0.0
    maxVal: float = 1.0
    curVal: float = 0.0


struct Button
    name: string
    pos: float2
    size: float2


var currentControlName: string


def is_cursor_inside(s)
    if !empty(currentControlName)
        return s.name == currentControlName
    let p = get_mouse_position()
    return p.x >= s.pos.x && p.x <= s.pos.x + s.size.x && p.y >= s.pos.y && p.y <= s.pos.y + s.size.y


def update(var s: Slider &): bool
    let mdown = (get_mouse_button(MB_LEFT))
    if !mdown
        currentControlName = ""
    if mdown && is_cursor_inside(s)
        currentControlName = s.name
        s.curVal = clamp((get_mouse_position().x - s.pos.x) / s.size.x * (s.maxVal - s.minVal) + s.minVal, s.minVal, s.maxVal)
        return true
    return false


def render(s: Slider & const)
    text_out(s.pos.x, s.pos.y - 20.0, "{s.name} = {s.curVal}", 0xFFFFFF)
    fill_rect(s.pos.x, s.pos.y, s.size.x, s.size.y, currentControlName == s.name ? 0x309060 : 0x306060)
    fill_rect(s.pos.x + (s.curVal - s.minVal) / (s.maxVal - s.minVal) * s.size.x, s.pos.y, 3.0, s.size.y, 0xFFFFFF)


def update(var s: Button &): bool
    let mdown = (get_mouse_button(MB_LEFT))
    if !mdown
        currentControlName = ""
    if mdown && is_cursor_inside(s)
        currentControlName = s.name
        return true
    return false


def render(s: Button & const)
    fill_rect(s.pos.x, s.pos.y, s.size.x, s.size.y, currentControlName == s.name ? 0x808080 : 0xFFFFFF)
    text_out(s.pos.x + 20.0, s.pos.y + s.size.y * 0.5 - 10.0, "{s.name}", 0x0)


var
    buttonPlayGong <- [[Button name = "Play 'gong.wav'", pos = float2(50, 100), size = float2(270, 40) ]]
    buttonPlayMusic <- [[Button name = "Play 'music_loop.wav'", pos = float2(50, 200), size = float2(270, 40) ]]
    sliderVolume <- [[Slider name = "volume", pos = float2(50, 300), size = float2(400, 40), minVal = 0.0, maxVal = 1.0, curVal = 1.0 ]]
    sliderPitch <- [[Slider name = "pitch", pos = float2(50, 400), size = float2(400, 40), minVal = 0.1, maxVal = 4.0, curVal = 1.0 ]]
    sliderPos <- [[Slider name = "pos", pos = float2(50, 500), size = float2(400, 40), minVal = 0.0, maxVal = 1.0, curVal = 0.0 ]]

/////////////////////////////////////////////////////////////////////////////


[export]
def initialize
    set_window_title("Sound Demo")
    gong <- create_managed_sound("gong.wav")
    print("gong.channels = {gong.channels}\n")
    print("gong.frequency = {gong.frequency}\n")
    print("gong.samples = {gong.samples}\n")
    print("gong.duration = {gong.duration} sec\n\n")

    musicLoop <- create_managed_sound("music_loop.wav")
    print("musicLoop.channels = {musicLoop.channels}\n")
    print("musicLoop.frequency = {musicLoop.frequency}\n")
    print("musicLoop.samples = {musicLoop.samples}\n")
    print("musicLoop.duration = {musicLoop.duration} sec\n")

    sliderPos.maxVal = musicLoop.duration


[export]
def act(dt: float)
    if get_key(VK_ESCAPE)
        schedule_quit_game()

    if buttonPlayGong |> update() && get_mouse_button_down(MB_LEFT)
        gong |> play_sound()

    if buttonPlayMusic |> update() && get_mouse_button_down(MB_LEFT)
        if is_playing(handle)
            stop_sound(handle)
            buttonPlayMusic.name = "Play 'music_loop.wav'"
        else
            handle = musicLoop |> play_sound_loop(sliderVolume.curVal, sliderPitch.curVal)
            buttonPlayMusic.name = "Stop"


    if is_playing(handle)
        sliderPos.curVal = get_sound_play_pos(handle)

    if sliderVolume |> update() && is_playing(handle)
        set_sound_volume(handle, sliderVolume.curVal)

    if sliderPitch |> update() && is_playing(handle)
        set_sound_pitch(handle, sliderPitch.curVal)

    if sliderPos |> update() && is_playing(handle) && get_mouse_button_down(MB_LEFT)
        set_sound_play_pos(handle, sliderPos.curVal)



[export]
def draw

    sliderVolume |> render()
    sliderPitch |> render()
    sliderPos |> render()
    buttonPlayGong |> render()
    buttonPlayMusic |> render()

