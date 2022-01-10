#!/bin/bash
CPU_COUNT=`nproc --all`

# ============ dasbox ================
rm -rf ./cmake_tmp/ 2>/dev/null
mkdir cmake_tmp
pushd cmake_tmp
cmake -G "Unix Makefiles" -DDASBOX_USE_STATIC_STD_LIBS:BOOL=TRUE ..
make all -j $CPU_COUNT
popd
