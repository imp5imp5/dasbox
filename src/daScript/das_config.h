#pragma once

#ifndef DAS_SKA_HASH
#define DAS_SKA_HASH    1
#endif

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <string>
#include <memory>
#include <vector>
#include <type_traits>
#include <initializer_list>
#include <functional>
#include <algorithm>
#include <functional>
#include <climits>

#include <limits.h>
#include <setjmp.h>

#include <mutex>

namespace das {using namespace std;}

void print_error(const char * format, ...);
namespace fs { bool is_path_string_valid(const char * path); }

#ifdef BUILDING_DASBOX
#  define DAS_IS_PATH_VALID(path) fs::is_path_string_valid(path)
#else
#  define DAS_IS_PATH_VALID(path) !!path
#endif

#ifndef DAS_STD_HAS_BIND
#  define DAS_STD_HAS_BIND 1
#endif

#ifndef DAS_FUSION
#  define DAS_FUSION  1
#endif

#ifndef DAS_DEBUGGER
#  define DAS_DEBUGGER  1
#endif

#ifndef DAS_TRACK_ALLOCATIONS
#  define DAS_TRACK_ALLOCATIONS  0
#endif

#ifndef DAS_SANITIZER
#  define DAS_SANITIZER  0
#endif


#ifndef DAS_FATAL_LOG
#  define DAS_FATAL_LOG  print_error
#endif

#ifndef DAS_FATAL_ERROR
#  define DAS_FATAL_ERROR  print_error
#endif


#ifndef DAS_BIND_EXTERNAL
  #if defined(_WIN32) && defined(_WIN64)
    #define DAS_BIND_EXTERNAL 1
  #elif defined(__APPLE__)
    #define DAS_BIND_EXTERNAL 1
  #else
    #define DAS_BIND_EXTERNAL 0
  #endif
#endif


namespace das { using namespace std; }

#if DAS_SKA_HASH
#ifdef _MSC_VER
#pragma warning(disable:4503)    // decorated name length exceeded, name was truncated
#endif
#include <ska/flat_hash_map.hpp>
namespace das {
template <typename K, typename V, typename H = das::hash<K>, typename E = das::equal_to<K>>
using das_map = das_ska::flat_hash_map<K,V,H,E>;
template <typename K, typename H = das::hash<K>, typename E = das::equal_to<K>>
using das_set = das_ska::flat_hash_set<K,H,E>;
template <typename K, typename V, typename H = das::hash<K>, typename E = das::equal_to<K>>
using das_hash_map = das_ska::flat_hash_map<K,V,H,E>;
template <typename K, typename H = das::hash<K>, typename E = das::equal_to<K>>
using das_hash_set = das_ska::flat_hash_set<K,H,E>;
template <typename K, typename V>
using das_safe_map = std::map<K,V>;
template <typename K, typename C=das::less<K>>
using das_safe_set = std::set<K,C>;
}
#else
namespace das {
template <typename K, typename V, typename H = das::hash<K>, typename E = das::equal_to<K>>
using das_map = std::unordered_map<K,V,H,E>;
template <typename K, typename H = das::hash<K>, typename E = das::equal_to<K>>
using das_set = std::unordered_set<K,H,E>;
template <typename K, typename V, typename H = das::hash<K>, typename E = das::equal_to<K>>
using das_hash_map = std::unordered_map<K,V,H,E>;
template <typename K, typename H = das::hash<K>, typename E = das::equal_to<K>>
using das_hash_set = std::unordered_set<K,H,E>;
template <typename K, typename V>
using das_safe_map = std::map<K,V>;
template <typename K, typename C=das::less<K>>
using das_safe_set = std::set<K,C>;
}
#endif


#ifndef DAS_BIND_EXTERNAL
  #if defined(_WIN32) && defined(_WIN64)
    #define DAS_BIND_EXTERNAL 1
  #elif defined(__APPLE__)
    #define DAS_BIND_EXTERNAL 1
  #elif defined(__linux__)
    #define DAS_BIND_EXTERNAL 1
  #else
    #define DAS_BIND_EXTERNAL 0
  #endif
#endif

#ifndef DAS_PRINT_VEC_SEPARATROR
#define DAS_PRINT_VEC_SEPARATROR ","
#endif


#ifndef das_to_stdout
#define das_to_stdout(...) { fprintf(stdout, __VA_ARGS__); fflush(stdout); }
#endif

#ifndef das_to_stderr
#define das_to_stderr(...) { fprintf(stderr, __VA_ARGS__); fflush(stderr); }
#endif


inline int das_soft_popcnt(uint32_t v)
{
  v = v - ((v >> 1) & 0x55555555);
  v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
  return int(((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24);
}

inline int das_soft_popcnt64(uint64_t v)
{
  v = v - ((v >> 1) & 0x5555555555555555ULL);
  v = (v & 0x3333333333333333ULL) + ((v >> 2) & 0x3333333333333333ULL);
  v = (v + (v >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
  return int((uint64_t)(v * 0x0101010101010101ULL) >> 56);
}







