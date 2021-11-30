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
def init_test1_resolution(t: T?)
    var heights = [[auto 720; 1080; 300]]
    var widths = [[auto 1280; 1920; 400]]
    var tests = [[auto "HD"; "full HD"; "400x300"]]

    for my_height, my_width, my_test in heights, widths, tests
        t |> run(my_test) <| @ ( t : T? )
            set_resolution(my_width, my_height)
            t |> equal(get_screen_height(), my_height)
            t |> equal(get_screen_width(), my_width)

   
    t |> run("800x800") <| @ ( t : T? )
        let x = 800
        set_resolution(x, x)
        t |> equal(get_screen_width(), get_screen_height())
    

[test]
def init_test2_font(t: T?)
    var font_names = [[auto "mono"; "sans"; "Montserrat-Regular"; "Aqum"; "Shadower"]]
    for font_name in font_names
        var font := font_name
        if font != "mono" && font != "sans"
            font = "custom_fonts/{font}.ttf"
        t |> run("{font_name} font type") <| @ ( t : T? )
            set_font_name(font)
            t |> equal(get_current_font_name(), font)

    
    for size in range(10, 21)
        t |> run("{size} font size") <| @ ( t : T? )
            set_font_size(size)
            t |> equal(get_current_font_size(), size)


[test]
def init_test3_make_color32(t: T?)
    t |> run("rgba_int") <| @ ( t : T? )
        for color in range(255)
            t |> equal(make_color32(color, 0, 0, 0), construct_color_int(color, 0, 0, 0))
            t |> equal(make_color32(0, color, 0, 0), construct_color_int(0, color, 0, 0))
            t |> equal(make_color32(0, 0, color, 0), construct_color_int(0, 0, color, 0))
            t |> equal(make_color32(0, 0, 0, color), construct_color_int(0, 0, 0, color))

    t |> run("rgb_int") <| @ ( t : T? )
        for color in range(255)
            t |> equal(make_color32(color, 0, 0), construct_color_int(color, 0, 0))
            t |> equal(make_color32(0, color, 0), construct_color_int(0, color, 0))
            t |> equal(make_color32(0, 0, color), construct_color_int(0, 0, color))
    
    t |> run("rgba_uint") <| @ ( t : T? )
        for color in range(255)
            t |> equal(make_color32(uint(color), 0U, 0U, 0U), construct_color_int(color, 0, 0, 0))
            t |> equal(make_color32(0U, uint(color), 0U, 0U), construct_color_int(0, color, 0, 0))
            t |> equal(make_color32(0U, 0U, uint(color), 0U), construct_color_int(0, 0, color, 0))
            t |> equal(make_color32(0U, 0U, 0U, uint(color)), construct_color_int(0, 0, 0, color))

    t |> run("rgb_uint") <| @ ( t : T? )
        for color in range(255)
            t |> equal(make_color32(uint(color), 0U, 0U), construct_color_int(color, 0, 0))
            t |> equal(make_color32(0U, uint(color), 0U), construct_color_int(0, color, 0))
            t |> equal(make_color32(0U, 0U, uint(color)), construct_color_int(0, 0, color))


def construct_color_int(red, green, blue, alpha = 255)
    var res = 0U
    res += uint(blue)
    res += uint(green) * uint(pow(16.0, 2.0))
    res += uint(red) * uint(pow(16.0, 4.0))
    res += uint(alpha) * uint(pow(16.0, 6.0))
    return res

[test]
def draw_test2_pixel(t: T?)
    var my_height = 10
    var my_width = 20
    var img <- create_image(my_width, my_height)

    for i in range(my_width)
        for j in range(my_height)
            let my_color = make_color(float(i) / float(my_width), float(j) / float(my_height))
            img |> set_pixel(i, j) <| my_color
            t |> equal(my_color, img |> get_pixel(i, j))
    
    delete img
    
    


    














    




// options debugger
// require daslib/debug
