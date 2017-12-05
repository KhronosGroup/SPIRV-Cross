#!/bin/bash

echo "Building spirv-cross"
make -j$(nproc)

export PATH="./external/glslang-build/StandAlone:./external/spirv-tools-build/tools:$PATH"
echo "Using glslangValidation in: $(which glslangValidator)."
echo "Using spirv-opt in: $(which spirv-opt)."

./test_shaders.py shaders --update
./test_shaders.py shaders --update --opt
./test_shaders.py shaders-msl --msl --update
./test_shaders.py shaders-msl --msl --update --opt
./test_shaders.py shaders-hlsl --hlsl --update
./test_shaders.py shaders-hlsl --hlsl --update --opt
