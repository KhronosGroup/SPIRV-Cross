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
	auto size = file.tellg();
	check(bool(file) && size > 0 && size % 4 == 0, "Invalid fixture.");
	std::vector<uint32_t> words(size_t(size) / 4);
	file.seekg(0);
	file.read(reinterpret_cast<char *>(words.data()), size);
	check(bool(file), "Cannot read fixture.");
	return words;
}

static void write(const std::string &prefix, const char *suffix, const std::string &source)
{
	if (prefix.empty())
		return;
	std::ofstream file(prefix + suffix + ".metal");
	file << source;
	check(bool(file), "Cannot write Metal evidence.");
}

class CaptureCompiler : public CompilerMSL
{
public:
	using CompilerMSL::CompilerMSL;
	void check_overflow()
	{
		const auto &record = get_variable_data_type(get<SPIRVariable>(stage_out_var_id));
		for (auto id : record.member_types)
		{
			auto &type = get<SPIRType>(id);
			if (type.array.empty())
				continue;
			type.array.back() = UINT32_MAX;
			try
			{
				get_msl_captured_vertex_layout();
				throw std::runtime_error("Accepted a record larger than the uint32 ABI.");
			}
			catch (const CompilerError &error)
			{
				check(std::string(error.what()).find("uint32") != std::string::npos, error.what());
				return;
			}
		}
		throw std::runtime_error("No array available for overflow check.");
	}
};

