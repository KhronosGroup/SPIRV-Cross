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

// Test the captured-record ABI, including failures which must not emit plausible but incorrect MSL.
#include "spirv_msl.hpp"
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
using namespace spirv_cross;

static void check(bool condition, const char *message)
{
	if (!condition)
		throw std::runtime_error(message);
}

static std::vector<uint32_t> read_spirv(const char *path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	check(bool(file), "Cannot open SPIR-V fixture.");
	auto size = file.tellg();
	check(size > 0 && size % 4 == 0, "Invalid SPIR-V fixture size.");
	std::vector<uint32_t> words(size_t(size) / 4);
	file.seekg(0);
	file.read(reinterpret_cast<char *>(words.data()), size);
	check(bool(file), "Cannot read SPIR-V fixture.");
	return words;
}

static MSLCapturedVertexLayout layout()
{
	MSLCapturedVertexLayout result;
	result.stride = 96;
	auto add = [&](uint32_t location, uint32_t component, uint32_t offset, SPIRType::BaseType type) {
		MSLCapturedVertexComponent field;
		field.location = location;
		field.component = component;
		field.byte_offset = offset;
		field.scalar_type = type;
		result.components.push_back(field);
	};
	add(0, 0, 16, SPIRType::Float);
	add(0, 1, 24, SPIRType::Float);
	add(0, 2, 20, SPIRType::Float);
	add(1, 2, 0, SPIRType::Float);
	add(1, 3, 4, SPIRType::Float);
	for (uint32_t c = 0; c < 4; c++) add(2, c, 32 + 4 * c, SPIRType::Int);
	for (uint32_t c = 0; c < 3; c++) add(3, c, 48 + 4 * c, SPIRType::UInt);
	for (uint32_t c = 0; c < 2; c++) add(4, c, 8 + 2 * c, SPIRType::Half);
	for (uint32_t c = 0; c < 3; c++) add(5, c, 64 + 2 * c, SPIRType::Short);
	for (uint32_t c = 0; c < 4; c++) add(6, c, 72 + 2 * c, SPIRType::UShort);
	add(7, 3, 80, SPIRType::Float);
	return result;
}

static MSLPerVertexInputBinding binding()
{
	MSLPerVertexInputBinding result;
	result.vertex_buffer_index = 28;
	result.primitive_index_buffer_index = 29;
	result.primitive_index_location = 15;
	return result;
}

static void configure(CompilerMSL &compiler, bool argument_buffers = false, bool native_arrays = false)
{
	auto options = compiler.get_msl_options();
	options.set_msl_version(2, 4);
	options.argument_buffers = argument_buffers;
	options.force_native_arrays = native_arrays;
	compiler.set_msl_options(options);
	MSLResourceBinding resource;
	resource.stage = spv::ExecutionModelFragment;
	resource.desc_set = 0;
	resource.binding = 0;
	resource.msl_buffer = 5;
	compiler.add_msl_resource_binding(resource);
}

static void rejects(const std::vector<uint32_t> &words, const char *diagnostic, const std::function<void(MSLCapturedVertexLayout &, MSLPerVertexInputBinding &, CompilerMSL &)> &change)
{
	CompilerMSL compiler(words);
	configure(compiler);
	auto physical = layout();
	auto buffers = binding();
	change(physical, buffers, compiler);
	compiler.set_msl_per_vertex_input_buffer(physical, buffers);
	try
	{
		compiler.compile();
	}
	catch (const CompilerError &error)
	{
		check(std::string(error.what()).find(diagnostic) != std::string::npos, error.what());
		return;
	}
	throw std::runtime_error(std::string("Expected rejection: ") + diagnostic);
}

static void write_msl(const std::string &directory, const std::string &name, const std::string &msl)
{
	if (directory.empty())
		return;
	std::ofstream file(directory + "/" + name + ".metal");
	file << msl;
	check(bool(file), "Cannot write generated MSL.");
}

