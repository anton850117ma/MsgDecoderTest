#!/bin/bash

export CC=/usr/local/bin/gcc
export CXX=/usr/local/bin/g++

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j 4
# ctest --test-dir build -j 4