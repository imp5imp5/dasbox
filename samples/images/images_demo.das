require daslib/media

var image_tga: ImageHandle
var image_png: ImageHandle
var image_jpg: ImageHandle
var ball: ImageHandle
var ball2: ImageHandle

[export]
def initialize()
    print("initialize\n")
    set_window_title("Images Demo")

    image_tga <- create_managed_image("test.tga")
    if image_tga.width == 0
        print("Failed to load test.tga\n")

    image_png <- create_managed_image("test.png")
    image_jpg <- create_managed_image("test.jpg")
    ball <- create_managed_image("ball.png")
    make_image_color_transparent(ball, get_pixel(ball, 0, 0))
    ball2 := ball // clone
    //ball2 |> flip_image_x
    //ball2 |> flip_image_y
    delete ball


[export]
def act(dt: float)
    if get_key(VK_ESCAPE)
        schedule_quit_game()


[export]
def draw()
    draw_image(image_tga, 100, 30)
    draw_image(image_png, 280, 30)
    draw_image(image_jpg, 460, 30)
    text_out(640, 80, "create_managed_image(\"file_name\")", 0xFFF0F0F0)

    var top = get_screen_height() - 200
    for i in range(100)
        var t = float(i) * 0.01
        var color = make_color(t * t * t * float3(sin(t) * 0.2, sin(t * 80.0 + 1.4) * 1.0, sin(t + 1.0)))
        fill_rect(0, top + i * 2, get_screen_width(), 2, color)

    enable_alpha_blend()
    let size_x2 = int2(ball2.width * 2, ball2.height * 2)
    for iy in range(7)
        for ix in range(11)
            var x = float(ix) / 19.0 - 0.5
            var y = float(iy) / 7.0 - 0.5
            x += sin(get_time_after_start() + y * 5.0) * 0.05
            y += sin(get_time_after_start() + x * 15.0) * 0.1
            draw_image(ball2,
                get_screen_width() * 2 / 5 - int(x * float(get_screen_width()) * 0.7),
                get_screen_height() - 130 + int(y * float(get_screen_height()) * 0.2),
                0xFFFFFFFF,
                size_x2)

    disable_alpha_blend()
    var y = 200
    draw_image(ball2, 100, y, 0xFFFFFFFF, ball2.width * 2)
    draw_image_region(ball2, 100.0, float(y) + 100.0,
            float4(0.0, 8.0 + 8.0 * sin(get_time_after_start()), 16.0, 16.0), 0xFFFFFFFF, float2(64.0, 64.0))

    text_out(180, y, "image_b := image_a // clone image handle", 0xFFF0F0F0)
    y += 22
    text_out(180, y, "make_image_color_transparent(image, color)", 0xFFF0F0F0)
    y += 22
    text_out(180, y, "set_image_smooth(image, smooth)", 0xFFF0F0F0)
    y += 22
    text_out(180, y, "set_image_repeat(image, repeat)", 0xFFF0F0F0)
    y += 22
    text_out(180, y, "draw_image(image, x, y)", 0xFFF0F0F0)
    y += 22
    text_out(180, y, "draw_image(image, x, y, color)", 0xFFF0F0F0)
    y += 22
    text_out(180, y, "draw_image(image, x, y, color, size)", 0xFFF0F0F0)
    y += 22
    text_out(180, y, "draw_image_region(image, x, y, texture_rect, color, size)", 0xFFF0F0F0)
    y += 22
    text_out(180, y, "draw_image_transformed(image, x, y, texture_rect, color, size, angle, pivot_x, pivot_y)", 0xFFF0F0F0)
    y += 22
    text_out(180, y, "get_pixel(image, x, y)", 0xFFF0F0F0)
    y += 22
    text_out(180, y, "set_pixel(image, x, y, color)", 0xFFF0F0F0)
    y += 22
    text_out(180, y, "get_image_data(image, array&)", 0xFFF0F0F0)
    y += 22
    text_out(180, y, "set_image_data(image, array)", 0xFFF0F0F0)
    y += 22
    text_out(180, y, "delete image", 0xFFF0F0F0)
    y += 22


