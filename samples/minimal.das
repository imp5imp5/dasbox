require daslib/media

[export]
def initialize
    return

[export]
def act(dt: float)
    if get_key(VK_ESCAPE)
        schedule_quit_game()
    return

[export]
def draw
    return
