require daslib/media


var
    background_gradient: Image
    background_godrays: Image


def background_initialize()
    let buf <- [{
        for i in range(64 * 64);
        make_color(
            0.5 * sin(float(i / 64) * 0.03 + 0.1),
            0.8 * sin(float(i % 64) * 0.03 + 0.2),
            sin(float(i / 64 + i % 64) * 0.03 + 0.3),
            1.0 - length(float2(float((i / 64 - 32)) * 0.035, float((i % 64) - 32) * 0.035))
            )
        }]
    background_gradient <- create_image(64, 64, buf)
    background_gradient |> set_image_smooth(true)

    let grad <- [{
        for i in range(64 * 64);
        make_color(1.0, 1.0, 1.0, float(i / 64) * (1.0 / 64.0) -
            float(((i * (23459 + i)) ^ 345) & 15) / 16.0 * 0.2)
        }]
    background_godrays <- create_image(64, 64, grad)
    background_godrays |> set_image_smooth(true)


def draw_image_sqewed(img: Image& const; x, y: float; color: uint; sz: float2; skew: float)
    img |> draw_quad(
        float2(x, y),
        float2(x + skew, y + sz.y),
        float2(x + sz.x + skew, y + sz.y),
        float2(x + sz.x, y),
        color)


def background_draw()
    let sw = float(get_screen_width())
    let sh = float(get_screen_height())
    let t = get_time_after_start() * 0.03 - 1200.0
    let bck_color = make_color(
        abs(sin(t)), abs(sin(t) + 0.25), abs(sin(t) + 0.5));
    let fore_color = make_color(
        abs(sin(t * 0.6)), abs(sin(t * 0.7) + 0.25), abs(sin(t * 0.8) + 0.5), 0.32);

    background_gradient |> draw_image(0, 0, bck_color, get_screen_width())
    enable_alpha_blend()
    background_godrays |> draw_image_sqewed(0.0, -sh * 0.2, make_color(0.0, 0.0, 0.3, 0.7),
        float2((2.5 + sin(t * 2.0)) * sw, sh * 1.4), -400.0)
    background_godrays |> draw_image_sqewed(0.0, -sh * 0.2, make_color(0.0, 0.1, 0.1, 0.6),
        float2((2.9 + sin(t * 0.6)) * sw, sh * 1.4), -240.0)

    var seed: int4 = int4(20, 50, 60, 70)

    for layer in range(10)
        let la = 12 - layer
        for i in range(50)
            let offset = -float(la) * t * 0.05
            var pos = float2(random_float(seed), random_float(seed))
            pos.y += offset
            pos.x += sin(t + float(layer) * 0.2)
            pos.x = pos.x % 1.0
            pos.y = pos.y % 1.0
            pos.y += 0.1 * sin(t * 2.0 + float(i) * 0.6)
            pos = pos * float2(1.4, 1.4) - float2(0.2, 0.2)
            pos.x *= sw
            pos.y *= sh
            let scale = 64.0 / float(layer + 2)
            background_gradient |> draw_image(pos.x - 5.0, pos.y, 0x1F000000, scale)
            background_gradient |> draw_image(pos.x, pos.y, fore_color, scale)

    background_godrays |> draw_image_sqewed(0.0, -sh * 0.2, make_color(0.0, 0.0, 0.05, 0.3),
        float2((2.8 + sin(t * 1.3)) * sw, sh * 1.4), -200.0)
