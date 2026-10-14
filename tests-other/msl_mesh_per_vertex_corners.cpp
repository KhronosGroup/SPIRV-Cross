/*
 * Copyright 2026 Jean-Philippe Meunier
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Test mesh_per_vertex_corner_locations: the mesh copies and the fragment reads the same corner varyings, and the
// unsupported forms throw instead of emitting plausible but incorrect MSL.
// Usage: spirv-cross-msl-mesh-corners-test <mesh> <mesh MATRIX> <mesh LINES> <frag> <frag BLOCK> <frag MATRIX>
#include "spirv_msl.hpp"
#include <fstream>
#include <iostream>
#include <regex>
#include <set>
#include <stdexcept>
using namespace spirv_cross;

static void check(bool condition, const std::string &message)
{
	if (!condition)
		throw std::runtime_error(message);
}

static std::vector<uint32_t> read_spirv(const char *path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	check(bool(file), std::string("Cannot open ") + path);
	auto size = file.tellg();
	std::vector<uint32_t> words(size_t(size) / 4);
	file.seekg(0);
	file.read(reinterpret_cast<char *>(words.data()), size);
	return words;
}

// Mesh: Locations 0 and 3 (0b1001).
static std::string compile(const char *path, uint32_t mask = 9, bool native = false, bool buffer = false)
{
	CompilerMSL compiler(read_spirv(path));
	auto options = compiler.get_msl_options();
	options.set_msl_version(native ? 4 : 3, 0);
	options.supports_per_vertex_fragment_input = native;
	options.mesh_per_vertex_corner_locations = mask;
	compiler.set_msl_options(options);
	if (buffer)
		compiler.set_msl_per_vertex_input_buffer({}, {});
	return compiler.compile();
}

static void rejects(const char *path, uint32_t mask, const char *diagnostic, bool buffer = false)
{
	try
	{
		compile(path, mask, false, buffer);
	}
	catch (const CompilerError &error)
	{
		check(std::string(error.what()).find(diagnostic) != std::string::npos, std::string("Wrong diagnostic: ") + error.what());
		return;
	}
	throw std::runtime_error(std::string("Expected a rejection: ") + diagnostic);
}

static std::set<std::string> corners(const std::string &msl)
{
	static const std::regex name("user\\((locn[0-9]+(_[0-9]+)?_corner[0-9])\\)(, flat)?");
	std::set<std::string> result;
	for (std::sregex_iterator it(msl.begin(), msl.end(), name), end; it != end; ++it)
		result.insert((*it)[1].str() + ((*it)[3].matched ? " flat" : ""));
	return result;
}

int main(int argc, char **argv)
{
	try
	{
		check(argc == 7, "Usage: spirv-cross-msl-mesh-corners-test <mesh> <mesh MATRIX> <mesh LINES> <frag> <frag BLOCK> <frag MATRIX>");
		const std::set<std::string> expected_mesh = { "locn0_corner0", "locn0_corner1", "locn0_corner2", "locn3_1_corner0", "locn3_1_corner1", "locn3_1_corner2" };
		std::string mesh = compile(argv[1]);
		check(corners(mesh) == expected_mesh, "The mesh must copy Locations 0 and 3 only:\n" + mesh);
		check(mesh.find("[gl_PrimitiveTriangleIndicesEXT[spvPI].z]") != std::string::npos, "The third corner must follow the third index:\n" + mesh);
		check(corners(compile(argv[1], 0)).empty(), "No corners without the option.");

		// The fragment reads the corners by the same names, flat, even where vertex_value is available.
		for (bool native : { false, true })
		{
			std::string fragment = compile(argv[4], 9, native);
			std::set<std::string> read;
			for (auto &corner : corners(fragment))
			{
				check(corner.size() > 5 && corner.compare(corner.size() - 5, 5, " flat") == 0, "Corner inputs must be flat:\n" + fragment);
				read.insert(corner.substr(0, corner.size() - 5));
			}
			check(read == expected_mesh, "The fragment must read the corners the mesh writes:\n" + fragment);
			check(fragment.find("vertex_value") == std::string::npos, "Mesh corners must take precedence over vertex_value:\n" + fragment);
			check(fragment.find("[[user(locn2)]]") != std::string::npos, "Location 2 must stay interpolated:\n" + fragment);
		}

		rejects(argv[2], 9, "require scalar or vector per-vertex outputs");
		rejects(argv[3], 9, "require triangles and PrimitiveTriangleIndicesEXT");
		rejects(argv[1], 9 | 2, "names a Location without a per-vertex mesh output");
		rejects(argv[4], 1, "missing from mesh_per_vertex_corner_locations");
		rejects(argv[4], 9, "mutually exclusive", true);
		rejects(argv[5], 9, "must be arrays of scalars or vectors");
		rejects(argv[6], 11, "must be arrays of scalars or vectors");
		std::cout << "Mesh PerVertexKHR corner checks passed." << std::endl;
		return 0;
	}
	catch (const std::exception &error)
	{
		std::cerr << "FAIL: " << error.what() << std::endl;
		return 1;
	}
}
