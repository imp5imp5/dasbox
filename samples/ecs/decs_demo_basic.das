require daslib/media
require daslib/decs_boost


[decs_template]
struct Particle
    pos: float2
    vel: float2
    radius: float


def make_particle(pos, vel: float2; radius: float)
    create_entity <| @ ( eid, cmp )
        cmp |> set("eid", eid)
        apply_decs_template(cmp, [[Particle pos = pos, vel = vel, radius = radius]])


[decs(stage = update)]
def update_particle(var p: Particle)
    let dt = get_delta_time()
    let sw = float(get_screen_width())
    let sh = float(get_screen_height())
    p.vel.y += dt * 1200.0
    p.pos += dt * p.vel
    if p.pos.x < p.radius
        p.pos.x = p.radius
        p.vel.x = -p.vel.x
    if p.pos.x > sw - p.radius
        p.pos.x = sw - p.radius
        p.vel.x = -p.vel.x
    if p.pos.y > sh - p.radius
        p.pos.y = sh - p.radius
        p.vel.y = -p.vel.y


[decs(stage = draw)]
def draw_particle(var p: Particle)
    fill_circle(p.pos.x, p.pos.y, p.radius, 0xFFFFFF)



[export]
def initialize(is_hard_reload: bool)
    set_window_title("DECS Demo (Entity Component System)")

    if is_hard_reload
        for i in range(4)
            let pos = float2(200.0 + float(i * 100), 200.0 + float(i * 50))
            let vel = float2(120.1, 0.0)
            let radius = float(i + 1) * 10.0
            make_particle(pos, vel, radius)
        commit()


[export]
def act(dt: float)
    if get_key(VK_ESCAPE)
        schedule_quit_game()

    if get_mouse_button_down(MB_LEFT)
        make_particle(get_mouse_position(), float2(-300.0, -300.0), 20.0)
        commit()

    decs_stage("update")


[export]
def draw
    decs_stage("draw")
    text_out(20, 20, "F5 - reload (ECS database will be saved after reload)", 0xFFFFFF)
    text_out(20, 50, "Ctrl+F5 - hard reload", 0xFFFFFF)
    text_out(20, 80, "Click - create new particle", 0xFFFFFF)
