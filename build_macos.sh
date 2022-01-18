#!/bin/bash

# ============ resources =============
pushd src/resources
./stringify_resources_fs8_linux.sh
popd

# ============ dasbox ================
rm -rf ./cmake_tmp/ 2>/dev/null
mkdir cmake_tmp
pushd cmake_tmp
#cmake -G "Unix Makefiles" -DDASBOX_USE_STATIC_STD_LIBS:BOOL=TRUE -DCMAKE_OSX_ARCHITECTURES="arm64" ..
#make all -j 8
cmake -G Xcode -DDASBOX_USE_STATIC_STD_LIBS:BOOL=TRUE -DCMAKE_OSX_ARCHITECTURES="arm64" ..
cmake --build . --target dasbox --config Debug
popd
