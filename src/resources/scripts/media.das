require daslib/strings_boost public
require daslib/random public
require daslib/safe_addr public
require graphics public
require sound public
require dasbox public
require math public
require strings public
require daslib/dasbox_graphics_utils public
require daslib/dasbox_sound_utils public
require daslib/dasbox_unicode_utils public
require daslib/live public

require debugapi


class DasboxLogAgent__ : DapiDebugAgent
    def override onLog(level: int; text: string#): bool
        dasbox_log(level, text)
        return true


[export]
def dasbox_logger_agent__(ctx : Context)
    install_new_debug_agent(new DasboxLogAgent__(), "dasbox_debug_agent")


[export]
def intenral_before_initialize
    fork_debug_agent_context(@@dasbox_logger_agent__)


[export]
def print_error(text: string#)
    dasbox_log(LOG_ERROR, text)


[export]
def print_info(text: string#)
    dasbox_log(LOG_INFO, text)


[export]
def print_warning(text: string#)
    dasbox_log(LOG_WARNING, text)
