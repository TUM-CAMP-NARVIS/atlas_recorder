#!/bin/bash

set -e
set -x

rm -rf build
mkdir build

pushd build

# can use --build missing
CXX=/usr/bin/g++-9 CC=/usr/bin/gcc-9 CONAN_CPU_COUNT=12 conan install .. -s build_type=Debug --build "missing" --profile dev
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc-9 -DCMAKE_CXX_COMPILER=g++-9
cmake --build . -j 12

popd
