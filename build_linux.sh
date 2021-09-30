CUR_DIR=`pwd`
INCLUDE_DAS="$CUR_DIR/src"
CPU_COUNT=`nproc --all`

# ============ daScript ==============
pushd 3rdParty/daScript
rm -rf ./build/ 2>/dev/null
mkdir build
pushd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DDAS_CONFIG_INCLUDE_DIR=$INCLUDE_DAS ..
make libDaScript -j $CPU_COUNT
popd
popd

# ============ SFML ==================
pushd 3rdParty/SFML
rm -rf ./build/ 2>/dev/null
mkdir build
pushd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DSFML_BUILD_AUDIO=FALSE -DSFML_BUILD_EXAMPLES=FALSE -DBUILD_SHARED_LIBS:BOOL=FALSE ..
make all -j $CPU_COUNT
popd
popd

# ============ dasbox ================
rm -rf ./cmake_tmp/ 2>/dev/null
mkdir cmake_tmp
pushd cmake_tmp
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ../src
make all -j $CPU_COUNT
popd

# ============ copy ==================
mkdir bin 2>/dev/null
cp cmake_tmp/dasbox ./bin/


