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

..\..\3rdParty\fs8\build\Release\fs8pack.exe --hex --list:pack_files.txt . resources.inc
