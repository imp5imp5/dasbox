require sound
require dasbox
require math
require strings

var private sounds: array<PcmSound>
var private deleted_sounds: array<int>


struct SoundHandle
    index: int
    duration: float
    frequency: int
    samples: int
    channels: int


def private setup_sound(index: int): SoundHandle
    var h: SoundHandle
    h.index = index
    h.duration = sounds[index].duration
    h.frequency = sounds[index].frequency
    h.samples = sounds[index].samples
    h.channels = sounds[index].channels
    return h

def private allocate_sound(): int
    if empty(deleted_sounds)
        sounds |> reserve(3000)
        var snd: PcmSound
        sounds |> emplace(snd)
        return length(sounds) - 1
    let res = deleted_sounds |> back()
    deleted_sounds |> pop()
    return res

def move_to_handled_storage(var sound: PcmSound &): SoundHandle
    var i = allocate_sound()
    sounds[i] <- sound
    return setup_sound(i)

def clone_to_handled_storage(sound: PcmSound): SoundHandle
    var i = allocate_sound()
    sounds[i] := sound
    return setup_sound(i)

def delete_managed_sound(sound_handle: SoundHandle)
    if (sound_handle.index > 0 && sounds[sound_handle.index].valid)
        deleted_sounds |> push(sound_handle.index)
        delete sounds[sound_handle.index]

def clone_managed_sound(sound_handle: SoundHandle): SoundHandle
    var i = allocate_sound()
    sounds[i] := sounds[sound_handle.index]
    return setup_sound(i)

def create_managed_sound(file_name: string): SoundHandle
    var i = allocate_sound()
    sounds[i] <- create_sound(file_name)
    return setup_sound(i)

def create_managed_sound(frequency: int; data): SoundHandle
    var i = allocate_sound()
    sounds[i] <- create_sound(frequency, data)
    return setup_sound(i)

def set_sound_data(sound_handle: SoundHandle; data)
    sounds[sound_handle.index] |> set_sound_data(data)

def get_sound_data(sound_handle: SoundHandle; var data: array<float> & implicit)
    sounds[sound_handle.index] |> get_sound_data(data)

def get_sound_data(sound_handle: SoundHandle; var data: array<float2> & implicit)
    sounds[sound_handle.index] |> get_sound_data(data)


def play_sound(sound_handle: SoundHandle): PlayingSoundHandle
    return sounds[sound_handle.index] |> play_sound()

def play_sound(sound_handle: SoundHandle; volume): PlayingSoundHandle
    return sounds[sound_handle.index] |> play_sound(volume)

def play_sound(sound_handle: SoundHandle; volume, pitch): PlayingSoundHandle
    return sounds[sound_handle.index] |> play_sound(volume, pitch)

def play_sound(sound_handle: SoundHandle; volume, pitch, pan): PlayingSoundHandle
    return sounds[sound_handle.index] |> play_sound(volume, pitch, pan)

def play_sound(sound_handle: SoundHandle; volume, pitch, pan, start_time, stop_time): PlayingSoundHandle
    return sounds[sound_handle.index] |> play_sound(volume, pitch, pan, start_time, stop_time)

def play_sound_loop(sound_handle: SoundHandle; volume): PlayingSoundHandle
    return sounds[sound_handle.index] |> play_sound_loop(volume)

def play_sound_loop(sound_handle: SoundHandle; volume, pitch): PlayingSoundHandle
    return sounds[sound_handle.index] |> play_sound_loop(volume, pitch)

def play_sound_loop(sound_handle: SoundHandle; volume, pitch, pan): PlayingSoundHandle
    return sounds[sound_handle.index] |> play_sound_loop(volume, pitch, pan)

def play_sound_loop(sound_handle: SoundHandle; volume, pitch, pan, start_time, stop_time): PlayingSoundHandle
    return sounds[sound_handle.index] |> play_sound_loop(volume, pitch, pan, start_time, stop_time)

def play_sound_deferred(sound_handle: SoundHandle; volume): PlayingSoundHandle
    return sounds[sound_handle.index] |> play_sound_deferred(volume)

def play_sound_deferred(sound_handle: SoundHandle; volume, pitch): PlayingSoundHandle
    return sounds[sound_handle.index] |> play_sound_deferred(volume, pitch)

def play_sound_deferred(sound_handle: SoundHandle; volume, pitch, pan): PlayingSoundHandle
    return sounds[sound_handle.index] |> play_sound_deferred(volume, pitch, pan)

def play_sound_deferred(sound_handle: SoundHandle; volume, pitch, pan, start_time, stop_time): PlayingSoundHandle
    return sounds[sound_handle.index] |> play_sound_deferred(volume, pitch, pan, start_time, stop_time)