static void test(const std::vector<uint32_t> &vertex, const std::vector<uint32_t> &fragment, bool native, bool compute, const std::string &prefix)
{
	CaptureCompiler capture(vertex);
	CompilerMSL replay(vertex), consumer(fragment), raster(vertex);
	auto options = capture.get_msl_options();
	options.set_msl_version(2, 4);
	options.force_native_arrays = native;
	options.capture_output_to_buffer = true;
	options.vertex_for_tessellation = compute;
	capture.set_msl_options(options);
	auto captured = capture.compile();
	auto layout = capture.get_msl_captured_vertex_layout();
	check(layout.stride == 928 && layout.components.size() == 179 && layout.builtins.size() == 4, "Wrong recursive ABI size or field count.");
	options.capture_output_to_buffer = false;
	options.vertex_for_tessellation = false;
	replay.set_msl_options(options);
	raster.set_msl_options(options);
	consumer.set_msl_options(options);
	MSLCapturedOutputReplayBinding binding;
	binding.vertex_buffer_index = 0;
	binding.occurrence_buffer_index = 1;
	binding.draw_parameters_buffer_index = 2;
	binding.primitive_index_location = 120;
	auto replayed = replay.compile_captured_output_replay(layout, binding);
	auto normal = raster.compile();
	check(replayed.find("fill(") == std::string::npos && replayed.find("located.nested") == std::string::npos, "Replay emitted application functions or copy hooks.");
	MSLPerVertexInputBinding input;
	input.vertex_buffer_index = 0;
	input.primitive_index_buffer_index = 1;
	input.primitive_index_location = 120;
	consumer.set_msl_per_vertex_input_buffer(layout, input);
	auto consumed = consumer.compile();
	check(consumer.needs_per_vertex_input_buffer(), "Consumer did not use captured ABI.");
	captured += "\nstatic_assert(sizeof(main0_out) == 928, \"record stride\");\n";
	uint32_t checked_components = 0;
	auto array = [&](uint32_t index) { return std::string(native ? "[" : ".elements[") + std::to_string(index) + "]"; };
	auto vector = [&](uint32_t location, uint32_t component, uint32_t width, const std::string &path, uint32_t displacement, const std::string &output) {
		std::string attr = "[[user(locn" + std::to_string(location) + (component ? "_" + std::to_string(component) : "") + ")]]";
		check(replayed.find(output + " " + attr) != std::string::npos, "Wrong explicit replay linkage: " + output);
		check(normal.find(output + " " + attr) != std::string::npos, "Wrong explicit raster linkage: " + output);
		for (uint32_t c = 0; c < width; c++)
		{
			const MSLCapturedVertexComponent *found = nullptr;
			for (const auto &field : layout.components)
				if (field.location == location && field.component == component + c)
				{
					check(!found, "Duplicate captured linkage.");
					found = &field;
				}
			check(found && found->scalar_type == SPIRType::Float, "Missing or mistyped expected field.");
			captured += "static_assert(__builtin_offsetof(main0_out, " + path + ") + " + std::to_string(displacement + c * 4) + " == " + std::to_string(found->byte_offset) + ", \"physical offset\");\n";
			std::string target = "out." + output + (width > 1 ? "[" + std::to_string(c) + "]" : "");
			check(replayed.find(target + " = *reinterpret_cast<const device float*>(spvReplayRecord + " + std::to_string(found->byte_offset) + "ul);") != std::string::npos, "Replay loaded the wrong scalar.");
			checked_components++;
		}
	};
	auto leaf = [&](uint32_t location, const std::string &path, const std::string &output) {
		vector(location, 0, 3, path + ".color", 0, output + "_color");
		for (uint32_t i = 0; i < 2; i++)
			vector(location + 1 + i, 0, 1, path + ".weights" + array(i), 0, output + "_weights_" + std::to_string(i));
	};
	auto node = [&](uint32_t location, const std::string &path, const std::string &output) {
		for (uint32_t i = 0; i < 4; i++)
		{
			leaf(location + 3 * i, path + ".branches" + array(i / 2) + array(i % 2), output + "_branches_" + std::to_string(i));
			for (uint32_t col = 0; col < 3; col++)
				vector(location + 12 + 3 * i + col, 0, 2, path + ".matrices" + array(i / 2) + array(i % 2), col * 8, output + "_matrices_" + std::to_string(3 * i + col));
		}
	};
	node(4, "located_nested", "located_nested");
	node(80, "topNodes" + array(0) + array(0), "topNodes_0");
	vector(2, 1, 1, "located_early", 0, "located_early");
	for (uint32_t i = 0; i < 2; i++)
		leaf(50 + 3 * i, "located_structs" + array(0) + array(i), "located_structs_" + std::to_string(i));
	for (uint32_t i = 0; i < 4; i++)
	{
		for (uint32_t col = 0; col < 2; col++)
		{
			vector(32 + 2 * i + col, 0, 3, "located_basis" + array(i / 2) + array(i % 2), col * 16, "located_basis_" + std::to_string(2 * i + col));
			vector(60 + 2 * i + col, 0, 3, "tops" + array(i / 2) + array(i % 2), col * 16, "tops_" + std::to_string(2 * i + col));
		}
		vector(42 + i, 2, 2, "located_upper" + array(i / 2) + array(i % 2), 0, "located_upper_" + std::to_string(i));
		vector(42 + i, 0, 2, "located_lower" + array(i / 2) + array(i % 2), 0, "located_lower_" + std::to_string(i));
		vector(70 + i, 2, 2, "m_location_" + std::to_string(70 + i), 8, "topHi_" + std::to_string(i));
		vector(70 + i, 0, 2, "m_location_" + std::to_string(70 + i), 0, "topLo_" + std::to_string(i));
	}
	check(checked_components == layout.components.size(), "Unchecked exported components.");
	write(prefix, "-capture", captured);
	write(prefix, "-replay", replayed);
	write(prefix, "-raster", normal);
	write(prefix, "-fragment", consumed);
	capture.check_overflow();
	// Missing coverage must fail before returning any replay shader.
	layout.components.pop_back();
	CompilerMSL incomplete(vertex);
	incomplete.set_msl_options(options);
	try
	{
		incomplete.compile_captured_output_replay(layout, binding);
		throw std::runtime_error("Replay accepted incomplete composite coverage.");
	}
	catch (const CompilerError &error)
	{
		check(std::string(error.what()).find("missing Location") != std::string::npos, error.what());
	}
}

