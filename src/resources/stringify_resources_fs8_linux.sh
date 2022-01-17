if ! [ -f ../../3rdParty/fs8/build/fs8pack ]; then
  pushd ../../3rdParty/fs8
  ./build_linux.sh
  popd
fi

../../3rdParty/fs8/build/fs8pack --hex --list:pack_files.txt . resources.inc
