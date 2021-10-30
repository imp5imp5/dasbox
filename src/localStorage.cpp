#include "localStorage.h"
#include "fileSystem.h"
#include "globals.h"

#include <unordered_map>
#include <string>

#define LOCAL_STORAGE_MAGIC_NUMBER 0x010000BD
#define STORAGE_STRING_SIZE_LIMIT (1 << 24)

using namespace std;

namespace fs
{

static unordered_map<std::string, std::string> inmemory_local_storage;
static string local_storage_file_name = string("");
static string app_name = string("");
static float time_to_save_local_storage = -1.0;
static float flush_storage_delay = 0.1f;

static uint32_t simple_hash(const char * s, uint32_t len)
{
  uint32_t res = len;
  uint32_t m = 0;
  uint8_t * p = (uint8_t *)s;
  for (uint32_t i = 0; i < len; i++)
  {
    res += *p + (*p << m);
    m = (m + 3) & 15;
    p++;
  }
  return res;
}

void flush_local_storage()
{
  time_to_save_local_storage = -1.0f;

  if (local_storage_file_name.empty())
    return;

  FILE * f = fopen(local_storage_file_name.c_str(), "wb");
  if (f)
  {
    uint32_t hash = LOCAL_STORAGE_MAGIC_NUMBER;
    int32_t startN = LOCAL_STORAGE_MAGIC_NUMBER;
    int32_t count = int(inmemory_local_storage.size());

    fwrite(&startN, sizeof(startN), 1, f);
    fwrite(&count, sizeof(count), 1, f);

    for (auto && v : inmemory_local_storage)
    {
      int32_t nameLen = int(v.first.length());
      int32_t valLen = int(v.second.length());
      fwrite(&nameLen, sizeof(nameLen), 1, f);
      fwrite(&valLen, sizeof(valLen), 1, f);
      fwrite(v.first.c_str(), nameLen, 1, f);
      fwrite(v.second.c_str(), valLen, 1, f);

      hash += simple_hash(v.first.c_str(), nameLen);
      hash += simple_hash(v.second.c_str(), valLen);
    }

    bool error = fwrite(&hash, sizeof(hash), 1, f) != 1;

    if (error)
      print_error("Cannot write to storage file (%s)", local_storage_file_name.c_str());

    fclose(f);
  }
  else
    print_error("Cannot open storage file for write (%s)", local_storage_file_name.c_str());
}

void update_local_storage(float dt)
{
  if (time_to_save_local_storage >= 0.0f)
  {
    time_to_save_local_storage -= dt;
    if (time_to_save_local_storage < 0.0f)
    {
      flush_local_storage();
      flush_storage_delay *= 4.0f;
    }
  }

  flush_storage_delay = ::clamp(flush_storage_delay - dt * 0.1f, 0.1f, 10.0f);
}

void initialize_local_storage(const char * app_name_)
{
  inmemory_local_storage.clear();
  app_name = string(app_name_);
  local_storage_file_name.clear();
}

void ensure_open_storage()
{
  if (local_storage_file_name.empty())
  {
    local_storage_file_name = fs::get_user_data_dir() + app_name + ".storage";
    if (fs::is_file_exists(local_storage_file_name.c_str()))
    {
      inmemory_local_storage.clear();
      FILE * f = fopen(local_storage_file_name.c_str(), "rb");
      if (f)
      {
        bool error = false;
        int32_t nameLen = 0;
        int32_t count = 0;
        int32_t valLen = 0;
        int32_t startN = 0;
        uint32_t hash = LOCAL_STORAGE_MAGIC_NUMBER;
        uint32_t endN = 0;
        error = !(fread(&startN, sizeof(startN), 1, f) &&
          startN == LOCAL_STORAGE_MAGIC_NUMBER &&
          fread(&count, sizeof(count), 1, f) &&
          count >= 0);

        for (int i = 0; i < count && !error; i++)
          if (fread(&nameLen, sizeof(nameLen), 1, f) && fread(&valLen, sizeof(valLen), 1, f))
          {
            if (nameLen >= 0 && nameLen <= STORAGE_STRING_SIZE_LIMIT && valLen >= 0 && valLen <= STORAGE_STRING_SIZE_LIMIT)
            {
              string name(nameLen, 0);
              string value(valLen, 0);
              fread(&name[0], nameLen, 1, f);
              fread(&value[0], valLen, 1, f);
              inmemory_local_storage[name] = value;

              hash += simple_hash(name.c_str(), nameLen);
              hash += simple_hash(value.c_str(), valLen);
            }
            else
              error = true;
          }
          else
            error = true;

        if (!error)
          error = !(fread(&endN, sizeof(endN), 1, f) && endN == hash);

        if (error)
        {
          print_error("Corrupted storage file (%s)", local_storage_file_name.c_str());
          inmemory_local_storage.clear();
        }

        fclose(f);
      }
      else
        print_error("Cannot open storage file (%s)", local_storage_file_name.c_str());
    }
  }
}

void local_storage_set(const char * key, const char * value)
{
  if (!key)
    return;
  if (!value)
    value = "";
  ensure_open_storage();
  inmemory_local_storage[std::string(key)] = std::string(value);
  if (time_to_save_local_storage < 0)
    time_to_save_local_storage = flush_storage_delay;
}

const char * local_storage_get(const char * key)
{
  if (!key)
    return "";
  ensure_open_storage();
  return das_str_dup(inmemory_local_storage[std::string(key)].c_str());
}

bool local_storage_has_key(const char * key)
{
  if (!key)
    return false;
  ensure_open_storage();
  return inmemory_local_storage.find(std::string(key)) != inmemory_local_storage.end();
}

} // namespace fs
