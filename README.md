# dasbox

## Build for Windows [![Build status](https://ci.appveyor.com/api/projects/status/9bbceuifufc846v0/branch/main?svg=true)](https://ci.appveyor.com/project/imp5imp5/dasbox/branch/main)

Install Visual Studio 2017 or 2019\
Install CMake https://cmake.org/install/ \
`git clone --recurse-submodules https://github.com/imp5imp5/dasbox.git`\
`cd dasbox`\
`build_windows_vs_*.bat`


## Build for Linux (tested only on Ubuntu) [![Build status](https://ci.appveyor.com/api/projects/status/pfvxn3u8davbb8rj/branch/main?svg=true)](https://ci.appveyor.com/project/imp5imp5/dasbox-linux/branch/main)
`sudo apt update`\
`sudo apt install g++ make`

update CMake (3.21.3 or newer version required) https://cmake.org/install/

`sudo apt install libudev-dev libasound2-dev libpulse-dev libx11-dev mesa-common-dev libgl1-mesa-dev libglu1-mesa-dev libxrandr-dev libxcursor-dev libfreetype-dev`\
\
`git clone --recurse-submodules https://github.com/imp5imp5/dasbox.git `\
`cd dasbox`\
`./build_linux.sh`

## How to use

Run your application:

  `dasbox.exe <path_to_application_folder/file_name.das>`

Optional comand line arguments:

  `--dasbox-console - duplicate output to console ('--' is also acceptable)` \
  `--trust - allow access to any file on this computer`

Once the application is running, all the sources ('file_name.das' and all the sources requested from it) will be checked for changes and automatically reloaded. \
The current directory will change to 'path_to_application_folder'. For security reasons access to parent directories from within the script will be forbidden.

**F5** or **Ctrl+R** - reload sources in the manual mode \
**Ctrl+F5** or **Ctrl+Alt+R** - hard reload, **ECS** will be reloaded too \
**Tab** - switch to the logging screen and back

In the log screen your application will be paused.

Controls in the log screen:

  **Up, Down, Ctrl+Up, Ctrl+Down, PgUp, PgDown, Mouse Scroll** - scroll log \
  **Left Mouse Button** - text selection \
  **Ctrl+C** - copy to clipboard
  

## More to read and watch

* See [API][api]

* Watch [videos][]

* See [roadmap][roadmap]

* See [how to setup VS Code][vscodesetup]



[videos]: https://www.youtube.com/playlist?list=PL6Ke-5R5eg2I7oVLR7TJIT5Q0ikGecVrT
[api]: https://github.com/imp5imp5/dasbox/blob/main/doc/api.txt
[vscodesetup]: https://github.com/imp5imp5/dasbox/blob/main/doc/vscode_setup.txt
[roadmap]: https://github.com/imp5imp5/dasbox/blob/main/doc/roadmap.md
