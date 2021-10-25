#pragma once

namespace fs
{
  void initialize_local_storage(const char * app_name);
  void update_local_storage(float dt);
  void flush_local_storage();
  void local_storage_set(const char * key, const char * value);
  const char * local_storage_get(const char * key);
}


