if ! [ -f ../../3rdParty/fs8/build/Release/fs8pack ]; then
  pushd ../../3rdParty/fs8
  ./build_macos.sh
  popd
fi

../../3rdParty/fs8/build/Release/fs8pack --hex --list:pack_files.txt . resources.inc
