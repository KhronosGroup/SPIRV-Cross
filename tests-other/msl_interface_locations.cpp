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

// Explicit linkage must survive both retained capture composites and flattened stage I/O.
#include "spirv_msl.hpp"
#include <fstream>
#include <iostream>
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
	check(bool(file), "Cannot open fixture.");
	auto size = file.tellg();
	check(size > 0 && size % 4 == 0, "Invalid fixture size.");
	std::vector<uint32_t> words(size_t(size) / 4);
	file.seekg(0);
	file.read(reinterpret_cast<char *>(words.data()), size);
	check(bool(file), "Cannot read fixture.");
	return words;
}

class InterfaceCompiler : public CompilerMSL
{
public:
	using CompilerMSL::CompilerMSL;
	const SPIRType &interface_type(bool input) const
	{
		return get_variable_data_type(get<SPIRVariable>(input ? stage_in_var_id : stage_out_var_id));
	}
};

struct Member
{
	const char *name;
	uint32_t location;
	uint32_t component;
};

static void check_member(InterfaceCompiler &compiler, const SPIRType &type, const Member &member, const std::string &msl, bool capture)
{
	std::string name = std::string("located_") + member.name;
	for (uint32_t i = 0; i < type.member_types.size(); i++)
	{
		if (compiler.get_member_name(type.self, i) != name)
			continue;
		check(compiler.has_member_decoration(type.self, i, spv::DecorationLocation), "Missing Location for " + name);
		auto location = compiler.get_member_decoration(type.self, i, spv::DecorationLocation);
		check(location == member.location, name + " Location " + std::to_string(location) + " != " + std::to_string(member.location));
		check(compiler.get_member_decoration(type.self, i, spv::DecorationComponent) == member.component, "Wrong Component for " + name);
		if (!capture)
		{
			std::string suffix = member.component ? "_" + std::to_string(member.component) : "";
			check(msl.find(name + " [[user(locn" + std::to_string(member.location) + suffix + ")]]") != std::string::npos, "Wrong MSL linkage for " + name);
		}
		return;
	}
	throw std::runtime_error("Missing member " + name);
}

static void test_interface(const std::vector<uint32_t> &words, bool input, bool capture, bool compute, bool native_arrays, const std::string &directory, bool widen_input = false)
{
	InterfaceCompiler compiler(words);
	auto options = compiler.get_msl_options();
	options.set_msl_version(2, 4);
	options.capture_output_to_buffer = capture;
	options.vertex_for_tessellation = compute;
	options.force_native_arrays = native_arrays;
	compiler.set_msl_options(options);
	if (widen_input)
	{
		MSLShaderInterfaceVariable binding;
		binding.location = 5;
		binding.vecsize = 2;
		compiler.add_msl_shader_input(binding);
	}
	auto msl = compiler.compile();
	const auto &type = compiler.interface_type(input);
	if (capture)
	{
		for (const auto &member : {Member{"nested", 4, 0}, Member{"basis", 10, 0}, Member{"tail", 15, 0}, Member{"upper", 18, 2}, Member{"lower", 18, 0}, Member{"early", 2, 1}, Member{"structured", 23, 0}})
			check_member(compiler, type, member, msl, true);
		auto layout = compiler.get_msl_captured_vertex_layout();
		check(layout.stride == 192 && layout.components.size() == 29, "Incorrect composite capture ABI.");
	}
	else
	{
		for (const auto &member : {Member{"nested_color", 4, 0}, Member{"nested_weights_0", 5, 0}, Member{"nested_weights_1", 6, 0}, Member{"basis_0", 10, 0}, Member{"basis_1", 11, 0}, Member{"tail", 15, 0}, Member{"upper_0", 18, 2}, Member{"upper_1", 19, 2}, Member{"lower_0", 18, 0}, Member{"lower_1", 19, 0}, Member{"early", 2, 1}, Member{"structured_0_color", 23, 0}, Member{"structured_0_weights_0", 24, 0}, Member{"structured_0_weights_1", 25, 0}, Member{"structured_1_color", 26, 0}, Member{"structured_1_weights_0", 27, 0}, Member{"structured_1_weights_1", 28, 0}})
			check_member(compiler, type, member, msl, false);
	}
	if (widen_input)
	{
		check(msl.find("float2 located_nested_weights_0 [[user(locn5)]]") != std::string::npos, "Nested input did not use its inherited Location for interface matching.");
		check(msl.find("located.nested.weights[0] = in.located_nested_weights_0.x;") != std::string::npos, "Nested input widening lost its load swizzle.");
	}
	if (!directory.empty())
	{
		std::string name = input ? "fragment" : compute ? "compute-capture" : capture ? "vertex-capture" : "raster";
		std::ofstream file(directory + "/" + name + (native_arrays ? "-native" : "") + (widen_input ? "-wide" : "") + ".metal");
		file << msl;
		check(bool(file), "Cannot write generated MSL.");
	}
}

static void test_tessellation_rejection(const std::vector<uint32_t> &words, bool native_arrays)
{
	CompilerMSL compiler(words);
	auto options = compiler.get_msl_options();
	options.set_msl_version(2, 4);
	options.force_native_arrays = native_arrays;
	compiler.set_msl_options(options);
	try
	{
		compiler.compile();
	}
	catch (const CompilerError &error)
	{
		check(std::string(error.what()).find("Component decoration is not supported in tessellation shaders.") != std::string::npos, error.what());
		return;
	}
	throw std::runtime_error("Component-decorated tessellation arrays must fail before emitting MSL.");
}

int main(int argc, char **argv)
{
	try
	{
		check(argc >= 4, "Usage: test vertex.spv fragment.spv tessellation.spv [output-directory]");
		auto vertex = read_spirv(argv[1]);
		auto fragment = read_spirv(argv[2]);
		auto tessellation = read_spirv(argv[3]);
		std::string directory = argc > 4 ? argv[4] : "";
		for (bool native_arrays : {false, true})
		{
			test_tessellation_rejection(tessellation, native_arrays);
			test_interface(vertex, false, true, false, native_arrays, directory);
			test_interface(vertex, false, true, true, native_arrays, directory);
			test_interface(vertex, false, false, false, native_arrays, directory);
			test_interface(fragment, true, false, false, native_arrays, directory);
			test_interface(fragment, true, false, false, native_arrays, directory, true);
		}
		std::cout << "Explicit capture/raster interface location checks passed.\n";
		return 0;
	}
	catch (const std::exception &error)
	{
		std::cerr << error.what() << '\n';
		return 1;
	}
}
