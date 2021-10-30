options indenting = 4
require graphics
require dasbox
require math
require daslib/random
require strings


struct Rock
    curvature: float[ROCK_SHAPE_POINTS]
    radius: float
    pos: float2
    vel: float2
    alive: bool


struct Bullet
    ttl: float
    canDamage: bool
    pos: float2
    vel: float2


struct Ship
    radius: float
    pos: float2
    vel: float2
    angle: float
    accelerating: float
    invTime: float


var
    rocks: array<Rock>
    bullets: array<Bullet>
    new_rocks: array<Rock>
    new_bullets: array<Bullet>
    ship: Ship
    timeToFire: float = 0.0
    timeToRestart: float = -1.0
    lives: int = 3


let
    ROCK_SHAPE_POINTS = 16
    ROCK_SPEED = 30.0
    WHITE = 0xFFFFFFFF


var
    seed: int4



def angle2vec(angle: float): float2
    return float2(sin(angle), cos(angle))


def dist_wrapped(a: float2; b: float2): float
    var res = 1000000.f
    for dx in range(-1, 2)
        for dy in range(-1, 2)
            let d = length(a - b + float2(float(dx * get_screen_width()), float(dx * get_screen_height())))
            if d < res
                res = d
    return res


def wrap(var v: float2&)
    if v.x < 0.0
        v.x += float(get_screen_width())
    if v.y < 0.0
        v.y += float(get_screen_height())
    if v.x > float(get_screen_width())
        v.x -= float(get_screen_width())
    if v.y > float(get_screen_height())
        v.y -= float(get_screen_height())


def rock_update(var a: Rock; dt: float)
    a.pos += a.vel * dt
    wrap(a.pos)


def rock_reset(var a: Rock)
    a.alive = true
    for i in range(0, ROCK_SHAPE_POINTS)
        a.curvature[i] = random_float(seed)

    a.radius = 80.0
    a.pos = float2(float(random_int(seed) % get_screen_width()), 0.0)
    a.vel = (float2(random_float(seed), random_float(seed)) - float2(0.5))
    a.vel = normalize(a.vel) * ROCK_SPEED


def rock_render(var a: Rock)
    for dx in range(-1, 2)
        for dy in range(-1, 2)
            for i1 in range(0, ROCK_SHAPE_POINTS)
                let i2 = (i1 + 1) % ROCK_SHAPE_POINTS
                let angle1 = float(i1) * PI * 2.0 / float(ROCK_SHAPE_POINTS)
                let angle2 = float(i2) * PI * 2.0 / float(ROCK_SHAPE_POINTS)
                let r1 = a.curvature[i1] * a.radius * 0.25 + a.radius
                let r2 = a.curvature[i2] * a.radius * 0.25 + a.radius
                let p1 = a.pos + float2(sin(angle1), cos(angle1)) * r1
                let p2 = a.pos + float2(sin(angle2), cos(angle2)) * r2
                line(p1.x + float(dx * get_screen_width()), p1.y + float(dy * get_screen_height()),
                     p2.x + float(dx * get_screen_width()), p2.y + float(dy * get_screen_height()), WHITE)


def ship_reset(var s: Ship)
    s.pos.x = float(get_screen_width()) * 0.5
    s.pos.y = float(get_screen_height()) * 0.5
    s.vel = float2(0.0)
    s.angle = -PI
    s.accelerating = 0.0
    s.radius = 20.0
    s.invTime = 4.0


def ship_rotate(var s: Ship; dir: float; dt: float)
    s.angle += dir * dt * 4.0f


def ship_accelerate(var s: Ship; dt: float)
    s.vel += angle2vec(s.angle) * 50.0 * 4.0f * dt
    s.accelerating = 0.05


def ship_brake(var s: Ship; dt: float)
    if length(s.vel) < 10.0
        s.vel = float2(0.0)
    else
        s.vel -= normalize(s.vel) * 100.0f * 4.0f * dt
    s.accelerating = 0.05


def ship_update(var s: Ship; dt: float)
    s.pos += s.vel * dt
    wrap(s.pos)
    s.accelerating -= dt
    s.invTime -= dt


def ship_render(var s: Ship)
    if s.invTime > 0.0
        if (int(s.invTime * 16.0) & 1) == 0
            return;

    let pnt = [[float2[4]
        angle2vec(s.angle + 0.0) * s.radius;
        angle2vec(s.angle + PI * 0.8) * float2(s.radius);
        angle2vec(s.angle + PI) * s.radius * 0.6;
        angle2vec(s.angle + PI * 1.2) * s.radius
    ]]

    for dx in range(-1, 2)
        for dy in range(-1, 2)
            for i in range(0, 4)
                let p1 = pnt[i] + s.pos
                let p2 = pnt[(i + 1) % 4] + s.pos
                line(p1.x + float(dx * get_screen_width()), p1.y + float(dy * get_screen_height()),
                     p2.x + float(dx * get_screen_width()), p2.y + float(dy * get_screen_height()), WHITE)


