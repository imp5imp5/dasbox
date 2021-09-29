@if [%CMAKE_GEN_TARGET%] == [] (
  exit/b
)

@echo off

if exist "%programfiles(x86)%\Microsoft Visual Studio\%VS_NUMBER%\Professional" (
  call   "%programfiles(x86)%\Microsoft Visual Studio\%VS_NUMBER%\Professional\VC\Auxiliary\Build\vcvarsall.bat" x86_x64
  goto on_found
) 
if exist "%programfiles(x86)%\Microsoft Visual Studio\%VS_NUMBER%\Enterprise" (
  call   "%programfiles(x86)%\Microsoft Visual Studio\%VS_NUMBER%\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x86_x64
  goto on_found
) 
if exist "%programfiles(x86)%\Microsoft Visual Studio\%VS_NUMBER%\Community" (
  call   "%programfiles(x86)%\Microsoft Visual Studio\%VS_NUMBER%\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_x64
  goto on_found
) 
if exist "%programfiles(x86)%\Microsoft Visual Studio\%VS_NUMBER%\WDExpress" (
  call   "%programfiles(x86)%\Microsoft Visual Studio\%VS_NUMBER%\WDExpress\VC\Auxiliary\Build\vcvarsall.bat" x86_x64
  goto on_found
) 
if exist "%programfiles%\Microsoft Visual Studio\%VS_NUMBER%\Professional" (
  call   "%programfiles%\Microsoft Visual Studio\%VS_NUMBER%\Professional\VC\Auxiliary\Build\vcvarsall.bat" x86_x64
  goto on_found
) 
if exist "%programfiles%\Microsoft Visual Studio\%VS_NUMBER%\Enterprise" (
  call   "%programfiles%\Microsoft Visual Studio\%VS_NUMBER%\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x86_x64
  goto on_found
) 
if exist "%programfiles%\Microsoft Visual Studio\%VS_NUMBER%\Community" (
  call   "%programfiles%\Microsoft Visual Studio\%VS_NUMBER%\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_x64
  goto on_found
) 
if exist "%programfiles%\Microsoft Visual Studio\%VS_NUMBER%\WDExpress" (
  call   "%programfiles%\Microsoft Visual Studio\%VS_NUMBER%\WDExpress\VC\Auxiliary\Build\vcvarsall.bat" x86_x64
  goto on_found
) 


@echo.
@echo ERROR: Visual Studio %VS_NUMBER% not found
@exit/b

:on_found


set "INCLUDE=%CD%\src;%INCLUDE%"


rem ============ daScript ==============
pushd 3rdParty\daScript
rd /S /Q build
mkdir build
pushd build
cmake -G %CMAKE_GEN_TARGET% -DDAS_USE_STATIC_STD_LIBS:BOOL=TRUE ..
msbuild libDaScript.vcxproj /p:Configuration=Release
popd
popd

rem ============ SFML ==================
pushd 3rdParty\SFML
rd /S /Q build
mkdir build
pushd build
cmake -G %CMAKE_GEN_TARGET% -DSFML_BUILD_EXAMPLES=FALSE -DBUILD_SHARED_LIBS:BOOL=FALSE -DSFML_USE_STATIC_STD_LIBS:BOOL=TRUE ..
msbuild ALL_BUILD.vcxproj /p:Configuration=Release
popd
popd

rem ============ dasbox ================
rd /S /Q cmake_tmp
mkdir cmake_tmp
pushd cmake_tmp
cmake -G %CMAKE_GEN_TARGET% -DDASBOX_USE_STATIC_STD_LIBS:BOOL=TRUE ../src
msbuild dasbox.vcxproj /p:Configuration=Release
popd

rem ============ copy ==================
mkdir bin
copy cmake_tmp\Release\dasbox.exe bin


