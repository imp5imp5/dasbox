#pragma once

#include <daScript/daScript.h>


namespace sound
{
  struct PcmSound;

  struct PlayingSoundHandle
  {
    unsigned handle = 0;
  };



  void initialize();
  void finalize();

  PcmSound create_sound(int frequency, const das::TArray<float> & data);
  PcmSound create_sound_stereo(int frequency, const das::TArray<das::float2> & data);
  PcmSound create_sound_from_file(const char * file_name);
  void get_sound_data(const PcmSound & sound, das::TArray<float> & out_data);
  void get_sound_data_stereo(const PcmSound & sound, das::TArray<das::float2> & out_data);
  void set_sound_data(PcmSound & sound, const das::TArray<float> & in_data);
  void set_sound_data_stereo(PcmSound & sound, const das::TArray<das::float2> & in_data);
  void delete_sound(PcmSound * sound);
  void delete_allocated_sounds();

  PlayingSoundHandle play_sound_1(const PcmSound & sound);
  PlayingSoundHandle play_sound_2(const PcmSound & sound, float volume);
  PlayingSoundHandle play_sound_3(const PcmSound & sound, float volume, float pitch);
  PlayingSoundHandle play_sound_4(const PcmSound & sound, float volume, float pitch, float pan);
  PlayingSoundHandle play_sound_5(const PcmSound & sound, float volume, float pitch, float pan, float start_time, float end_time);
  PlayingSoundHandle play_sound_loop_1(const PcmSound & sound);
  PlayingSoundHandle play_sound_loop_2(const PcmSound & sound, float volume);
  PlayingSoundHandle play_sound_loop_3(const PcmSound & sound, float volume, float pitch);
  PlayingSoundHandle play_sound_loop_4(const PcmSound & sound, float volume, float pitch, float pan);
  PlayingSoundHandle play_sound_loop_5(const PcmSound & sound, float volume, float pitch, float pan, float start_time, float end_time);
  PlayingSoundHandle play_sound_deferred_1(const PcmSound & sound, float defer_seconds);
  PlayingSoundHandle play_sound_deferred_2(const PcmSound & sound, float defer_seconds, float volume);
  PlayingSoundHandle play_sound_deferred_3(const PcmSound & sound, float defer_seconds, float volume, float pitch);
  PlayingSoundHandle play_sound_deferred_4(const PcmSound & sound, float defer_seconds, float volume, float pitch, float pan);
  PlayingSoundHandle play_sound_deferred_5(const PcmSound & sound, float defer_seconds, float volume, float pitch, float pan,
    float start_time, float end_time);

  bool is_playing(PlayingSoundHandle handle);
  void set_sound_pitch(PlayingSoundHandle handle, float pitch);
  void set_sound_volume(PlayingSoundHandle handle, float volume);
  void set_sound_pan(PlayingSoundHandle handle, float pan);
  float get_sound_play_pos(PlayingSoundHandle handle);
  void set_sound_play_pos(PlayingSoundHandle handle, float pos_seconds);
  void stop_sound(PlayingSoundHandle handle);
  void stop_all_sounds();
  void enter_sound_critical_section();  // ?
  void leave_sound_critical_section();  // ?

  void set_master_volume(float volume);
  float get_output_sample_rate();
  int64_t get_total_samples_played();
  double get_total_time_played();
  int get_total_sound_count();
  int get_playing_sound_count();
  void print_debug_infos(int from_frame);
}
