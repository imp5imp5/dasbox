@if [%CMAKE_GEN_TARGET: =%] == [] (
  exit/b
)

@echo off

if [%CONFIGURATION%] == [] (
  set CONFIGURATION=Release
)

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
if exist "%programfiles(x86)%\Microsoft Visual Studio\%VS_NUMBER%\BuildTools" (
  call   "%programfiles(x86)%\Microsoft Visual Studio\%VS_NUMBER%\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86_x64
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
if exist "%programfiles%\Microsoft Visual Studio\%VS_NUMBER%\BuildTools" (
  call   "%programfiles%\Microsoft Visual Studio\%VS_NUMBER%\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86_x64
  goto on_found
) 


@echo.
@echo ERROR: Visual Studio %VS_NUMBER% not found
@exit/b

:on_found


set "INCLUDE=%CD%\src;%INCLUDE%"

for /f %%x in ('wmic path win32_localtime get /format:list ^| findstr "="') do set %%x
echo #pragma once> src\buildDate.h
echo #define DASBOX_BUILD_DATE "%Day%.%Month%.%Year%">> src\buildDate.h


rem ============ dasbox ================
rd /S /Q cmake_tmp
mkdir build
pushd build
cmake -G %CMAKE_GEN_TARGET% -DDASBOX_USE_STATIC_STD_LIBS:BOOL=TRUE ..
msbuild dasbox.vcxproj /p:Configuration=%CONFIGURATION%
popd
