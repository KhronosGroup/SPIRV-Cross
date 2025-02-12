#!/bin/bash
# Copyright 2016-2021 The Khronos Group Inc.
# SPDX-License-Identifier: Apache-2.0

GLSLANG_REV=b3a6aa7b03c51ba976e4f4e96b1e31f77f43f312
SPIRV_TOOLS_REV=4d2f0b40bfe290dea6c6904dafdf7fd8328ba346
SPIRV_HEADERS_REV=3f17b2af6784bfa2c5aa5dbb8e0e74a607dd8b3b
PROTOCOL=https

if [ -d external/glslang ]; then
	echo "Updating glslang to revision $GLSLANG_REV."
	cd external/glslang
	git fetch origin
	git checkout $GLSLANG_REV
else
	echo "Cloning glslang revision $GLSLANG_REV."
	mkdir -p external
	cd external
	git clone $PROTOCOL://github.com/KhronosGroup/glslang.git
	cd glslang
	git checkout $GLSLANG_REV
fi
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
	git clone $PROTOCOL://github.com/KhronosGroup/SPIRV-Tools.git spirv-tools
	cd spirv-tools
	git checkout $SPIRV_TOOLS_REV
fi

if [ -d external/spirv-headers ]; then
	cd external/spirv-headers
	git fetch origin
	git checkout $SPIRV_HEADERS_REV
	cd ../..
else
	git clone $PROTOCOL://github.com/KhronosGroup/SPIRV-Headers.git external/spirv-headers
	cd external/spirv-headers
	git checkout $SPIRV_HEADERS_REV
	cd ../..
fi

cd ../..

