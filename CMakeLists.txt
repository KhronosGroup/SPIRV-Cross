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
set(CMAKE_CXX_STANDARD 11)
project(SPIRV-Cross LANGUAGES CXX)
enable_testing()

option(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS "Instead of throwing exceptions assert" OFF)
option(SPIRV_CROSS_SHARED "Build the C API as a single shared library." OFF)
option(SPIRV_CROSS_STATIC "Build the C and C++ API as static libraries." ON)
option(SPIRV_CROSS_CLI "Build the CLI binary. Requires SPIRV_CROSS_STATIC." ON)

if(${CMAKE_GENERATOR} MATCHES "Makefile")
	if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
		message(FATAL_ERROR "Build out of tree to avoid overwriting Makefile")
	endif()
endif()

set(spirv-compiler-options "")
set(spirv-compiler-defines "")

if(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)
	set(spirv-compiler-defines ${spirv-compiler-defines} SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)
endif()

if (CMAKE_COMPILER_IS_GNUCXX OR (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang"))
	set(spirv-compiler-options ${spirv-compiler-options} -Wall -Wextra -Werror -Wshadow)

	if(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)
		set(spirv-compiler-options ${spirv-compiler-options} -fno-exceptions)
	endif()
elseif (MSVC)
	set(spirv-compiler-options ${spirv-compiler-options} /wd4267)
endif()

macro(extract_headers out_abs file_list)
	set(${out_abs}) # absolute paths
	foreach(_a ${file_list})
		# get_filename_component only returns the longest extension, so use a regex
		string(REGEX REPLACE ".*\\.(h|hpp)" "\\1" ext ${_a})

		# For shared library, we are only interested in the C header.
		if (SPIRV_CROSS_STATIC)
			if(("${ext}" STREQUAL "h") OR ("${ext}" STREQUAL "hpp"))
				list(APPEND ${out_abs} "${_a}")
			endif()
		else()
			if("${ext}" STREQUAL "h")
				list(APPEND ${out_abs} "${_a}")
			endif()
		endif()
	endforeach()
endmacro()

macro(spirv_cross_add_library name config_name)
	add_library(${name} ${ARGN})
	extract_headers(hdrs "${ARGN}")
	target_include_directories(${name} PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:include>)
	set_target_properties(${name} PROPERTIES
		PUBLIC_HEADERS "${hdrs}")
	target_compile_options(${name} PRIVATE ${spirv-compiler-options})
	target_compile_definitions(${name} PRIVATE ${spirv-compiler-defines})
	install(TARGETS ${name}
		EXPORT ${config_name}Config
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		PUBLIC_HEADER DESTINATION include/spirv_cross)
	install(FILES ${hdrs} DESTINATION include/spirv_cross)
	install(EXPORT ${config_name}Config DESTINATION share/${config_name}/cmake)
	export(TARGETS ${name} FILE ${config_name}Config.cmake)
endmacro()

set(spirv-cross-core-sources
	${CMAKE_CURRENT_SOURCE_DIR}/GLSL.std.450.h
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_common.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_parser.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_parser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross_parsed_ir.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross_parsed_ir.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_cfg.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_cfg.cpp)

set(spirv-cross-c-sources
	spirv.h
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross_c.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross_c.h)

set(spirv-cross-glsl-sources
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_glsl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_glsl.hpp)

set(spirv-cross-cpp-sources
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_cpp.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_cpp.hpp)

set(spirv-cross-msl-sources
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_msl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_msl.hpp)

set(spirv-cross-hlsl-sources
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_hlsl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_hlsl.hpp)

set(spirv-cross-reflect-sources
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect.hpp)

set(spirv-cross-util-sources
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross_util.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross_util.hpp)

if (SPIRV_CROSS_STATIC)
	spirv_cross_add_library(spirv-cross-core spirv_cross_core STATIC
		${spirv-cross-core-sources})

	spirv_cross_add_library(spirv-cross-c spirv_cross_c STATIC
		${spirv-cross-c-sources})

	spirv_cross_add_library(spirv-cross-glsl spirv_cross_glsl STATIC
		${spirv-cross-glsl-sources})

	spirv_cross_add_library(spirv-cross-cpp spirv_cross_cpp STATIC
		${spirv-cross-cpp-sources})

	spirv_cross_add_library(spirv-cross-reflect spirv_cross_reflect STATIC
		${spirv-cross-reflect-sources})

	spirv_cross_add_library(spirv-cross-msl spirv_cross_msl STATIC
		${spirv-cross-msl-sources})

	spirv_cross_add_library(spirv-cross-hlsl spirv_cross_hlsl STATIC
		${spirv-cross-hlsl-sources})

	spirv_cross_add_library(spirv-cross-util spirv_cross_util STATIC
		${spirv-cross-util-sources})

	target_link_libraries(spirv-cross-util PRIVATE spirv-cross-core)
	target_link_libraries(spirv-cross-glsl PRIVATE spirv-cross-core)
	target_link_libraries(spirv-cross-msl PRIVATE spirv-cross-glsl)
	target_link_libraries(spirv-cross-hlsl PRIVATE spirv-cross-glsl)
	target_link_libraries(spirv-cross-cpp PRIVATE spirv-cross-glsl)
endif()

if (SPIRV_CROSS_SHARED)
	set(spirv-cross-abi-major 0)
	set(spirv-cross-abi-minor 1)
	set(spirv-cross-abi-patch 0)
	set(SPIRV_CROSS_VERSION ${spirv-cross-abi-major}.${spirv-cross-abi-minor}.${spirv-cross-abi-patch})
	set(SPIRV_CROSS_INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib)
	set(SPIRV_CROSS_INSTALL_INC_DIR ${CMAKE_INSTALL_PREFIX}/include)
	configure_file(
		${CMAKE_CURRENT_SOURCE_DIR}/pkg-config/spirv-cross-c-shared.pc.in
		${CMAKE_CURRENT_BINARY_DIR}/spirv-cross-c-shared.pc @ONLY)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/spirv-cross-c-shared.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/share/pkgconfig)

	spirv_cross_add_library(spirv-cross-c-shared spirv_cross_c_shared SHARED
		${spirv-cross-core-sources}
		${spirv-cross-glsl-sources}
		${spirv-cross-cpp-sources}
		${spirv-cross-reflect-sources}
		${spirv-cross-msl-sources}
		${spirv-cross-hlsl-sources}
		${spirv-cross-c-sources})
	if (CMAKE_COMPILER_IS_GNUCXX OR (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang"))
		# Only export the C API.
		target_compile_options(spirv-cross-c-shared PRIVATE -fvisibility=hidden)
		if (NOT APPLE)
			set_target_properties(spirv-cross-c-shared PROPERTIES LINK_FLAGS "-Wl,--no-undefined")
		endif()
	endif()

	set_target_properties(spirv-cross-c-shared PROPERTIES
		VERSION ${SPIRV_CROSS_VERSION}
		SOVERSION ${spirv-cross-abi-major})
endif()

if (SPIRV_CROSS_CLI)
	if (NOT SPIRV_CROSS_STATIC)
		message(FATAL_ERROR "Must build static libraries if building CLI.")
	endif()
	add_executable(spirv-cross main.cpp)
	target_compile_options(spirv-cross PRIVATE ${spirv-compiler-options})
	target_compile_definitions(spirv-cross PRIVATE ${spirv-compiler-defines})

	install(TARGETS spirv-cross RUNTIME DESTINATION bin)
	target_link_libraries(spirv-cross PRIVATE
		spirv-cross-glsl
		spirv-cross-hlsl
		spirv-cross-cpp
		spirv-cross-reflect
		spirv-cross-msl
		spirv-cross-util
		spirv-cross-core)

	# Set up tests, using only the simplest modes of the test_shaders
	# script.  You have to invoke the script manually to:
	#  - Update the reference files
	#  - Get cycle counts from malisc
	#  - Keep failing outputs
	find_package(PythonInterp)
	if (${PYTHONINTERP_FOUND})
		if (${PYTHON_VERSION_MAJOR} GREATER 2)
			add_test(NAME spirv-cross-test
				COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_shaders.py --parallel
				${CMAKE_CURRENT_SOURCE_DIR}/shaders
				WORKING_DIRECTORY $<TARGET_FILE_DIR:spirv-cross>)
			add_test(NAME spirv-cross-test-no-opt
				COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_shaders.py --parallel
				${CMAKE_CURRENT_SOURCE_DIR}/shaders-no-opt
				WORKING_DIRECTORY $<TARGET_FILE_DIR:spirv-cross>)
			add_test(NAME spirv-cross-test-metal
				COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_shaders.py --metal --parallel
				${CMAKE_CURRENT_SOURCE_DIR}/shaders-msl
				WORKING_DIRECTORY $<TARGET_FILE_DIR:spirv-cross>)
			add_test(NAME spirv-cross-test-metal-no-opt
				COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_shaders.py --metal --parallel
				${CMAKE_CURRENT_SOURCE_DIR}/shaders-msl-no-opt
				WORKING_DIRECTORY $<TARGET_FILE_DIR:spirv-cross>)
			add_test(NAME spirv-cross-test-hlsl
				COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_shaders.py --hlsl --parallel
				${CMAKE_CURRENT_SOURCE_DIR}/shaders-hlsl
				WORKING_DIRECTORY $<TARGET_FILE_DIR:spirv-cross>)
			add_test(NAME spirv-cross-test-hlsl-no-opt
				COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_shaders.py --hlsl --parallel
				${CMAKE_CURRENT_SOURCE_DIR}/shaders-hlsl-no-opt
				WORKING_DIRECTORY $<TARGET_FILE_DIR:spirv-cross>)
			add_test(NAME spirv-cross-test-opt
				COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_shaders.py --opt --parallel
				${CMAKE_CURRENT_SOURCE_DIR}/shaders
				WORKING_DIRECTORY $<TARGET_FILE_DIR:spirv-cross>)
			add_test(NAME spirv-cross-test-metal-opt
				COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_shaders.py --metal --opt --parallel
				${CMAKE_CURRENT_SOURCE_DIR}/shaders-msl
				WORKING_DIRECTORY $<TARGET_FILE_DIR:spirv-cross>)
			add_test(NAME spirv-cross-test-hlsl-opt
				COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_shaders.py --hlsl --opt --parallel
				${CMAKE_CURRENT_SOURCE_DIR}/shaders-hlsl
				WORKING_DIRECTORY $<TARGET_FILE_DIR:spirv-cross>)
			add_test(NAME spirv-cross-test-reflection
				COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_shaders.py --reflect --parallel
				${CMAKE_CURRENT_SOURCE_DIR}/shaders-reflection
				WORKING_DIRECTORY $<TARGET_FILE_DIR:spirv-cross>)
		endif()
	else()
		message(WARNING "Testing disabled. Could not find python3. If you have python3 installed try running "
			"cmake with -DPYTHON_EXECUTABLE:FILEPATH=/path/to/python3 to help it find the executable")
	endif()
endif()
