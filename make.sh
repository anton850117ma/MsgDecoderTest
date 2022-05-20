#!/bin/bash

# export CC=/usr/bin/clang
# export CXX=/usr/bin/clang++

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j 4
ctest --test-dir build -j 4