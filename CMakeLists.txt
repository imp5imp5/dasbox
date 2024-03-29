cmake_minimum_required(VERSION 3.15)
project(dasbox)
set(CMAKE_CONFIGURATION_TYPES "Release" "Debug")


if (MSVC)
  if (USE_MSVC_RUNTIME_LIBRARY_DLL)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  else()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  endif()
endif()


if(MSVC)
  add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)
endif()

add_definitions(-DSFML_STATIC)
add_definitions(-DMINFFT_SINGLE)


# daScript -------------------------------

set(DAS_BUILD_TEST NO)
set(DAS_BUILD_PROFILE NO)
set(DAS_BUILD_TUTORIAL NO)
set(DAS_GLFW_DISABLED YES)
set(DAS_IMGUI_DISABLED YES)
set(DAS_CLANG_BIND_DISABLED YES)
set(DAS_BGFX_DISABLED YES)
set(DAS_STBIMAGE_DISABLED YES)
set(DAS_STBTRUETYPE_DISABLED YES)
set(DAS_STDDLG_DISABLED YES)
set(DAS_XBYAK_DISABLED YES)
set(DAS_SFML_DISABLED YES)
set(DAS_FLEX_BISON_DISABLED YES)
set(DAS_HV_DISABLED YES)
set(DAS_MINFFT_DISABLED YES)
set(DAS_SOUND_DISABLED YES)
# set(DAS_BUILD_TOOLS NO) # <--- need to build tools to support AOT generation build step in daScript modules
set(DAS_CONFIG_INCLUDE_DIR "src")

add_subdirectory(3rdParty/daScript)


# zstd -------------------------------

set(ZSTD_BUILD_PROGRAMS OFF)
set(ZSTD_BUILD_SHARED OFF)
set(ZSTD_MULTITHREAD_SUPPORT ON)
set(ZSTD_USE_STATIC_RUNTIME ON)

add_subdirectory(3rdParty/zstd/build/cmake)


# SFML -------------------------------
set(SFML_BUILD_EXAMPLES FALSE)
set(SFML_BUILD_AUDIO FALSE)
set(BUILD_SHARED_LIBS FALSE)
set(SFML_USE_STATIC_STD_LIBS TRUE)

add_subdirectory(3rdParty/SFML)


file(GLOB SRC
  src/*.cpp
  src/*.h
  3rdParty/minfft/minfft.c
  3rdParty/fs8/library/fs8.cpp
)

include_directories(
  src
  3rdParty/fs8/library
  3rdParty/minfft
  3rdParty/miniaudio
  3rdParty/SFML/include
  3rdParty/daScript/include
  3rdParty/zstd/lib
)

link_directories(
  3rdParty/daScript/build
  3rdParty/SFML/build/lib
)

if(MSVC)
  set(SRC ${SRC} src/icon/icon.rc)
  link_directories(3rdParty/SFML/extlibs/libs-msvc/x64)
endif()

if(APPLE)
  link_directories(3rdParty/SFML/extlibs/libs-osx/Frameworks/freetype.framework)
endif()


# log implementation for standalone executables
add_library(logStandalone STATIC src/daScript/logStandalone.cpp)
add_dependencies(daScript logStandalone)
target_link_libraries(daScript logStandalone)


# dasbox
add_executable(dasbox ${SRC})
target_compile_features(dasbox PRIVATE cxx_std_17)

# library dependencies on all platforms
target_link_libraries(dasbox
  libDaScript
  libUriParser
  libzstd_static
  sfml-graphics
  sfml-network
  sfml-system
  sfml-window
)

add_dependencies(dasbox
  libzstd_static
  sfml-graphics
  sfml-network
  sfml-system
  sfml-window
)

# platform-specific
if(WIN32)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT dasbox)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
  set(CMAKE_CXX_FLAGS "-DBUILDING_DASBOX=1")
  target_link_libraries(dasbox
    legacy_stdio_definitions
    winmm
    Opengl32
    sfml-main
  )

  add_dependencies(dasbox
    sfml-main
  )
elseif(APPLE)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fno-rtti -fno-exceptions -fomit-frame-pointer -fno-stack-protector -s -DBUILDING_DASBOX=1 -DDAS_FUSION=2 -DDAS_DEBUGGER=1 -DNDEBUG=1")
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -fno-rtti -fno-exceptions -fomit-frame-pointer -fno-stack-protector -DBUILDING_DASBOX=1")
  target_link_libraries(dasbox
    pthread
    dl
  )
elseif(UNIX)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fno-rtti -fno-exceptions -fomit-frame-pointer -fno-stack-protector -s -DBUILDING_DASBOX=1 -DDAS_FUSION=2 -DDAS_DEBUGGER=1 -DNDEBUG=1")
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -fno-rtti -fno-exceptions -fomit-frame-pointer -fno-stack-protector -DBUILDING_DASBOX=1")
  target_link_libraries(dasbox
    udev
    pthread
    GL
    dl
    X11 Xrandr Xcursor
  )
endif()


set_target_properties(dasbox PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/bin"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/bin"
)
