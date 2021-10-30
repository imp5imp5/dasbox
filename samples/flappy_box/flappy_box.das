require daslib/media

let
    BIRD_COLOR      = 0xFF3F70
    GROUND_COLOR    = 0x504022
    COLUMN_COLOR    = 0x449988
    SKY_COLOR       = 0xB0E0FF
    BACKGROUND_COLOR= 0xA6D6D6

    FONT_WIDTH = 4
    FONT_HEIGHT = 5
    font = [[string[FONT_HEIGHT]
        "###  #  ### ### # # ### ### ### ### ### ";
        "# # ##    #   # # # #   #     # # # # # ";
        "# #  #  ###  ## ### ### ###  #  ### ### ";
        "# #  #  #     #   #   # # # #   # #   # ";
        "### ### ### ###   # ### ### #   ### ### "
    ]]


struct Bird
    position: float2
    velocity: float2
    size: float
    alive: bool


struct Column
    alive: bool
    passed: bool
    posX: float
    width: float
    holeTop: float
    holeBottom: float


var
    RAND_SEED: int4
    camera_x = 0.0
    score = 0
    jump_pressed = false
    bird: Bird
    columns: array<Column>


def render_num_centered(x: int; y: int; size: int; border: int; num: int; color: uint)
    let buf = "{num}"
    let len = length(buf)
    let left = x - FONT_WIDTH * size * len / 2
    var i = -1

    for c in buf
        i++
        let n = c - '0'
        for iy in range(0, FONT_HEIGHT)
            for ix in range(0, FONT_WIDTH)
                if character_at(font[iy], ix + n * FONT_WIDTH) != ' '
                    fill_rect(left + (ix + i * FONT_WIDTH) * size - border, y + iy * size - border,
                        size + border * 2, size + border * 2, color)


def reset_bird(var b: Bird)
    b.position = float2(0, 200)
    b.velocity = float2(180.0, -180.0)
    b.size = 15.0
    b.alive = true


def kill_bird(var b: Bird)
    b.alive = false
    b.velocity.x = 0.0


def update_bird(var b: Bird; dt: float)
    b.position = b.position + b.velocity * dt
    b.velocity.y += 870.0 * dt
    if b.position.y < 0.0
        b.velocity.y = 50.0
    if b.position.y > float(get_screen_height()) - 50.0
        kill_bird(b)
        b.position.y = float(get_screen_height()) - 50.0


def jump_bird(var b: Bird)
    if b.alive
        b.velocity.y = clamp(b.velocity.y - 600.0, -1200.0, -300.0)


def render_bird(var b: Bird)
    let x = int(b.position.x - camera_x - b.size * 0.5)
    let y = int(b.position.y - b.size * 0.5)
    fill_rect(x, y, int(b.size), int(b.size), BIRD_COLOR)


def init_column(var c: Column)
    c.alive = true
    c.passed = false
    c.width = 160.0
    c.posX = camera_x + float(get_screen_width())
    c.holeTop = float(random_int(RAND_SEED) % (get_screen_height() - 300))
    c.holeBottom = c.holeTop + 200.0


def render_column(var c: Column)
    fill_rect(int(c.posX - camera_x), 0, int(c.width), int(c.holeTop), COLUMN_COLOR)
    fill_rect(int(c.posX - camera_x), int(c.holeBottom), int(c.width), get_screen_height(), COLUMN_COLOR)


def render_background(t: float)
    fill_rect(0, 0, get_screen_width(), get_screen_height() - 200, SKY_COLOR)
    var width = get_screen_width() + 32

    for i in range(0, width / 32)
        let randomY = (i * (i + 123)) & 255
        fill_rect(width - (int(t + float(width)) % width + i * 32) % width - 32,
            get_screen_height() - 200 - randomY, 32, 200 + randomY, BACKGROUND_COLOR)


def render_ground()
    fill_rect(0, get_screen_height() - 50, get_screen_width(), 200, GROUND_COLOR)


def check_for_new_column()
    if length(columns) == 0 || columns[length(columns) - 1].posX < camera_x + float(get_screen_width()) - 400.0
        var c: Column
        init_column(c)
        push(columns, c)


def remove_out_of_screen_column()
    let index = find_index_if(columns) <| $(item)
        return item.posX + item.width < camera_x - float(get_screen_width()) * 0.5
    if index >= 0
        erase(columns, index)


def reset_game()
    reset_bird(bird)
    clear(columns)
    score = 0


[export]
def initialize
    set_window_title("Flappy Box")
    randomize_seed(RAND_SEED)
    reset_game()


def update_input()
    let jump = get_key(VK_SPACE)
    if jump && !jump_pressed
        if bird.alive
            jump_bird(bird)
        else
            reset_game()
    jump_pressed = jump


def update_gameplay(dt: float)
    update_bird(bird, dt)
    camera_x = bird.position.x - float(get_screen_width()) * 0.5
    for c in columns
        if !c.passed
            if (bird.position.x > c.posX && bird.position.x < c.posX + c.width &&
                (bird.position.y < c.holeTop || bird.position.y > c.holeBottom))
                kill_bird(bird)

            if c.posX + c.width < bird.position.x
                c.passed = true
                score++

    check_for_new_column()
    remove_out_of_screen_column()


def draw_score()
    render_num_centered(get_screen_width() / 2, 10, 6, 2, score, 0x0)
    render_num_centered(get_screen_width() / 2, 10, 6, 0, score, 0xFFFFFF)


[export]
def act(dt: float)
    // print("act dt={dt}\n")
    if get_key(VK_ESCAPE)
        schedule_quit_game()
    update_input()
    update_gameplay(dt)


[export]
def draw()
    render_background(camera_x * 0.4)
    for c in columns
        render_column(c)
    render_ground()
    render_bird(bird)
    draw_score()