def bullet_update(var b: Bullet; dt: float)
    b.pos += b.vel * dt
    wrap(b.pos)
    b.ttl -= dt


def bullet_render(var b: Bullet)
    for dx in range(-1, 2)
        for dy in range(-1, 2)
            let p1 = b.pos + b.vel * 0.01
            let p2 = b.pos - b.vel * 0.01
            line(p1.x + float(dx * get_screen_width()), p1.y + float(dy * get_screen_height()),
                 p2.x + float(dx * get_screen_width()), p2.y + float(dy * get_screen_height()), WHITE)


def add_bullet(pos: float2; vel: float2; ttl: float; canDamage: bool)
    var b: Bullet
    b.canDamage = canDamage
    b.pos = pos
    b.vel = vel
    b.ttl = ttl
    push(new_bullets, b)


def reset_game
    timeToRestart = -1.0
    lives = 3
    ship_reset(ship)
    ship.invTime = 0.0

    clear(bullets)
    clear(rocks)
    clear(new_rocks)
    clear(new_bullets)

    for i in range(0, 3)
        var a: Rock
        rock_reset(a)
        push(rocks, a)


[export]
def initialize()
    set_antialiasing(4)
    set_window_title("Space, Rocks and Lasers")
    print("Screen size: {get_screen_width()} x {get_screen_height()}\n")
    // schedule_pause()
    randomize_seed(seed)
    reset_game()


def create_explosion(pos: float2; vel: float2; speed: float; radius: float; particles: int)
    for i in range(0, particles)
        let dir = angle2vec(random_float(seed) * PI * 2.0) * float(random_int(seed) % 150 + 1) / 150.0
        add_bullet(pos + dir * radius,
                vel + dir * 100.0f / speed, float(random_int(seed) % 150 + 1) / 100.0f * speed, false)


[export]
def act(dt: float)
    // print("dt = {dt}\n")
    if get_key(VK_ESCAPE)
        schedule_quit_game()

    for a in rocks
        rock_update(a, dt)
        if ship.invTime < 0.0 && dist_wrapped(a.pos, ship.pos) < a.radius + ship.radius * 0.6f
            create_explosion(ship.pos, ship.vel, 1.0, ship.radius, 250)
            ship_reset(ship)
            lives--
            break

    for b in bullets
        bullet_update(b, dt)
        if b.canDamage
            for a in rocks
                if dist_wrapped(a.pos, b.pos) < a.radius + 4.0
                    create_explosion(a.pos, a.vel, 0.5, a.radius, 50)

                    if a.radius > 10.0
                        var newAst = a
                        newAst.radius /= 2.0f
                        newAst.vel = float2(a.vel.y, -a.vel.x) * 2.2
                        newAst.pos += normalize(newAst.vel) * newAst.radius
                        push(new_rocks, newAst)
                        newAst.pos -= normalize(newAst.vel) * newAst.radius * 2.0
                        newAst.vel = -newAst.vel
                        push(new_rocks, newAst)

                    a.alive = false
                    b.canDamage = false
                    b.ttl = -1.0
                    break

    var idx = length(rocks) - 1
    while idx >= 0
        if !rocks[idx].alive
            erase(rocks, idx)
        idx--

    for a in new_rocks
        push(rocks, a)


    idx = length(bullets) - 1
    while idx >= 0
        if bullets[idx].ttl < 0.0
            erase(bullets, idx)
        idx--

    for b in new_bullets
        push(bullets, b)


    clear(new_rocks)
    clear(new_bullets)


    if get_key(VK_LEFT)
        ship_rotate(ship, 1.0, dt)
    if get_key(VK_RIGHT)
        ship_rotate(ship, -1.0, dt)
    if get_key(VK_UP)
        ship_accelerate(ship, dt)
    if get_key(VK_DOWN)
        ship_brake(ship, dt)


    timeToFire -= dt
    if timeToFire < 0.0 && get_key(VK_SPACE) && lives >= 0
        add_bullet(ship.pos + angle2vec(ship.angle) * ship.radius, angle2vec(ship.angle) * 600.0, 0.75, true)
        timeToFire = 0.15

    ship_update(ship, dt)

    if timeToRestart <= 0.0
        if lives < 0 || empty(rocks)
            timeToRestart = 2.0

    if timeToRestart > 0.0
        timeToRestart -= dt
        if timeToRestart < 0.0
            reset_game()


[export]
def draw()
    for a in rocks
        rock_render(a)

    for b in bullets
        bullet_render(b)

    if lives > 0
        for i in range(0, lives)
            fill_rect(10 + i * 15, 10, 10, 10, WHITE);

    if lives >= 0
        ship_render(ship)

