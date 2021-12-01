require daslib/media

var points: array<float2>
var points2: array<float2>
var image1: ImageHandle
var image2: ImageHandle
var texture: ImageHandle


[export]
def initialize()
    print("initialize\n")
    set_window_title("Graphics Demo")
    //set_antialiasing(4)

    image1 <- create_managed_image(24, 17,
        "...............x......x." +
        "...............xx....xx." +
        "...............xxx..xxx." +
        "..............xxxxxxxxxx" +
        "aa............xxx--xx--x" +
        "aa.....aaaaaa.xxx--xx--x" +
        "aa...aaaaaaaa.xxxxxxxxxx" +
        "aaa.aaaaaaaaa.xxxxxx.xxx" +
        ".aaaaaaaaaaaa.xxxxx...xx" +
        "..aaaaaaaaaaaa.xxxxxxxx." +
        "..aaaaaaaaaaaaa........." +
        "..aaaaaaaaaaaaaaaaaaa..." +
        "..aa...aa......aa..aa..." +
        "..aa...aa......aa..aa..." +
        ".......aa..........aa..." +
        ".......aa..........aa..." +
        "........................",

        {{ '.' => 0x000000;
           'a' => 0xFFAFA0;
           '-' => 0x0000F0;
           'x' => 0xFFFFFF
        }}
        )

    image2 <- create_managed_image(24, 17,
        "........................" +
        "...............x......x." +
        "...............xx....xx." +
        "...............xxx..xxx." +
        "..............xxxxxxxxxx" +
        "aa............xxx--xx--x" +
        "aa.....aaaaaa.xxx--xx--x" +
        "aa...aaaaaaaa.xxxxxxxxxx" +
        "aaa.aaaaaaaaa.xxxxxx.xxx" +
        ".aaaaaaaaaaaa.xxxxx...xx" +
        "..aaaaaaaaaaaa.xxxxxxxx." +
        "..aaaaaaaaaaaaa........." +
        "..aaaaaaaaaaaaaaaaaaa..." +
        "..aa..aa.......aa..aa..." +
        "..aa..aa.......aa......." +
        "..aa...........aa......." +
        "..aa...........aa.......",

        {{ '.' => 0x000000;
           'a' => 0xFFAFA0;
           '-' => 0x0000F0;
           'x' => 0xFFFFFF
        }}
        )

    texture <- create_managed_image(16, 16)
    texture |> set_image_smooth(true)
    for y in range(16)
        for x in range(16)
            texture |> set_pixel(x, y,
                make_color(x % 2 == y % 2 ? 1.0 : 0.4))


[export]
def act(dt: float)
    if get_key(VK_ESCAPE)
        schedule_quit_game()