static void test_composites(const std::vector<uint32_t> &words, const std::string &directory)
{
	MSLCapturedVertexLayout physical;
	physical.stride = 416;
	for (uint32_t location = 0; location <= 24; location++)
		for (uint32_t component = 0; component < 4; component++)
		{
			MSLCapturedVertexComponent field;
			field.location = location;
			field.component = component;
			field.byte_offset = 16 * location + 4 * (3 - component);
			field.scalar_type = location == 10 ? SPIRType::Int : location == 11 ? SPIRType::UInt : SPIRType::Float;
			physical.components.push_back(field);
		}
	auto buffers = binding();
	buffers.primitive_index_location = 31;
	for (bool arguments : {false, true})
		for (bool arrays : {false, true})
		{
			CompilerMSL compiler(words);
			configure(compiler, arguments, arrays);
			compiler.set_msl_per_vertex_input_buffer(physical, buffers);
			auto msl = compiler.compile();
			check(compiler.needs_per_vertex_input_buffer(), "Composite input has no portable buffers.");
			check(msl.find("vertex_value") == std::string::npos, "Composite portable input uses native transport.");
			// Verify actual leaf-to-producer mapping, not just the presence of loads.
			auto leaf = [&](const std::string &path, uint32_t vertex, uint32_t location, uint32_t component) {
				auto start = msl.find(path + " = ");
				check(start != std::string::npos, ("Missing composite assignment: " + path).c_str());
				auto assignment = msl.substr(start, msl.find(';', start) - start);
				check(assignment.find("* 3ul + " + std::to_string(vertex) + "ul]") != std::string::npos, "Wrong composite vertex index.");
				check(assignment.find("* 416ul + " + std::to_string(16 * location + 4 * (3 - component)) + "ul)") != std::string::npos, "Wrong composite Location/Component offset.");
				check(compiler.is_msl_shader_input_used(location), "Composite leaf location missing from reflection.");
			};
			leaf("texcoord[2][1]", 2, 1, 0);
			leaf("transforms[1][1]", 1, 3, 0);
			leaf("inputs[2].color", 2, 4, 0);
			leaf("inputs[2].basis[1]", 2, 6, 0);
			leaf("inputs[2].nested.weights[1]", 2, 9, 0);
			leaf("high[2]", 2, 10, 2);
			leaf("first[0]", 0, 11, 0);
			leaf("pair[1]", 1, 12, 0);
			leaf("matrixArray[2][1][1]", 2, 16, 0);
			leaf("nestedArray[2][1][1]", 2, 20, 0);
			leaf("located[2].a", 2, 21, 0);
			leaf("located[2].b", 2, 24, 0);
			leaf("packedValues[2].value[0]", 2, 22, 2);
			leaf("packedValues[2].value[1]", 2, 23, 2);
			check(compiler.is_msl_shader_input_used(25), "Ordinary input missing from reflection.");
			check(msl.find("[[user(locn25)") != std::string::npos, "Ordinary interpolation lost.");
			check(msl.find("\n    first[1] =") == std::string::npos && msl.find("\n    pair[2] =") == std::string::npos, "Loaded beyond declared vertex count.");
			write_msl(directory, std::string("composites") + (arguments ? "-arguments" : "-classic") + (arrays ? "-native-arrays" : ""), msl);
		}
	// Every recursively visited leaf must validate mappings and remaps, including later columns.
	for (bool remap : {false, true})
	{
		CompilerMSL compiler(words);
		configure(compiler);
		uint32_t location = 16;
		auto incomplete = physical;
		if (remap)
		{
			MSLShaderInterfaceVariable input;
			input.location = location;
			compiler.add_msl_shader_input(input);
		}
		else
			incomplete.components.erase(incomplete.components.begin() + 4 * location);
		compiler.set_msl_per_vertex_input_buffer(incomplete, buffers);
		try
		{
			compiler.compile();
		}
		catch (const CompilerError &error)
		{
			check(std::string(error.what()).find(remap ? "remapping" : "missing producer scalar") != std::string::npos, error.what());
			continue;
		}
		throw std::runtime_error("Missing composite validation.");
	}
}

