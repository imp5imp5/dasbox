if not exist ..\..\3rdParty\fs8\build\Release\fs8pack.exe (
  pushd ..\..\3rdParty\fs8
  call build_2019.bat
  popd
)

if not exist ..\..\3rdParty\fs8\build\Release\fs8pack.exe (
  pushd ..\..\3rdParty\fs8
  call build_2017.bat
  popd
)

if not exist ..\..\3rdParty\fs8\build\Release\fs8pack.exe (
  pushd ..\..\3rdParty\fs8
  rd /s /q build
  mkdir build
  pushd build
  cmake -G "Visual Studio 17" -A x64 ..
  cmake --build . --config Release
  popd
  popd
)


..\..\3rdParty\fs8\build\Release\fs8pack.exe --hex --list:pack_files.txt . resources.inc
