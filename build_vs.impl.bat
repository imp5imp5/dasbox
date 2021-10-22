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


rem ============ daScript ==============
pushd 3rdParty\daScript
rd /S /Q build
mkdir build
pushd build
cmake -G %CMAKE_GEN_TARGET% -DDAS_BUILD_TOOLS:BOOL=NO -DDAS_BUILD_TEST:BOOL=NO -DDAS_BUILD_PROFILE:BOOL=NO -DDAS_BUILD_TUTORIAL:BOOL=NO -DDAS_CONFIG_INCLUDE_DIR:STRING="%CD%/src" -DCMAKE_CXX_FLAGS_DEBUG:STRING="/MTd /Od /DDAS_FUSION=1 /DDAS_DEBUGGER=1 /Zi /EHa" ..
msbuild libDaScript.vcxproj /p:Configuration=%CONFIGURATION%
msbuild libDasModuleUriparser.vcxproj /p:Configuration=%CONFIGURATION%
msbuild libUriParser.vcxproj /p:Configuration=%CONFIGURATION%
popd
popd

rem ============ SFML ==================
pushd 3rdParty\SFML
rd /S /Q build
mkdir build
pushd build
cmake -G %CMAKE_GEN_TARGET% -DSFML_BUILD_EXAMPLES=FALSE -DBUILD_SHARED_LIBS:BOOL=FALSE -DSFML_USE_STATIC_STD_LIBS:BOOL=TRUE ..
msbuild ALL_BUILD.vcxproj /p:Configuration=%CONFIGURATION%
popd
popd

rem ============ dasbox ================
rd /S /Q cmake_tmp
mkdir cmake_tmp
pushd cmake_tmp
cmake -G %CMAKE_GEN_TARGET% -DDASBOX_USE_STATIC_STD_LIBS:BOOL=TRUE ../src
msbuild dasbox.vcxproj /p:Configuration=%CONFIGURATION%
popd

rem ============ copy ==================
mkdir bin
copy cmake_tmp\%CONFIGURATION%\dasbox.exe bin


