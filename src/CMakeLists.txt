cmake_minimum_required(VERSION 3.15)
project(dasbox)
set(CMAKE_CONFIGURATION_TYPES "Release" "Debug")

if(MSVC)
  add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)
  if(DASBOX_USE_STATIC_STD_LIBS)
    add_compile_options(
      "$<$<CONFIG:RELEASE>:/MT>"
      "$<$<CONFIG:DEBUG>:/MTd>"
    )
  endif()
endif()

add_definitions(-DSFML_STATIC)
add_definitions(-DMINFFT_SINGLE)


file(GLOB SRC
  *.cpp
  *.h
  ../3rdParty/minfft/minfft.c
  ../3rdParty/fs8/library/fs8.cpp
)

include_directories(
  .
  ../3rdParty/fs8/library
  ../3rdParty/minfft
  ../3rdParty/miniaudio
  ../3rdParty/SFML/include
  ../3rdParty/daScript/include
  ../3rdParty/zstd/lib
)

link_directories(
  ../3rdParty/daScript/build
  ../3rdParty/SFML/build/lib
  ../3rdParty/zstd/build/cmake/build/lib
)

if(MSVC)
  set(SRC ${SRC} icon/icon.rc)
  link_directories(../3rdParty/SFML/extlibs/libs-msvc/x64)
endif()


if(WIN32)
  add_executable(dasbox ${SRC})
  target_compile_features(dasbox PRIVATE cxx_std_17)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT dasbox)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
  target_link_libraries(
    dasbox
    legacy_stdio_definitions
    winmm
    Opengl32
    libUriParser
    libDaScript
    freetype
    zstd_static
    optimized sfml-main
    optimized sfml-graphics-s
    optimized sfml-network-s
    optimized sfml-system-s
    optimized sfml-window-s
    debug sfml-main-d
    debug sfml-graphics-s-d
    debug sfml-network-s-d
    debug sfml-system-s-d
    debug sfml-window-s-d
  )
endif()

if(UNIX)
  add_executable(dasbox ${SRC})
  target_compile_features(dasbox PRIVATE cxx_std_17)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fno-rtti -fno-exceptions -fomit-frame-pointer -fno-stack-protector -s -DDAS_FUSION=2 -DDAS_DEBUGGER=1 -DNDEBUG=1")
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -fno-rtti -fno-exceptions -fomit-frame-pointer -fno-stack-protector")
  target_link_libraries(
    dasbox
    libUriParser
    libDaScript
    optimized sfml-network-s
    optimized sfml-graphics-s
    optimized sfml-window-s
    optimized sfml-system-s
    debug sfml-network-s-d
    debug sfml-graphics-s-d
    debug sfml-window-s-d
    debug sfml-system-s-d
    zstd_static
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