int main(int argc, char **argv)
{
	try
	{
		check(argc >= 2, "Usage: test fixture.spv [output-directory] [existing-reproducer.spv]");
		auto words = read_spirv(argv[1]);
		if (argc >= 3 && std::string(argv[2]) == "--composites")
		{
			test_composites(words, argc > 3 ? argv[3] : "");
			std::cout << "Portable composite PerVertexKHR checks passed.\n";
			return 0;
		}
		if (argc == 4 && std::string(argv[2]) == "--reject")
		{
			rejects(words, argv[3], [](MSLCapturedVertexLayout &, MSLPerVertexInputBinding &, CompilerMSL &) {});
			std::cout << "Rejected unsupported input: " << argv[3] << '\n';
			return 0;
		}
		std::string directory = argc > 2 ? argv[2] : "";
		for (bool arguments : {false, true})
			for (bool arrays : {false, true})
			{
				CompilerMSL compiler(words);
				configure(compiler, arguments, arrays);
				check(!compiler.needs_per_vertex_input_buffer(), "Portable resources reported before compilation.");
				compiler.set_msl_per_vertex_input_buffer(layout(), binding());
				auto msl = compiler.compile();
				check(compiler.needs_per_vertex_input_buffer(), "Missing portable resource reflection.");
				check(compiler.is_msl_shader_input_used(0) && compiler.is_msl_shader_input_used(7) && compiler.is_msl_shader_input_used(8), "Missing shader input reflection.");
				check(!compiler.is_msl_shader_input_used(15), "Private varying leaked into shader input reflection.");
				check(msl.find("vertex_value") == std::string::npos && msl.find("[[primitive_id]]") == std::string::npos, "Portable path depends on native inputs.");
				check(msl.find("uint spvPerVertexPrimitive [[user(locn15)") != std::string::npos, "Missing private integral varying.");
				check(msl.find("* 96ul + 24ul") != std::string::npos, "Physical component offsets or stride lost.");
				check(msl.find("* 3ul + 2ul") != std::string::npos, "Triplet indexing lost.");
				for (auto type : {"float", "int", "uint", "half", "short", "ushort"})
					check(msl.find(std::string("reinterpret_cast<const device ") + type + "*>") != std::string::npos, "Typed load lost.");
				check((msl.find("color[index]") != std::string::npos || msl.find("color[_") != std::string::npos), "Dynamic indexing lost.");
				write_msl(directory, std::string(arguments ? "arguments" : "classic") + (arrays ? "-native-arrays" : ""), msl);
			}
		rejects(words, "MSL 2.4", [](MSLCapturedVertexLayout &, MSLPerVertexInputBinding &, CompilerMSL &c) { auto o = c.get_msl_options(); o.set_msl_version(2, 3); c.set_msl_options(o); });
		rejects(words, "mutually exclusive", [](MSLCapturedVertexLayout &, MSLPerVertexInputBinding &, CompilerMSL &c) { auto o = c.get_msl_options(); o.supports_per_vertex_fragment_input = true; c.set_msl_options(o); });
		rejects(words, "capture_output_to_buffer", [](MSLCapturedVertexLayout &, MSLPerVertexInputBinding &, CompilerMSL &c) { auto o = c.get_msl_options(); o.capture_output_to_buffer = true; c.set_msl_options(o); });
		rejects(words, "nonzero producer stride", [](MSLCapturedVertexLayout &l, MSLPerVertexInputBinding &, CompilerMSL &) { l.stride = 0; });
		rejects(words, "nonzero producer stride", [](MSLCapturedVertexLayout &l, MSLPerVertexInputBinding &, CompilerMSL &) { l.components.clear(); });
		rejects(words, "distinct Metal buffer", [](MSLCapturedVertexLayout &, MSLPerVertexInputBinding &b, CompilerMSL &) { b.vertex_buffer_index = b.primitive_index_buffer_index; });
		rejects(words, "distinct Metal buffer", [](MSLCapturedVertexLayout &, MSLPerVertexInputBinding &b, CompilerMSL &) { b.vertex_buffer_index = 31; });
		rejects(words, "private primitive index Location", [](MSLCapturedVertexLayout &, MSLPerVertexInputBinding &b, CompilerMSL &) { b.primitive_index_location = ~0u; });
		rejects(words, "producer layout", [](MSLCapturedVertexLayout &, MSLPerVertexInputBinding &b, CompilerMSL &) { b.primitive_index_location = 0; });
		rejects(words, "fragment input", [](MSLCapturedVertexLayout &, MSLPerVertexInputBinding &b, CompilerMSL &) { b.primitive_index_location = 8; });
		rejects(words, "missing producer scalar", [](MSLCapturedVertexLayout &l, MSLPerVertexInputBinding &, CompilerMSL &) { l.components.erase(l.components.begin()); });
		rejects(words, "type mismatch", [](MSLCapturedVertexLayout &l, MSLPerVertexInputBinding &, CompilerMSL &) { l.components[0].scalar_type = SPIRType::Int; });
		rejects(words, "16/32-bit", [](MSLCapturedVertexLayout &l, MSLPerVertexInputBinding &, CompilerMSL &) { l.components[0].scalar_type = SPIRType::Double; });
		rejects(words, "duplicate Location/Component", [](MSLCapturedVertexLayout &l, MSLPerVertexInputBinding &, CompilerMSL &) { l.components.push_back(l.components.front()); });
		rejects(words, "invalid Location/Component", [](MSLCapturedVertexLayout &l, MSLPerVertexInputBinding &, CompilerMSL &) { l.components[0].component = 4; });
		rejects(words, "naturally aligned", [](MSLCapturedVertexLayout &l, MSLPerVertexInputBinding &, CompilerMSL &) { l.components[0].byte_offset = 17; });
		rejects(words, "naturally aligned", [](MSLCapturedVertexLayout &l, MSLPerVertexInputBinding &, CompilerMSL &) { l.stride = 95; });
		rejects(words, "outside the producer stride", [](MSLCapturedVertexLayout &l, MSLPerVertexInputBinding &, CompilerMSL &) { l.components[0].byte_offset = ~3u; });
		rejects(words, "overlapping scalar offsets", [](MSLCapturedVertexLayout &l, MSLPerVertexInputBinding &, CompilerMSL &) { l.components[0].byte_offset = 24; });
		rejects(words, "buffer index 5 collides", [](MSLCapturedVertexLayout &, MSLPerVertexInputBinding &b, CompilerMSL &) { b.vertex_buffer_index = 5; });
		rejects(words, "buffer index 6 collides", [](MSLCapturedVertexLayout &, MSLPerVertexInputBinding &b, CompilerMSL &) { b.primitive_index_buffer_index = 6; });
		rejects(words, "buffer index 0 collides", [](MSLCapturedVertexLayout &, MSLPerVertexInputBinding &b, CompilerMSL &c) { b.vertex_buffer_index = 0; auto o = c.get_msl_options(); o.argument_buffers = true; c.set_msl_options(o); });
		rejects(words, "requires a Location", [](MSLCapturedVertexLayout &, MSLPerVertexInputBinding &, CompilerMSL &c) {
			for (auto &input : c.get_shader_resources().stage_inputs)
				if (c.has_decoration(input.id, spv::DecorationPerVertexKHR) && c.get_decoration(input.id, spv::DecorationLocation) == 0)
					c.unset_decoration(input.id, spv::DecorationLocation);
		});
		rejects(words, "remapping", [](MSLCapturedVertexLayout &, MSLPerVertexInputBinding &, CompilerMSL &c) { MSLShaderInterfaceVariable i; i.location = 0; i.vecsize = 4; c.add_msl_shader_input(i); });
		// An inactive interface has no buffers, no private varying, and identical generated code.
		CompilerMSL inactive(words), control(words);
		configure(inactive);
		configure(control);
		// Exercise true inactivity by removing the decorations; ordinary interpolation remains valid.
		for (auto &r : inactive.get_shader_resources().stage_inputs)
		{
			inactive.unset_decoration(r.id, spv::DecorationPerVertexKHR);
			control.unset_decoration(r.id, spv::DecorationPerVertexKHR);
		}
		inactive.set_msl_per_vertex_input_buffer(layout(), binding());
		check(inactive.compile() == control.compile(), "Inactive option changes ordinary shaders.");
		check(!inactive.needs_per_vertex_input_buffer(), "Inactive option requires buffers.");
		if (argc > 3)
		{
			CompilerMSL compiler(read_spirv(argv[3]));
			configure(compiler);
			auto physical = layout();
			physical.stride = 32;
			physical.components.resize(3);
			for (uint32_t i = 0; i < 3; i++) physical.components[i].byte_offset = 4 * i;
			compiler.set_msl_per_vertex_input_buffer(physical, binding());
			write_msl(directory, "existing-reproducer", compiler.compile());
		}
		std::cout << "Portable PerVertexKHR API checks passed.\n";
		return 0;
	}
	catch (const std::exception &error)
	{
		std::cerr << error.what() << '\n';
		return 1;
	}
}