static void test_mixed_components(const std::vector<uint32_t> &words, bool native, bool compute, const std::string &prefix)
{
	CompilerMSL capture(words), replay(words);
	auto options = capture.get_msl_options();
	options.set_msl_version(2, 4);
	options.force_native_arrays = native;
	options.capture_output_to_buffer = true;
	options.vertex_for_tessellation = compute;
	capture.set_msl_options(options);
	auto source = capture.compile();
	auto layout = capture.get_msl_captured_vertex_layout();
	check(layout.stride == 64 && layout.components.size() == 8, "Component mask leaked into a separate block member.");
	source += "\nstatic_assert(sizeof(main0_out) == 64, \"mixed stride\");\n";
	for (uint32_t loc = 0; loc < 2; loc++)
		for (uint32_t c = 0; c < 4; c++)
		{
			bool found = false;
			for (const auto &field : layout.components)
				if (field.location == loc && field.component == c)
				{
					uint32_t offset = c < 2 ? loc * 8 + c * 4 : 16 + loc * 16 + c * 4;
					check(!found && field.byte_offset == offset, "Wrong mixed Component byte offset.");
					found = true;
					std::string path = c < 2 ? "lower_values" + std::string(native ? "[" : ".elements[") + std::to_string(loc) + "]" : "m_location_" + std::to_string(loc);
					source += "static_assert(__builtin_offsetof(main0_out, " + path + ") + " + std::to_string(c * 4) + " == " + std::to_string(offset) + ", \"mixed offset\");\n";
				}
			check(found, "Missing mixed Location/Component.");
		}
	options.capture_output_to_buffer = false;
	options.vertex_for_tessellation = false;
	replay.set_msl_options(options);
	MSLCapturedOutputReplayBinding binding;
	binding.vertex_buffer_index = 0;
	binding.occurrence_buffer_index = 1;
	binding.draw_parameters_buffer_index = 2;
	binding.primitive_index_location = 120;
	write(prefix, "-mixed-capture", source);
	write(prefix, "-mixed-replay", replay.compile_captured_output_replay(layout, binding));
}

static void test_typed(const std::vector<uint32_t> &vertex, const std::vector<uint32_t> &fragment, bool native, bool compute, const std::string &prefix)
{
	CompilerMSL capture(vertex), replay(vertex), raster(vertex), consumer(fragment);
	auto options = capture.get_msl_options();
	options.set_msl_version(2, 4);
	options.force_native_arrays = native;
	options.capture_output_to_buffer = true;
	options.vertex_for_tessellation = compute;
	capture.set_msl_options(options);
	auto source = capture.compile();
	auto layout = capture.get_msl_captured_vertex_layout();
	check(layout.stride == 176 && layout.components.size() == 46 && layout.builtins.size() == 4, "Wrong typed composite ABI.");
	options.capture_output_to_buffer = false;
	options.vertex_for_tessellation = false;
	replay.set_msl_options(options);
	raster.set_msl_options(options);
	consumer.set_msl_options(options);
	MSLCapturedOutputReplayBinding binding;
	binding.vertex_buffer_index = 0;
	binding.occurrence_buffer_index = 1;
	binding.draw_parameters_buffer_index = 2;
	binding.primitive_index_location = 120;
	auto replayed = replay.compile_captured_output_replay(layout, binding);
	auto normal = raster.compile();
	MSLPerVertexInputBinding input;
	input.vertex_buffer_index = 0;
	input.primitive_index_buffer_index = 1;
	input.primitive_index_location = 120;
	consumer.set_msl_per_vertex_input_buffer(layout, input);
	auto consumed = consumer.compile();
	source += "\nstatic_assert(sizeof(main0_out) == 176, \"typed stride\");\n";
	uint32_t checked_components = 0;
	auto array = [&](uint32_t i) { return std::string(native ? "[" : ".elements[") + std::to_string(i) + "]"; };
	auto leaf = [&](uint32_t location, uint32_t width, SPIRType::BaseType type, const char *scalar, uint32_t bytes, uint32_t offset, const std::string &path, uint32_t displacement, const std::string &output) {
		std::string declaration = std::string(scalar) + (width > 1 ? std::to_string(width) : "") + " " + output + " [[user(locn" + std::to_string(location) + ")]]";
		check(normal.find(declaration) != std::string::npos && replayed.find(declaration) != std::string::npos, "Wrong typed raster/replay linkage: " + output);
		for (uint32_t c = 0; c < width; c++)
		{
			bool found = false;
			for (const auto &field : layout.components)
				if (field.location == location && field.component == c)
				{
					check(!found && field.scalar_type == type && field.byte_offset == offset + c * bytes, "Wrong typed scalar ABI: " + output);
					found = true;
				}
			check(found, "Missing typed scalar: " + output);
			std::string target = "out." + output + (width > 1 ? "[" + std::to_string(c) + "]" : "");
			check(replayed.find(target + " = *reinterpret_cast<const device " + scalar + "*>(spvReplayRecord + " + std::to_string(offset + c * bytes) + "ul);") != std::string::npos, "Wrong typed replay load: " + output);
			source += "static_assert(__builtin_offsetof(main0_out, " + path + ") + " + std::to_string(displacement + c * bytes) + " == " + std::to_string(offset + c * bytes) + ", \"typed offset\");\n";
			checked_components++;
		}
	};
	for (uint32_t n = 0; n < 2; n++)
	{
		std::string base = "typed_" + std::to_string(n) + "_";
		std::string path = "typed" + array(n) + ".";
		leaf(n * 10, 3, SPIRType::Half, "half", 2, n * 80, path + "h", 0, base + "h");
		for (uint32_t a = 0; a < 2; a++)
			leaf(n * 10 + 1 + a, 1, SPIRType::Short, "short", 2, n * 80 + 8 + a * 2, path + "s" + array(a), 0, base + "s_" + std::to_string(a));
		leaf(n * 10 + 3, 1, SPIRType::UShort, "ushort", 2, n * 80 + 12, path + "u", 0, base + "u");
		leaf(n * 10 + 4, 2, SPIRType::Int, "int", 4, n * 80 + 16, path + "i", 0, base + "i");
		leaf(n * 10 + 5, 3, SPIRType::UInt, "uint", 4, n * 80 + 32, path + "j", 0, base + "j");
		for (uint32_t col = 0; col < 4; col++)
			leaf(n * 10 + 6 + col, 3, SPIRType::Half, "half", 2, n * 80 + 48 + col * 8, path + "m" + array(col / 2), (col % 2) * 8, base + "m_" + std::to_string(col));
	}
	check(checked_components == layout.components.size(), "Unchecked typed scalar.");
	write(prefix, "-typed-capture", source);
	write(prefix, "-typed-replay", replayed);
	write(prefix, "-typed-raster", normal);
	write(prefix, "-typed-fragment", consumed);
}

