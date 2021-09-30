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
std::string combine_path(const std::string & path1, const std::string & path2);
std::string extract_dir(const std::string & path);
std::string extract_file_name(const std::string & path);
bool change_dir(const std::string & dir);
bool is_file_exists(const std::string & file_name);
uint64_t get_file_time(const std::string & file_name);


class DasboxFsFileInfo final : public das::FileInfo
{
public:
  virtual void freeSourceData() override
  {
    if (source)
      das_aligned_free16((void*)source);
    source = nullptr;
  }

  virtual ~DasboxFsFileInfo()
  {
    freeSourceData();
  }
};


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
