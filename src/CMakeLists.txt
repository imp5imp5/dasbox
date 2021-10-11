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


if(WIN32)

  if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_executable(dasbox ${SRC})
  else()
    add_executable(dasbox WIN32 ${SRC})
  endif()

  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT dasbox)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
  target_link_libraries(
    dasbox
    legacy_stdio_definitions
    winmm
    Opengl32
    libDaScript
    libUriParser
    libDasModuleUriparser
    freetype
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
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fno-rtti -fno-exceptions -fomit-frame-pointer -fno-stack-protector -s -DDAS_FUSION=2 -DDAS_DEBUGGER=0 -DNDEBUG=1")
  target_link_libraries(
    dasbox
    libDaScript
    libUriParser
    libDasModuleUriparser
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

