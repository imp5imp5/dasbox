require daslib/media
// options debugger            // uncomment for debug in VS Code
// require daslib/debug        // uncomment for debug in VS Code


// 'initialize' runs once when game starts and every hot-reload
[export]
def initialize
    return

// this function is called to update game data,
// dt - time elapsed since the previous update (in seconds)
[export]
def act(dt: float)
    if get_key(VK_ESCAPE)
        schedule_quit_game()
    return

// this function will be called every frame after 'act',
// you can draw anything in this function
[export]
def draw
    return
