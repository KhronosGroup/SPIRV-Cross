# Copyright 2016 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 2.8)
project(SPIRV-Cross)
enable_testing()

if(${CMAKE_GENERATOR} MATCHES "Makefile")
  if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
    message(FATAL_ERROR "Build out of tree to avoid overwriting Makefile")
  endif()
endif()

add_executable(spirv-cross
	${CMAKE_CURRENT_SOURCE_DIR}/GLSL.std.450.h
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_common.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_cpp.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_glsl.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_msl.hpp

	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_cpp.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_glsl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_msl.cpp
	)

# To specify special debug or optimization options, use
# -DCMAKE_CXX_COMPILE_FLAGS
# However, we require the C++11 dialect.
if (NOT "${MSVC}")
  target_compile_options(spirv-cross PRIVATE -std=c++11 -Wall -Wextra -Werror -Wshadow)
endif(NOT "${MSVC}")


# Set up tests, using only the simplest modes of the test_shaders
# script.  You have to invoke the script manually to:
#  - Update the reference files
#  - Get cycle counts from malisc
#  - Keep failing outputs
find_program(PYTHON3_EXE python3)
if(${PYTHON3_EXE} MATCHES "NOTFOUND")
  message(WARNING "Testing disabled. Could not find python3")
else()
  add_test(NAME spirv-cross-test
	   COMMAND ${PYTHON3_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/test_shaders.py
		   ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
endif()
