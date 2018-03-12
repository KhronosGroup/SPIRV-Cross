#!/bin/bash

echo "Building glslang."
mkdir -p external/glslang-build
cd external/glslang-build
cmake ../glslang -DCMAKE_BUILD_TYPE=Release -G"Unix Makefiles"
make -j$(nproc)
cd ../..

echo "Building SPIRV-Tools."
mkdir -p external/spirv-tools-build
cd external/spirv-tools-build
cmake ../spirv-tools -DCMAKE_BUILD_TYPE=Release -G"Unix Makefiles"
make -j$(nproc)
cd ../..

