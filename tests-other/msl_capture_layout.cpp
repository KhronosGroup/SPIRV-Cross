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

// Physical layout reflection is checked against both expected ABI values and the emitted MSL type.
#include "spirv_msl.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace spirv_cross;

static void check(bool condition, const char *message)
{
	if (!condition)
		throw std::runtime_error(message);
}

static std::vector<uint32_t> read_spirv(const char *filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	check(bool(file), "Cannot open fixture.");
	auto size = file.tellg();
	check(size > 0 && size % 4 == 0, "Invalid fixture size.");
	std::vector<uint32_t> words(size_t(size) / 4);
	file.seekg(0);
	file.read(reinterpret_cast<char *>(words.data()), size);
	check(bool(file), "Cannot read fixture.");
	return words;
}

static void configure(CompilerMSL &compiler, bool native_arrays = false, bool compute = false)
{
	auto options = compiler.get_msl_options();
	options.set_msl_version(2, 4);
	options.capture_output_to_buffer = true;
	options.force_native_arrays = native_arrays;
	options.vertex_for_tessellation = compute;
	compiler.set_msl_options(options);
}

static void rejects(CompilerMSL &compiler, const char *diagnostic)
{
	try
	{
		compiler.get_msl_captured_vertex_layout();
	}
	catch (const CompilerError &error)
	{
		check(std::string(error.what()).find(diagnostic) != std::string::npos, error.what());
		return;
	}
	throw std::runtime_error(std::string("Expected layout error: ") + diagnostic);
}

static void write_msl(const std::string &directory, const std::string &name, const std::string &source)
{
	if (directory.empty())
		return;
	std::ofstream file(directory + "/" + name + ".metal");
	file << source;
	check(bool(file), "Cannot write MSL assertions.");
}

static std::string stride_assertion(const MSLCapturedVertexLayout &layout)
{
	return "\nstatic_assert(sizeof(main0_out) == " + std::to_string(layout.stride) + ", \"capture stride\");\n";
}

static std::string field_assertion(const MSLCapturedVertexLayout &layout, uint32_t location, uint32_t component, uint32_t expected_offset, SPIRType::BaseType expected_type, const char *member, uint32_t lane, uint32_t scalar_size)
{
	for (const auto &field : layout.components)
		if (field.location == location && field.component == component)
		{
			if (field.byte_offset != expected_offset)
				throw std::runtime_error("Wrong physical offset at Location " + std::to_string(location) + " Component " + std::to_string(component) + ": " + std::to_string(field.byte_offset) + " expected " + std::to_string(expected_offset));
			check(field.scalar_type == expected_type, "Wrong physical scalar type.");
			return std::string("static_assert(__builtin_offsetof(main0_out, ") + member + ") + " + std::to_string(lane * scalar_size) + " == " + std::to_string(field.byte_offset) + ", \"capture field offset\");\n";
		}
	throw std::runtime_error("Missing captured field.");
}

static std::string builtin_assertion(const MSLCapturedVertexLayout &layout, spv::BuiltIn builtin, uint32_t array_index, uint32_t component, uint32_t expected_offset, SPIRType::BaseType expected_type, const char *member, uint32_t scalar_offset)
{
	for (const auto &field : layout.builtins)
		if (field.builtin == builtin && field.array_index == array_index && field.component == component)
		{
			check(field.byte_offset == expected_offset, "Wrong builtin offset.");
			check(field.scalar_type == expected_type, "Wrong builtin physical type.");
			return std::string("static_assert(__builtin_offsetof(main0_out, ") + member + ") + " + std::to_string(scalar_offset) + " == " + std::to_string(field.byte_offset) + ", \"capture builtin offset\");\n";
		}
	throw std::runtime_error("Missing captured builtin scalar.");
}

