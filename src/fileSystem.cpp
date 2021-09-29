#include "fileSystem.h"

#include <algorithm>

#ifdef _WIN32
#include <direct.h>
#define chdir _chdir
#else
#include "unistd.h"
#endif

using namespace std;

namespace fs
{


DasboxFsFileAccess::DasboxFsFileAccess(const char * pak, bool allow_hot_reload) :
  das::ModuleFileAccess(pak, das::make_smart<DasboxFsFileAccess>(false)), storeOpenedFiles(allow_hot_reload)
{
  daslibPath = das::getDasRoot() + "/daslib";
}

DasboxFsFileAccess::DasboxFsFileAccess(bool allow_hot_reload) :
  storeOpenedFiles(allow_hot_reload)
{
  daslibPath = das::getDasRoot() + "/daslib";
}

DasboxFsFileAccess::DasboxFsFileAccess(DasboxFsFileAccess * modAccess, bool allow_hot_reload) :
  storeOpenedFiles(allow_hot_reload)
{
  daslibPath = das::getDasRoot() + "/daslib";
  if (modAccess)
  {
    context = modAccess->context;
    modGet = modAccess->modGet;
    includeGet = modAccess->includeGet;
    moduleAllowed = modAccess->moduleAllowed;
    derivedAccess = true;
  }
}

DasboxFsFileAccess::~DasboxFsFileAccess()
{
  if (derivedAccess)
    context = nullptr;
}

das::FileInfo * DasboxFsFileAccess::getNewFileInfo(const das::string & fname)
{
  FILE * f = fopen(fname.c_str(), "rb");
  if (!f)
  {
    print_error("Script file '%s' not found", fname.c_str());
    return nullptr;
  }

  fseek(f, 0, SEEK_END);
  const uint32_t fileLength = uint32_t(ftell(f));
  fseek(f, 0, SEEK_SET);

  auto info = das::make_unique<DasboxFsFileInfo>();
  info->sourceLength = fileLength;
  char * source = (char *)das_aligned_alloc16(info->sourceLength + 1);
  if (fread((void*)source, 1, info->sourceLength, f) != info->sourceLength)
  {
    fclose(f);
    free(source);
    print_error("Cannot read file '%s'", fname.c_str());
    return nullptr;
  }

  fclose(f);

  source[info->sourceLength] = 0;
  info->source = source;
  if (storeOpenedFiles)
  {
    int64_t fileTime = get_file_time(fname.c_str());
    filesOpened.emplace_back(fname, fileTime);
  }
  return setFileInfo(fname, std::move(info));
}

das::ModuleInfo DasboxFsFileAccess::getModuleInfo(const das::string & req, const das::string & from) const
{
  if (!failed())
    return das::ModuleFileAccess::getModuleInfo(req, from);
  bool req_uses_mnt = strncmp(req.c_str(), "%", 1) == 0;
  return das::ModuleFileAccess::getModuleInfo(req, req_uses_mnt ? das::string() : from);
}



inline bool is_slash(char c)
{
  return c == '\\' || c == '/';
}

bool is_path_string_valid(const char * path)
{
  if (!path || !path[0])
    return true;
  if (strchr(path, ':'))
    return false;
  if (path[0] == '/' || (path[0] == '~' && path[1] == '/'))
    return false;

  if (path[0] == '.' && path[1] == '.' && is_slash(path[2]))
    return false;

  int len = int(strlen(path));
  int depth = 0;
  for (int i = 0; i < len; i++)
  {
    if (is_slash(path[i]) && !is_slash(path[i + 1]))
      depth++;
    if (path[i] == '.' && path[i + 1] == '.')
      depth--;
    if (depth < 0)
      return false;
  }

  return true;
}

string combine_path(const string & path1, const string & path2)
{
  string res = path1;
  while (!res.empty() && (res.back() == '/' || res.back() == '\\'))
    res.pop_back();
  res += '/';
  res += path2;
  return res;
}

string extract_dir(const string & path)
{
  const char * p = path.c_str();
  const char * slash = std::max(strrchr(p, '/'), strrchr(p, '\\'));
  return slash ? string(p, slash) : string("");
}

string extract_file_name(const string & path)
{
  const char * p = path.c_str();
  const char * slash = std::max(strrchr(p, '/'), strrchr(p, '\\'));
  return slash ? string(slash + 1) : path;
}

bool change_dir(const string & dir)
{
  if (dir.empty())
    return true;
  string s = dir;
  while (!s.empty() && (s.back() == '/' || s.back() == '\\'))
    s.pop_back();
  return chdir(s.c_str()) == 0;
}

bool is_file_exists(const string & file_name)
{
  struct stat buffer;
  return stat(file_name.c_str(), &buffer) == 0;
}

uint64_t get_file_time(const string & file_name)
{
  struct stat buf;
  if (!stat(file_name.c_str(), &buf))
    return uint64_t(buf.st_mtime);
  return 0;
}

}
