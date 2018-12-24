#! /bin/bash
mkdir -p build
pushd build
cmake ../src
make 
# make install
popd