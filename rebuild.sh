#!/bin/bash

set -e
set -x

rm -rf build
mkdir build

pushd build

# can use --build missing
CXX=/usr/bin/g++-7 CC=/usr/bin/gcc-7 CONAN_CPU_COUNT=12 conan install .. -s build_type=Debug --build "missing" --profile pcpd
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc-7 -DCMAKE_CXX_COMPILER=g++-7
cmake --build . -j 12

popd
