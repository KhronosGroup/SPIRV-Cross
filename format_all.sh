#!/bin/bash

for file in spirv_*.{cpp,hpp} include/spirv_cross/*.{hpp,h}
do
    echo "Formatting file: $file ..."
    clang-format -style=file -i $file
done