static void reject_specialization_array(const std::vector<uint32_t> &words, bool native, bool capture_output, bool compute)
{
	CompilerMSL compiler(words);
	auto options = compiler.get_msl_options();
	options.set_msl_version(2, 4);
	options.force_native_arrays = native;
	options.capture_output_to_buffer = capture_output;
	options.vertex_for_tessellation = compute;
	compiler.set_msl_options(options);
	try
	{
		compiler.compile();
	}
	catch (const CompilerError &error)
	{
		check(std::string(error.what()).find("MSL interface composite flattening requires literal array sizes.") != std::string::npos, error.what());
		return;
	}
	throw std::runtime_error("Specialization-sized interface array was flattened using its default size.");
}

int main(int argc, char **argv)
{
	try
	{
		check(argc >= 7, "Usage: test vertex.spv fragment.spv mixed.spv spec-array.spv typed-vertex.spv typed-fragment.spv [output-directory]");
		auto vertex = read_spirv(argv[1]);
		auto fragment = read_spirv(argv[2]);
		auto mixed = read_spirv(argv[3]);
		auto spec_array = read_spirv(argv[4]);
		auto typed_vertex = read_spirv(argv[5]);
		auto typed_fragment = read_spirv(argv[6]);
		for (bool native : {false, true})
		{
			for (bool compute : {false, true})
			{
				std::string prefix = argc > 7 ? std::string(argv[7]) + "/composites" + (native ? "-native" : "") + (compute ? "-compute" : "") : "";
				test(vertex, fragment, native, compute, prefix);
				test_mixed_components(mixed, native, compute, prefix);
				test_typed(typed_vertex, typed_fragment, native, compute, prefix);
				reject_specialization_array(spec_array, native, true, compute);
			}
			reject_specialization_array(spec_array, native, false, false);
		}
		std::cout << "Recursive capture, replay and consumer linkage checks passed.\n";
		return 0;
	}
	catch (const std::exception &error)
	{
		std::cerr << error.what() << '\n';
		return 1;
	}
}
