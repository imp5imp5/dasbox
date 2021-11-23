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

    finish_tests(testsRes, startTime)


[test]
def init_test1(t: T?)
    set_resolution(800, 800)


[test]
def act_test1(t: T?)
    t |> equal(get_screen_height(), 800)


[test]
def draw_test1(t: T?)
    t |> equal(get_screen_height(), 800)
