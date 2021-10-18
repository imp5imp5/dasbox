pushd build
cmake ../
make -j ${nproc}
make install
popd
