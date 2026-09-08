#!/bin/bash

mkdir -p build
cd ./build

cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=ON

cmake --build . --parallel $(nproc) --target all