[export]
def draw()
    set_font_size(16)
    var y = 40
    var delta = 18.0 * sin(get_time_after_start() * 0.4)

    set_pixel(70, y + 9, 0xFFFF80)
    text_out(100, y, "set_pixel(x, y, color)", 0xF0F0F0)
    y += 50

    rect(60, y, 23, 18, 0xFFFF80)
    text_out(100, y, "rect(x, y, width, height, color)", 0xF0F0F0)
    y += 50

    fill_rect(60, y, 23, 18, 0xFFFF80)
    text_out(100, y, "fill_rect(x, y, width, height, color)", 0xF0F0F0)
    y += 50

    circle(70, y + 10, 14, 0xFFFF80)
    text_out(100, y, "circle(x, y, radius, color)", 0xF0F0F0)
    y += 50

    fill_circle(70, y + 10, 14, 0xFFFF80)
    text_out(100, y, "fill_circle(x, y, radius, color)", 0xF0F0F0)
    y += 50

    line(50, y + int(delta), 85, y + 10 - int(delta), 0xFFFF80)
    text_out(100, y, "line(x1, y1, x2, y2, color)", 0xF0F0F0)
    y += 50

    if empty(points)
        points |> push(float2(50.0, float(y)))
        points |> push(float2(60.0, float(y) - 3.0))
        points |> push(float2(78.0, float(y) + 20.0))
    polygon(points, 0xFFFF80)
    polygon([[auto float2(74.0, float(y)); float2(80.0, -1.0 + float(y)); float2(85.0, 20.0 + float(y))]], 0xFFFF80)
    text_out(100, y, "polygon(points, color)", 0xF0F0F0)
    y += 50

    if empty(points2)
        points2 |> push(float2(50.0, float(y)))
        points2 |> push(float2(60.0, float(y) - 3.0))
        points2 |> push(float2(78.0, float(y) + 20.0))
    fill_convex_polygon(points2, 0xFFFF80)
    fill_convex_polygon([[auto float2(74.0, float(y)); float2(80.0, -1.0 + float(y)); float2(85.0, 20.0 + float(y))]], 0xFFFF80)
    text_out(100, y, "fill_convex_polygon(points, color)", 0xF0F0F0)
    y += 50

    text_out(52, y, "Abc", 0xFFFF80)
    text_out(100, y, "text_out(x, y, text, color)", 0xF0F0F0)
    y += 50

    if (uint(get_time_after_start() * 7.0) & 1u) == 1u
        draw_image(image1, 55, y)
    else
        draw_image(image2, 55, y)

    text_out(100, y, "draw_image(image, x, y)", 0xF0F0F0)
    y += 50

    if (uint(get_time_after_start() * 7.0) & 1u) == 1u
        draw_image(image1, 35, y - 5, make_color(1.0), int2(image1.width * 2, image1.height * 2))
    else
        draw_image(image2, 35, y - 5, make_color(1.0), int2(image2.width * 2, image2.height * 2))

    text_out(100, y, "draw_image(image, x, y, color, size)", 0xF0F0F0)
    y += 50


    let scale = 1.6 / 2.0
    let center = float2(60, y + 10)
    let angle = get_time_after_start()
    let dirX = float2(cos(angle), -sin(angle)) * float(image1.width) * scale
    let dirY = float2(sin(angle), cos(angle)) * float(image1.height) * scale
    let p1 = center - dirX - dirY
    let p2 = center - dirX + dirY
    let p3 = center + dirX + dirY
    let p4 = center + dirX - dirY

    draw_quad(image1, p1, p2, p3, p4, 0xFFFFFFFF)
    text_out(100, y, "draw_quad(image, p1, p2, p3, p4, color)", 0xF0F0F0)
    y += 50



    y = 40

    let screen_coord <- [{auto float2(630, 35 + y); float2(624, y); float2(650, 10 + y); float2(654, y - 16); float2(678, y) }]
    let tex_coord <- [{auto float2(0, 0); float2(0, 8); float2(5, 0); float2(5, 8); float2(10, 8) }]
    texture |> draw_triangle_strip(screen_coord, tex_coord, 0xffffffff)
    text_out(700, y, "draw_triangle_strip(screen_coord, tex_coord, color)", 0xF0F0F0)
    y += 50

    let screen_coord2 <- [{auto float2(630, 35 + y); float2(624, y); float2(650, 10 + y); float2(654, y - 16); float2(678, y) }]
    let colors <- [{auto 0xFF00FF00; 0xFF00FF00; 0xFF00FFEE; 0xFFFA0FF3; 0xFFFFFFFF}]
    texture |> draw_triangle_strip(screen_coord2, tex_coord, colors)
    text_out(700, y, "draw_triangle_strip(screen_coord, tex_coord, colors)", 0xF0F0F0)
    y += 50

    enable_alpha_blend()
    fill_circle(660, y, 15, 0xFF8FF080)
    fill_circle(647, y + 10, 15, 0xA0FF0080)
    text_out(700, y, "enable_alpha_blend()", 0xFFF0F0F0)
    y += 50

    disable_alpha_blend()
    fill_circle(660, y, 15, 0xFF8FF080)
    fill_circle(647, y + 10, 15, 0xA0FF0080)
    text_out(700, y, "disable_alpha_blend()", 0xFFF0F0F0)
    y += 50

    let color_1 = make_color(0.5) // make_color(0.5, 0.3)
    fill_rect(630, y, 50, 20, color_1)
    text_out(700, y, "make_color(brightness)   make_color(brightness, alpha)", 0xF0F0F0)
    y += 50

    let color_2 = make_color(0.5, 0.7, 1.0) // make_color(0.5, 0.7, 1.0, 1.0)
    fill_rect(630, y, 50, 20, color_2)
    text_out(700, y, "make_color(r, g, b)      make_color(r, g, b, a)", 0xF0F0F0)
    y += 50

    let color_3 = make_color(float3(0.7, 0.3, 1.0)) // make_color(0.5, 0.7, 1.0, 1.0)
    fill_rect(630, y, 50, 20, color_3)
    text_out(700, y, "make_color(float3(...))  make_color(float4(...))", 0xF0F0F0)
    y += 50


    set_font_size(11)
    text_out(582, y - 8, "A", 0xFFFFFF)
    set_font_size(16)
    text_out(600, y - 8, "A", 0xFFFFFF)
    set_font_size(30)
    text_out(620, y - 8, "A", 0xFFFFFF)
    set_font_size(46)
    text_out(648, y - 8, "A", 0xFFFFFF)
    set_font_size(16)

    text_out(700, y, "set_font_size(size)", 0xF0F0F0)
    y += 50

    set_font_name("sans")
    text_out(700, y, "set_font_name(\"sans\")", 0xF0F0F0)
    y += 50

    set_font_name("mono")
    text_out(700, y, "set_font_name(\"mono\")", 0xF0F0F0)
    y += 50


    text_out(700, y, "get_screen_width() = {get_screen_width()}", 0xF0F0F0)
    y += 50

    text_out(700, y, "get_screen_height() = {get_screen_height()}", 0xF0F0F0)
    y += 50

    if get_key_down(VK_S)
        schedule_screenshot("screenshot.png")
