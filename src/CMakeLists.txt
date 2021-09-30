cmake_minimum_required(VERSION 3.15)
project(dasbox)
set(CMAKE_CONFIGURATION_TYPES "Release" "Debug")

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
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
  target_link_libraries(
    dasbox
    legacy_stdio_definitions
    winmm
    Opengl32
    sfml-main
    sfml-graphics-s
    sfml-network-s
    sfml-system-s
    sfml-window-s
    libDaScript
    freetype
  )
endif()

if(UNIX)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fno-rtti -fno-exceptions -fomit-frame-pointer -fno-stack-protector -s -DDAS_FUSION=2 -DDAS_DEBUGGER=0 -DNDEBUG=1")
  target_link_libraries(
    dasbox
    libDaScript
    sfml-network-s
    sfml-graphics-s
    sfml-window-s
    sfml-system-s
    X11
    udev
    pthread
    freetype
    GL
    dl
    Xrandr
    Xcursor
  )
endif()

