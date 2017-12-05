#!/bin/bash

echo "Building spirv-cross"
make -j$(nproc)

export PATH="./external/glslang-build/StandAlone:./external/spirv-tools-build/tools:$PATH"
echo "Using glslangValidation in: $(which glslangValidator)."
echo "Using spirv-opt in: $(which spirv-opt)."

./test_shaders.py shaders
./test_shaders.py shaders --opt
./test_shaders.py shaders-msl --msl
./test_shaders.py shaders-msl --msl --opt
./test_shaders.py shaders-hlsl --hlsl
./test_shaders.py shaders-hlsl --hlsl --opt