int main(int argc, char **argv)
{
	try
	{
		check(argc >= 5, "Usage: test simple.spv complex.spv unsupported.spv consumer.spv [output-directory]");
		auto simple = read_spirv(argv[1]);
		auto complex = read_spirv(argv[2]);
		auto unsupported = read_spirv(argv[3]);
		auto consumer_words = read_spirv(argv[4]);
		std::string directory = argc > 5 ? argv[5] : "";
		for (bool compute : {false, true})
		{
			CompilerMSL compiler(simple);
			configure(compiler, false, compute);
			rejects(compiler, "successful MSL compilation");
			auto msl = compiler.compile();
			auto layout = compiler.get_msl_captured_vertex_layout();
			check(layout.stride == 32 && layout.components.size() == 3 && layout.builtins.size() == 4, "Position + float3 must occupy a 32-byte record.");
			msl += stride_assertion(layout);
			for (uint32_t c = 0; c < 3; c++)
				msl += field_assertion(layout, 0, c, 4 * c, SPIRType::Float, "color", c, 4);
			for (uint32_t c = 0; c < 4; c++)
				msl += builtin_assertion(layout, spv::BuiltInPosition, 0, c, 16 + 4 * c, SPIRType::Float, "gl_Position", 4 * c);
			msl += "static_assert(__is_same(decltype(main0_out::color), float3), \"physical color type\");\n";
			write_msl(directory, compute ? "simple-compute" : "simple", msl);
			CompilerMSL consumer(consumer_words);
			auto options = consumer.get_msl_options();
			options.set_msl_version(2, 4);
			consumer.set_msl_options(options);
			MSLPerVertexInputBinding binding;
			binding.vertex_buffer_index = 28;
			binding.primitive_index_buffer_index = 29;
			binding.primitive_index_location = 15;
			consumer.set_msl_per_vertex_input_buffer(layout, binding);
			auto fragment = consumer.compile();
			check(consumer.needs_per_vertex_input_buffer(), "Exported producer layout was not used by consumer.");
			write_msl(directory, compute ? "consumer-compute-layout" : "consumer", fragment);
		}
		for (bool native_arrays : {false, true})
			for (uint32_t mode = 0; mode < 3; mode++)
			{
				bool remap_builtin = mode == 1;
				bool compute = mode == 2;
				CompilerMSL compiler(complex);
				configure(compiler, native_arrays, compute);
				if (remap_builtin)
				{
					MSLShaderInterfaceVariable output;
					output.builtin = spv::BuiltInPosition;
					output.location = 1;
					compiler.add_msl_shader_output(output);
				}
				auto msl = compiler.compile();
				auto layout = compiler.get_msl_captured_vertex_layout();
				check(layout.stride == 112 && layout.components.size() == 13 && layout.builtins.size() == 9, "Wrong mixed capture stride or padding lanes exported.");
				msl += stride_assertion(layout);
				for (uint32_t c = 0; c < 2; c++)
					msl += field_assertion(layout, 0, c + 2, 8 + 4 * c, SPIRType::Float, "m_location_0", c + 2, 4);
				for (uint32_t c = 0; c < 3; c++)
				{
					msl += field_assertion(layout, 2, c, 16 + 2 * c, SPIRType::Half, "small", c, 2);
					msl += field_assertion(layout, 5, c, 32 + 4 * c, SPIRType::Float, "blockData_normal", c, 4);
					msl += field_assertion(layout, 7, c, 48 + 4 * c, SPIRType::Float, "later", c, 4);
				}
				msl += field_assertion(layout, 3, 0, 24, SPIRType::UShort, "label", 0, 2);
				msl += field_assertion(layout, 4, 0, 28, SPIRType::Float, "blockData_weight", 0, 4);
				for (uint32_t c = 0; c < 4; c++)
					msl += builtin_assertion(layout, spv::BuiltInPosition, 0, c, 64 + 4 * c, SPIRType::Float, "gl_Position", 4 * c);
				msl += builtin_assertion(layout, spv::BuiltInPointSize, 0, 0, 80, SPIRType::Float, "gl_PointSize", 0);
				for (uint32_t a = 0; a < 2; a++)
					msl += builtin_assertion(layout, spv::BuiltInClipDistance, a, 0, 84 + 4 * a, SPIRType::Float, "gl_ClipDistance", 4 * a);
				msl += builtin_assertion(layout, spv::BuiltInLayer, 0, 0, 92, SPIRType::UInt, "gl_Layer", 0);
				msl += builtin_assertion(layout, spv::BuiltInViewportIndex, 0, 0, 96, SPIRType::UInt, "gl_ViewportIndex", 0);
				msl += "static_assert(__is_same(decltype(main0_out::gl_Layer), uint), \"physical layer type\");\n";
				msl += "static_assert(__is_same(decltype(main0_out::gl_ViewportIndex), uint), \"physical viewport type\");\n";
				msl += "static_assert(__is_same(decltype(main0_out::small), half3), \"physical half type\");\n";
				msl += "static_assert(__is_same(decltype(main0_out::label), ushort), \"physical ushort type\");\n";
				write_msl(directory, std::string("complex") + (native_arrays ? "-arrays" : "") + (remap_builtin ? "-remap" : compute ? "-compute" : ""), msl);
			}
		CompilerMSL ordinary(simple);
		ordinary.compile();
		rejects(ordinary, "vertex capture_output_to_buffer");
		CompilerMSL masked(simple);
		configure(masked);
		masked.mask_stage_output_by_location(0, 0);
		auto masked_msl = masked.compile();
		auto masked_layout = masked.get_msl_captured_vertex_layout();
		check(masked_layout.stride == 16 && masked_layout.components.empty() && masked_layout.builtins.size() == 4, "Masked output still changes exported record.");
		write_msl(directory, "masked", masked_msl + stride_assertion(masked_layout));
		CompilerMSL masked_builtins(complex);
		configure(masked_builtins);
		masked_builtins.mask_stage_output_by_builtin(spv::BuiltInPointSize);
		masked_builtins.mask_stage_output_by_builtin(spv::BuiltInClipDistance);
		auto masked_builtins_msl = masked_builtins.compile();
		auto builtin_layout = masked_builtins.get_msl_captured_vertex_layout();
		check(builtin_layout.stride == 96 && builtin_layout.builtins.size() == 6, "Masked builtins leaked into the capture layout.");
		for (const auto &field : builtin_layout.builtins)
			check(field.builtin != spv::BuiltInPointSize && field.builtin != spv::BuiltInClipDistance, "Masked builtin descriptor still present.");
		masked_builtins_msl += stride_assertion(builtin_layout);
		masked_builtins_msl += builtin_assertion(builtin_layout, spv::BuiltInLayer, 0, 0, 80, SPIRType::UInt, "gl_Layer", 0);
		masked_builtins_msl += builtin_assertion(builtin_layout, spv::BuiltInViewportIndex, 0, 0, 84, SPIRType::UInt, "gl_ViewportIndex", 0);
		write_msl(directory, "masked-builtins", masked_builtins_msl);
		for (bool mask_wide : {false, true})
		{
			CompilerMSL compiler(unsupported);
			configure(compiler);
			if (mask_wide)
				compiler.mask_stage_output_by_location(0, 0);
			compiler.compile();
			if (mask_wide)
			{
				auto layout = compiler.get_msl_captured_vertex_layout();
				check(layout.stride == 32 && layout.components.size() == 4, "Masked matrix layout failed.");
			}
			else
				rejects(compiler, "16/32-bit");
		}
		std::cout << "Captured producer layout checks passed.\n";
		return 0;
	}
	catch (const std::exception &error)
	{
		std::cerr << error.what() << '\n';
		return 1;
	}
}
