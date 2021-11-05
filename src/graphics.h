#pragma once

namespace graphics
{
  void initialize();
  void finalize();
  void reset_transform();
  void on_graphics_frame_start();
  void delete_allocated_images();
  void print_debug_infos(int from_frame);
  int get_image_count();
  double get_memory_used();
  int get_render_primitives_count();
  int get_updated_textures_count();
}
