cmake_minimum_required(VERSION 3.15)
project(dasbox)
set(CMAKE_CONFIGURATION_TYPES "Release" "Debug")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")

if(MSVC)
  add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)
  if(DASBOX_USE_STATIC_STD_LIBS)
    add_compile_options(/MT)
  endif()
endif()

add_definitions(-DSFML_STATIC)

file(GLOB SRC *.cpp)

include_directories(
  .
  ../3rdParty/miniaudio
  ../3rdParty/SFML/include
  ../3rdParty/daScript/include
)

link_directories(
  ../3rdParty/daScript/build
  ../3rdParty/SFML/build/lib
)

if(MSVC)
  link_directories(../3rdParty/SFML/extlibs/libs-msvc/x64)
endif()


add_executable(dasbox ${SRC})

if(WIN32)
  target_link_libraries(
    dasbox
    legacy_stdio_definitions
    winmm
    Opengl32
  )
endif()

target_link_libraries(
  dasbox
  sfml-graphics-s
  sfml-main
  sfml-network-s
  sfml-system-s
  sfml-window-s
  libDaScript
  freetype
)
