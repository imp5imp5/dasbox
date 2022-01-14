require daslib/media

options rtti

require ast
require dastest/suite
require dastest/testing_boost
require dastest/dastest
require dastest/log

var
    startTime: int64
    testsRes: SuiteResult


def self_test(step_name: string)
    var ctx <- createSuiteCtx()
    ctx.testNames <- [{auto step_name}]
    let res = test_module(*get_this_module(this_program()), this_context(), ctx)
    testsRes += res
    print("{step_name} {to_string(res)}\n\n")


[export]
def initialize
    log::init_log() // initialize dastest log
    startTime = ref_time_ticks()
    self_test("init")

[export]
def act(dt: float)
    self_test("act")

[export]
def draw()
    self_test("draw")
 //   if ref_time_ticks() - startTime > 100000000l
    finish_tests(testsRes, startTime)




[test]
def init_test_resolution(t: T?)
    let heights = [[auto 720; 1080; 300]]
    let widths = [[auto 1280; 1920; 400]]
    let testNames = [[auto "HD"; "full HD"; "400x300"]]

    for h, w, testName in heights, widths, testNames
        t |> run(testName) <| @ ( t : T? )
            set_resolution(w, h)
            t |> equal(h , get_screen_height())
            t |> equal(w, get_screen_width())


    t |> run("800x800") <| @ ( t : T? )
        let x = 800
        set_resolution(x, x)
        t |> equal(get_screen_width(), get_screen_height())


[test]
def init_test_font_type(t: T?)
    for fontName in [[auto "mono"; "sans"; "Montserrat-Regular"; "Aqum"; "Shadower"]]
        var font = fontName
        if font != "mono" && font != "sans"
            font = "custom_fonts/{font}.ttf"

        set_font_name(font)
        t |> equal(font, get_current_font_name())


[test]
def act_test_make_color32(t: T?)
    t |> run("rgba_int") <| @ ( t : T? )
        for color in range(255)
            t |> equal(construct_color_int(color, 0, 0, 0), make_color32(color, 0, 0, 0))
            t |> equal(construct_color_int(0, color, 0, 0), make_color32(0, color, 0, 0))
            t |> equal(construct_color_int(0, 0, color, 0), make_color32(0, 0, color, 0))
            t |> equal(construct_color_int(0, 0, 0, color), make_color32(0, 0, 0, color))

    t |> run("rgb_int") <| @ ( t : T? )
        for color in range(255)
            t |> equal(construct_color_int(color, 0, 0), make_color32(color, 0, 0))
            t |> equal(construct_color_int(0, color, 0), make_color32(0, color, 0))
            t |> equal(construct_color_int(0, 0, color), make_color32(0, 0, color))

    t |> run("rgba_uint") <| @ ( t : T? )
        for color in range(255)
            t |> equal(construct_color_int(color, 0, 0, 0), make_color32(uint(color), 0U, 0U, 0U))
            t |> equal(construct_color_int(0, color, 0, 0), make_color32(0U, uint(color), 0U, 0U))
            t |> equal(construct_color_int(0, 0, color, 0), make_color32(0U, 0U, uint(color), 0U))
            t |> equal(construct_color_int(0, 0, 0, color), make_color32(0U, 0U, 0U, uint(color)))

    t |> run("rgb_uint") <| @ ( t : T? )
        for color in range(255)
            t |> equal(construct_color_int(color, 0, 0), make_color32(uint(color), 0U, 0U))
            t |> equal(construct_color_int(0, color, 0), make_color32(0U, uint(color), 0U))
            t |> equal(construct_color_int(0, 0, color), make_color32(0U, 0U, uint(color)))


def construct_color_int(red, green, blue, alpha = 255)
    return uint(alpha) << 24u | uint(red) << 16U | uint(green) << 8U | uint(blue)

def approxEqual(a; b; msg = ""; max_diff = 1e-5f; max_rel_diff = FLT_EPSILON): bool
    let diff = abs(a - b)
    if diff < max_diff
        return true
    return diff < (max(abs(a), abs(b)) * max_rel_diff)

[test]
def act_test_normalize_angle(t: T?)
    t |> run("-PI:PI") <| @@ ( t : T? )
        for a in range(-99, 100)
            let angle = float(a) * PI / 100.0
            t |> success(approxEqual(angle, normalize_angle(angle)))

    t |> run("-3PI:-PI") <| @@ ( t : T? )
        for a in range(-99, 100)
            let angle = float(a) * PI / 100.0
            t |> success(approxEqual(angle, normalize_angle(angle - 2.0 * PI)))

    t |> run("PI:3PI") <| @@ ( t : T? )
        for a in range(-99, 100)
            let angle = float(a) * PI / 100.0
            t |> success(approxEqual(angle, normalize_angle(angle + 2.0 * PI)))

    t |> run("2 PI N") <| @@ ( t : T? )
        for N in range(-10, 11)
            t |> success(approxEqual(0.0, normalize_angle(2.0 * PI * float(N))))


[test]
def draw_test_img_pixel(t: T?)
    let h = 10
    let w = 20
    var img <- create_image(w, h)

    for i in range(w)
        for j in range(h)
            let color = make_color(float(i) / float(w), float(j) / float(h))
            img |> set_pixel(i, j) <| color
            t |> equal(color, img |> get_pixel(i, j))

    delete img

// options debugger
// require daslib/debug
