# dasbox

## Build for Windows

Install Visual Studio 2017 or 2019\
Install CMake https://cmake.org/install/ \
`git clone --recurse-submodules https://github.com/imp5imp5/dasbox.git`\
`cd dasbox`\
`build_windows_vs_*.bat`


## Build for Linux (tested only on Ubuntu 20)

`sudo apt update`\
`sudo apt install g++ make cmake`\
`sudo apt install libudev-dev libasound2-dev libpulse-dev libx11-dev mesa-common-dev libgl1-mesa-dev libglu1-mesa-dev libxrandr-dev libxcursor-dev libfreetype-dev`\
\
`git clone --recurse-submodules https://github.com/imp5imp5/dasbox.git `\
`cd dasbox`\
`./build_linux.sh`
