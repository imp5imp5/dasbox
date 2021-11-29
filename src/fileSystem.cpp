#include "fileSystem.h"

#include <algorithm>
#include <string.h>
#include <fs8.h>

#ifdef _WIN32
#define NOMINMAX
#include <io.h>
#include <iostream>
#include <codecvt>
#include <locale>
#include <shlwapi.h>
#include <shlobj.h>
#include <direct.h>
#define chdir _chdir
#define getcwd _getcwd
#else
#include <libgen.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

using namespace std;



namespace fs
{

static uint32_t resources_data[] = {
  #include "resources/resources.inc"
};

Fs8FileSystem resources_fs;

// TODO: free at 'finalize'
//static unordered_map<std::string, das::TextFileInfo *> daslib_inc_files;


void initialize()
{
  resources_fs.initalizeFromMemory(resources_data, sizeof(resources_data));
}

uint64_t get_file_size(const char * file_name)
{
  if (!file_name || !file_name[0])
  {
    print_error("Cannot access file. File name is empty.");
    return false;
  }

  if (!is_path_string_valid(file_name))
  {
    print_error("Cannot access file '%s'. Absolute paths or access to the parent directory is prohibited.", file_name);
    return false;
  }

  FILE * f = fopen(file_name, "rb");
  if (!f)
  {
    print_error("Cannot access file '%s'", file_name);
    return false;
  }

#ifdef _WIN32
  _fseeki64(f, 0, SEEK_END);
  uint64_t fileLength = _ftelli64(f);
#else
  fseeko(f, 0, SEEK_END);
  uint64_t fileLength = ftello(f);
#endif

  fclose(f);
  return fileLength;
}


bool read_whole_file(const char * file_name, std::vector<uint8_t> & bytes)
{
  if (!file_name || !file_name[0])
  {
    print_error("Cannot open file. File name is empty.");
    return false;
  }

  if (!is_path_string_valid(file_name))
  {
    print_error("Cannot open file '%s'. Absolute paths or access to the parent directory is prohibited.", file_name);
    return false;
  }

  FILE * f = fopen(file_name, "rb");
  if (!f)
  {
    print_error("Cannot open file '%s'", file_name);
    return false;
  }

  fseek(f, 0, SEEK_END);
  const uint32_t fileLength = uint32_t(ftell(f));
  fseek(f, 0, SEEK_SET);
  bytes.resize(fileLength);
  if (fread((void*)&bytes[0], 1, fileLength, f) != fileLength)
  {
    fclose(f);
    bytes.clear();
    print_error("Cannot read file '%s'", file_name);
    return false;
  }

  fclose(f);
  return true;
}


std::string read_whole_file(const char * file_name)
{
  vector<uint8_t> data;
  read_whole_file(file_name, data);
  data.push_back(0);
  return string((const char *)&data[0]);
}

bool write_string_to_file(const char * file_name, const char * str)
{
  if (!file_name)
    file_name = "";

  FILE * f = fopen(file_name, "wb");
  if (!f)
  {
    print_error("Cannot open file '%s' for write", file_name);
    return false;
  }

  fwrite(str, strlen(str), 1, f);
  fclose(f);
  return true;
}


DasboxFsFileAccess::DasboxFsFileAccess(const char * pak, bool allow_hot_reload) :
  das::ModuleFileAccess(pak, das::make_smart<DasboxFsFileAccess>(false)), storeOpenedFiles(allow_hot_reload)
{
}

DasboxFsFileAccess::DasboxFsFileAccess(bool allow_hot_reload) :
  storeOpenedFiles(allow_hot_reload)
{
}

DasboxFsFileAccess::DasboxFsFileAccess(DasboxFsFileAccess * modAccess, bool allow_hot_reload) :
  storeOpenedFiles(allow_hot_reload)
{
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

/*
void check_source_for_correct_debug_position(const das::string & fname, char * s)
{
  const char * p = s;
  for (;;)
  {
    p = strstr(p, "require daslib/debug");
    if (!p)
      break;
    if (p == s || p[-1] == '\n')
    {
      const char * req = strstr(p + 1, "\nrequire ");
      if (req != nullptr)
        print_error("Lines 'options debugger' and 'require daslib/debug' should be placed after all other 'require'\nAt file: '%s'",
          fname.c_str());
    }
    p++;
  }
}
*/

static bool compare_identifier(const char * s, const char * word, int len)
{
  if (strncmp(s, word, len) != 0)
    return false;

  if ((isalnum(s[-1]) || s[-1] == '_') || (isalnum(s[len]) || s[len] == '_'))
    return false;

  return true;
}

static void check_source_for_special_markers(const das::string & fname, char * s)
{
  bool tag = false;
  bool options = false;
  int tagDepth = 0;
  
  for (char * p = s; *p; p++)
  {
    if (*p == '/' && p[1] == '/')
      while (*p && *p != '\n')
        p++;

    if (*p == '/' && p[1] == '*')
      while (*p)
      {
        if (*p == '*' && p[1] == '/')
        {
          p += 2;
          break;
        }
        p++;
      }

    if (*p == '\'')
    {
      p++;
      while (*p)
      {
        if (*p == '\'')
          break;
        if (*p == '\\')
          p++;
        p++;
      }
    }

    if (*p == '\"')
    {
      int depth = 0;
      p++;
      while (*p)
      {
        if (*p == '{')
          depth++;
        if (*p == '}')
          depth--;
        if (*p == '\"' && depth <= 0)
          break;
        if (*p == '\\')
          p++;
        p++;
      }
    }

    if (*p == '\n')
      options = false;

    if (*p == '\n' && p[1] == '[')
    {
      tagDepth = 0;
      tag = true;
    }

    if (*p == '[' && tag)
      tagDepth++;

    if (*p == ']' && tag)
    {
      tagDepth--;
      if (tagDepth <= 0)
        tag = false;
    }

    if (tag && *p == 'e' && !program_has_unsafe_operations)
      if (compare_identifier(p, "extern", sizeof("extern") - 1))
      {
        program_has_unsafe_operations = true;
        print_note("'extern' used in file '%s'", fname.c_str());
      }

    if (*p == '\n' && strncmp(p + 1, "options ", sizeof("options ") - 1) == 0)
      options = true;

    if (options && !is_in_debug_mode && *p == 'd')
      if (compare_identifier(p, "debugger", sizeof("debugger") - 1))
      {
        is_in_debug_mode = true;
        print_note("Debug mode was enabled in file '%s'", fname.c_str());
      }

    if (*p == 'u' && p[1] == 'n' && !program_has_unsafe_operations && !tag && p > s)
      if (compare_identifier(p, "unsafe", sizeof("unsafe") - 1))
        {
          program_has_unsafe_operations = true;
          print_note("'unsafe' used in file '%s'", fname.c_str());
        }
  }
}


das::FileInfo * DasboxFsFileAccess::getNewFileInfo(const das::string & fname)
{
  /*{
    FILE * f = fopen("d://1.txt", "at");
    if (f)
    {
      fprintf(f, "getNewFileInfo:  %s\n", fname.c_str());
      fclose(f);
    }
  }*/


  FILE * f = fopen(fname.c_str(), "rb");
  if (!f)
  {
    string key = fname;
    if (!strncmp(fname.c_str(), "daslib/", 7) || strstr(fname.c_str(), "/daslib/") || strstr(fname.c_str(), "\\daslib/"))
    {
      if (const char * p = strrchr(fname.c_str(), '/'))
        key = string("daslib/") + (p + 1);
    }

    if (!resources_fs.isFileExists(key.c_str()))
    {
      print_error("File '%s' not found", fname.c_str());
      return nullptr;
    }
    else
    {
      int64_t len = resources_fs.getFileSize(key.c_str());
      char * source = (char *)das_aligned_alloc16(len);
      if (!resources_fs.getFileBytes(key.c_str(), source, len))
      {
        das_aligned_free16(source);
        return nullptr;
      }

      auto info = das::make_unique<das::TextFileInfo>(source, len, true);
      return setFileInfo(fname, std::move(info));
    }
    return nullptr;
  }

  fseek(f, 0, SEEK_END);
  const uint32_t fileLength = uint32_t(ftell(f));
  fseek(f, 0, SEEK_SET);

  char * source = (char *)das_aligned_alloc16(fileLength + 1);
  if (fread((void*)source, 1, fileLength, f) != fileLength)
  {
    fclose(f);
    free(source);
    print_error("Cannot read file '%s'", fname.c_str());
    return nullptr;
  }

  fclose(f);

  source[fileLength] = 0;
  //check_source_for_correct_debug_position(fname, source);
  check_source_for_special_markers(fname, source);

  auto info = das::make_unique<das::TextFileInfo>(source, fileLength, true);
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
  if (trust_mode)
    return true;

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

std::string get_current_dir()
{
  char buffer[512] = { 0 };
  return std::string(getcwd(buffer, sizeof(buffer)));
}

bool is_file_exists(const char * file_name)
{
  if (!file_name)
    return false;

  if (resources_fs.isFileExists(file_name))
    return true;

  struct stat buffer;
  return stat(file_name, &buffer) == 0;
}

uint64_t get_file_time(const char * file_name)
{
  if (!file_name)
    return 0;
  struct stat buf;
  if (!stat(file_name, &buf))
    return uint64_t(buf.st_mtime);
  return 0;
}


void enumerate_files(const char * path, vector<string> & files)
{
  files.clear();
  if (!path || !path[0])
    path = ".";

#ifdef _WIN32
  _finddata_t c_file;
  intptr_t hFile;
  string findPath = string(path) + "/*";
  if ((hFile = _findfirst(findPath.c_str(), &c_file)) != -1L)
  {
    do
    {
      files.push_back(string(c_file.name));
    } while (_findnext(hFile, &c_file) == 0);
  }
  _findclose(hFile);
#else
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir(path)) != NULL)
  {
    while ((ent = readdir(dir)) != NULL)
      files.push_back(string(ent->d_name));
    closedir (dir);
  }
#endif

}


string find_main_das_file_in_directory(const char * path)
{
  if (!path || !path[0])
    path = ".";

  const char * p = std::max(strrchr(path, '\\'), strrchr(path, '/'));
  string projectName(p ? string(p + 1) : path);

  string t = string(path) + "/main.das";
  if (fs::is_file_exists(t.c_str()))
    return t;

  vector<string> files;
  enumerate_files(path, files);
  sort(files.begin(), files.end());

  for (auto && fn : files)
  {
    const char * ptr = strstr(fn.c_str(), "_main.das");
    if (ptr && !ptr[9])
      return string(path) + "/" + fn;
  }

  for (auto && fn : files)
  {
    const char * ptr = strstr(fn.c_str(), ".das");
    if (ptr && !ptr[4])
    {
      string data = read_whole_file((string(path) + "/" + fn).c_str());
      if (strstr(data.c_str(), "\ndef initialize") &&
          strstr(data.c_str(), "\ndef draw") &&
          strstr(data.c_str(), "\ndef act") &&
          strstr(data.c_str(), "\n[export]"))
      {
        return string(path) + "/" + fn;
      }
    }
  }

  return string();
}



#ifdef _WIN32

string convert_to_utf8(wchar_t * ws)
{
  wstring_convert<codecvt_utf8<wchar_t>> utf8_conv;
  wstring str = ws;
  return utf8_conv.to_bytes(str);
}

string get_user_data_dir()
{
  wchar_t buf[MAX_PATH + 1];
  wchar_t shortBuf[MAX_PATH + 1];
  buf[0] = 0;
  shortBuf[0] = 0;
  HRESULT hr = SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, buf);
  GetShortPathNameW(buf, shortBuf, MAX_PATH);
  string s = convert_to_utf8(shortBuf);
  if (!FAILED(hr))
  {
    s += "\\dasbox";
    if (!is_file_exists(s.c_str()))
      mkdir(s.c_str());
    return s + '\\';
  }

  print_error("Cannot access 'Local AppData' (%s)", s.c_str());
  return string("");
}

#else

string get_user_data_dir()
{
  string dir;
  const char * xdgDocumentsDir = getenv("XDG_DATA_HOME");
  const char * homeDir = getenv("HOME");
  if (xdgDocumentsDir)
    dir = string(xdgDocumentsDir) + "/dasbox";
  else if (homeDir)
    dir = string(homeDir) + "/.local/share/dasbox";
  else
  {
    dir = get_current_dir() + "/.config";
    if (!fs::is_file_exists(dir.c_str()))
      mkdir(dir.c_str(), 0777);
    dir += "/dasbox";
  }

  if (!fs::is_file_exists(dir.c_str()))
    mkdir(dir.c_str(), 0777);

  return dir + '/';
}

#endif

}
