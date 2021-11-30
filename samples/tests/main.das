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
    // finish_tests(testsRes, startTime)
    // text_out(200, 200, "hello world!", make_color(0.0, 0.0, 1.0, 1.0))


[test]
def init_test1_resolution(t: T?)
    set_resolution(1280, 720)
    t |> run("resolution_height_test") <| @@ ( t : T? )
        t |> equal(get_screen_height(), 720)
    t |> run("resolution_width_test") <| @@ ( t : T? )
        t |> equal(get_screen_width(), 1280)
    set_resolution(800, 800)
    t |> equal(get_screen_height(), get_screen_width())


[test]
def init_test2_font(t: T?)
    t |> run("font_name_test") <| @@ ( t : T? )
        set_font_name("mono")
        t |> equal(get_current_font_name(), "mono")
    t |> run("font_size_test") <| @@ ( t : T? )
        set_font_size(10)
        t |> equal(get_current_font_size(), 10)


[test]
def act_test1_make_color(t: T?)
    t |> run("rgb_float") <| @@ ( t : T? )
        var my_color <- make_color(1.0, 0.0, 0.5)
        t |> equal(my_color, 0xFFFF0080)
    t |> run("rgba_float") <| @@ ( t : T? ) 
        var my_color <- make_color(1.0, 0.0, 0.5, 0.0)
        t |> equal(my_color, 0x00FF0080)
    t |> run("float3") <| @@ ( t : T? )
        var my_color = make_color(float3(0.0, 1.0, 0.5))
        t |> equal(my_color, 0xFF00FF80)
    t |> run("float4") <| @@ ( t : T? )
        var my_color = make_color(float4(0.0, 1.0, 0.5, 0.0))
        t |> equal(my_color, 0x0000FF80)
    t |> run("brightness") <| @@ ( t : T? )
        var my_color <- make_color(1.0)
        t |> equal(my_color, 0xFFFFFFFF)
    t |> run("brightness_alpha") <| @@ ( t : T? )
        var my_color <- make_color(1.0, 0.0)
        t |> equal(my_color, 0x00FFFFFF)
    t |> run("rgb_int") <| @@ ( t : T? )
        var my_color <- make_color32(255, 0, 128)
        t |> equal(my_color, 0xFFFF0080)
    t |> run("rgb_uint") <| @@ ( t : T? )
        var my_color <- make_color32(255u, 0u, 128u)
        t |> equal(my_color, 0xFFFF0080)
    t |> run("rgba_int") <| @@ ( t : T? )
        var my_color <- make_color32(255, 0, 128, 10)
        t |> equal(my_color, 0x0AFF0080)
    t |> run("rgba_uint") <| @@ ( t : T? )
        var my_color <- make_color32(255u, 0u, 128u, 10u)
        t |> equal(my_color, 0x0AFF0080)
[test]
def draw_test1_pixel(t: T?)
    var img <- create_image(2, 2)
    var yellow_color <- make_color(1.0, 1.0, 0.0)
    img |> set_pixel(0, 0, yellow_color)
    t |> equal(img |> get_pixel(0, 0), yellow_color)












    




// options debugger
// require daslib/debug
