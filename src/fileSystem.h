#pragma once

#include <vector>
#include <string>
#include <stdint.h>
#include <daScript/daScript.h>
#include <stdio.h>
#include "logger.h"
#include "globals.h"

namespace fs
{

void initialize();
bool is_path_string_valid(const char * path);
bool read_whole_file(const char * file_name, std::vector<uint8_t> & bytes);
std::string read_whole_file(const char * file_name);
bool write_string_to_file(const char * file_name, const char * str);
std::string combine_path(const std::string & path1, const std::string & path2);
std::string extract_dir(const std::string & path);
std::string extract_file_name(const std::string & path);
bool change_dir(const std::string & dir);
bool is_file_exists(const char *  file_name);
std::string get_current_dir();
uint64_t get_file_time(const char * file_name);
uint64_t get_file_size(const char * file_name);
std::string get_user_data_dir();
das::string find_main_das_file_in_directory(const char * path);



class DasboxFsFileAccess final : public das::ModuleFileAccess
{
public:
  std::vector<std::pair<std::string, int64_t>> filesOpened;
  bool storeOpenedFiles = true;
  bool derivedAccess = false;
  DasboxFsFileAccess(const char * pak, bool allow_hot_reload = true);
  DasboxFsFileAccess(bool allow_hot_reload = true);
  DasboxFsFileAccess(DasboxFsFileAccess * modAccess, bool allow_hot_reload = true);

  virtual ~DasboxFsFileAccess() override;
  virtual das::FileInfo * getNewFileInfo(const das::string & fname) override;
  virtual das::ModuleInfo getModuleInfo(const das::string & req, const das::string & from) const override;
};

}
