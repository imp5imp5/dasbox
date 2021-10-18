require daslib/media

struct GrassMesh
    coord: array<float2>
    uv: array<float2>
    color: uint

var
    grass_leaf_gradient: Image
    sky_gradient: Image
    particle: Image
    m: array<GrassMesh>
    seed = int4(762, 22, 3, 4)


def smoothstep(x)
    let t = saturate(x)
    return t * t * (3.0 - 2.0 * t)


def rnd(from, to): float
    return float(from) + random_float(seed) * (float(to) - float(from))


def grass_leaf(var m: GrassMesh &; pos, dir_: float2; width_, buckling: float)
    var p = pos
    var dir = dir_
    var segments = 11
    var dirInc = float2(0.0, -dir.y * buckling * 10.0 / float(segments))

    m.coord |> push(p)
    m.uv |> push(float2(0, 0))

    for i in range(segments)
        var width = cvt(float(i), 0.0, float(segments), width_, 0.3)
        m.coord |> push(p)
        m.coord |> push(p - float2(width, 0.0))
        m.uv |> push(float2(0, 0))
        m.uv |> push(float2(rnd(0.3, 2.3), 0.0))
        p += dir
        dir += dirInc
        dir *= 0.93

    m.coord |> push(back(m.coord))
    m.uv |> push(back(m.uv))


def grass_mesh(var m: GrassMesh &; pos: float2; color_: uint; count: int)
    m.color = color_
    for i in range(-count, count)
        grass_leaf(m, float2(pos.x + float(i * 6), pos.y),
            float2(float(i) * 2.0 + rnd(-15.0, 15.0), float(-20 + abs(i) - random_int(seed) % 70)),
            rnd(5.0, 20.0),
            rnd(0.01, 0.2))



[export]
def initialize
    set_window_title("Will-o'-Wisp")
    set_antialiasing(4)
    grass_leaf_gradient <- create_image(2, 1, [{uint[] 0xFFFFFFFF; 0xFF000000}])
    grass_leaf_gradient |> set_image_smooth(true)

    sky_gradient <- create_image(2, 2, [{uint[] make_color(0.1, 0.2, 0.4);  make_color(0.25, 0.2, 0.5);
                                                make_color(0.0, 0.15, 0.0); make_color(0.0, 0.22, 0.0)}])
    sky_gradient |> set_image_smooth(true)

    particle <- create_image(64, 64)
    particle |> set_image_smooth(true)
    for iy in range(64)
        for ix in range(64)
            let x = float(ix - 32) / 32.0
            let y = float(iy - 32) / 32.0
            let angle = atan2(y, x)
            let len = length(float2(x, y))
            let k = len + (sin(angle * 10.0) + sin(angle * 18.0) + sin(angle * 34.0)) * 0.02 * len * len
            particle |> set_pixel(ix, iy, make_color(smoothstep(1.0 - k), 0.0))



    m |> resize(105)
    for x, i in m, range(99999)
        var k = float3(70 - abs(i - 50)) * 0.0123
        k *= k
        let posX = random_float(seed)
        let colorMult = 1.1 - abs(0.5 - posX) * 2.0;
        grass_mesh(x, float2(posX * 1280.0, float(i) * 2.0 + 600.0),
            make_color(float3(0.1 + random_float(seed) * 0.7, 0.9, 0.1) * k * colorMult),
            random_int(seed) % 12 + 2)



[export]
def act(dt: float)
    if get_key(VK_ESCAPE)
        schedule_quit_game()


def render_will_o_wisp(num: int; size_scale: float; x_scale: float; offset: float2 = float2(0))
    let sw = float(get_screen_width())
    let sh = float(get_screen_height())
    var t = get_time_after_start() * float(num) / 100.0
    t += sin(t) * 0.23 + sin(t * 0.4) * 0.3

    enable_premultiplied_alpha_blend()

    for i in range(num)
        let k = float(i) / float(num)

        let r = abs(sin(k * 20.0 + t)) * 0.35
        let g = abs(sin(k * 20.0 + 0.4 + t)) * 0.125
        let b = abs(sin(k * 20.0 + 0.2 + t)) * 0.1

        let x = sw * 0.5 + (sin(k * 8.0 + t * 1.3) + sin(k * 5.6 + t * 1.4) + offset.x) * sw * 0.12 * x_scale
        let y = sh * 0.5 + (sin(k * 5.0 + 2.0 + t * 1.3) + sin(k * 12.9 + 2.1 + t * 1.7) + offset.y) * sh * 0.1

        let sz = sh * (2.0 + 1.3 * sin(k * 4.0)) * 0.1 * size_scale

        particle |> draw_image(x - sz * 0.5, y - sz * 0.5, make_color(r, g, b), sz)

    disable_alpha_blend()



[export]
def draw
    sky_gradient |> draw_image(0.0, 0.0, make_color(1.0),
        float2(get_screen_width(), get_screen_height() * 4 / 5))

    let t = get_time_after_start()
    let sw = float(get_screen_width())

    render_will_o_wisp(7, 0.05, 3.0)

    for x, i in m, range(9999)
        if i == length(m) / 2
            render_will_o_wisp(100, 1.0, 1.0)

        if i == length(m) * 3 / 4
            render_will_o_wisp(2, 8.0 * (1.0 + 0.3 * sin(0.5 * t)), 0.2,
                float2(1.0 * (sin(t * 2.3) + sin(t * 1.1)), 2.5))

        var k = (1.0 - 2.0 * float(i - length(m) / 2) / float(length(m))) * 0.07
        k *= 1.0 - 2.0 * abs((x.coord[0].x - 0.4 * sw) / sw)

        grass_leaf_gradient |> draw_triangle_strip(x.coord, x.uv,
            add_colors(x.color, make_color(k * abs(sin(float(i) + t) + sin(float(i) + t * 1.4)))))


