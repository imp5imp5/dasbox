#pragma once

namespace graphics
{
  void initialize();
  void finalize();
  void reset_transform();
  void on_graphics_frame_start();
  void delete_allocated_images();
}
