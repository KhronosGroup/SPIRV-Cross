#!/bin/bash

GLSLANG_REV=698bf7547a96b6feb7291e8ddc0d5d16475dbae2
SPIRV_TOOLS_REV=e28edd458b729da7bbfd51e375feb33103709e6f

if [ -d external/glslang ]; then
	echo "Updating glslang to revision $GLSLANG_REV."
	cd external/glslang
	git fetch origin
	git checkout $GLSLANG_REV
else
	echo "Cloning glslang revision $GLSLANG_REV."
	mkdir -p external
	cd external
	git clone git://github.com/KhronosGroup/glslang.git
	cd glslang
	git checkout $GLSLANG_REV
fi
cd ../..

echo "Building glslang."
mkdir -p external/glslang-build
cd external/glslang-build
cmake ../glslang -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cd ../..

if [ -d external/spirv-tools ]; then
	echo "Updating SPIRV-Tools to revision $SPIRV_TOOLS_REV."
	cd external/spirv-tools
	git fetch origin
	git checkout $SPIRV_TOOLS_REV
else
	echo "Cloning SPIRV-Tools revision $SPIRV_TOOLS_REV."
	mkdir -p external
	cd external
	git clone git://github.com/KhronosGroup/SPIRV-Tools.git spirv-tools
	cd spirv-tools
	git checkout $SPIRV_TOOLS_REV

	if [ -d external/spirv-headers ]; then
		cd external/spirv-headers
		git pull origin master
		cd ../..
	else
		git clone git://github.com/KhronosGroup/SPIRV-Headers.git external/spirv-headers
	fi
fi
cd ../..

echo "Building SPIRV-Tools."
mkdir -p external/spirv-tools-build
cd external/spirv-tools-build
cmake ../spirv-tools -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cd ../..

