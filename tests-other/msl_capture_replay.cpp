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
#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
using namespace spirv_cross;

static void check(bool condition, const std::string &message)
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

static void configure(CompilerMSL &compiler, bool capture = false, bool native_arrays = false, bool compute = false)
{
	auto options = compiler.get_msl_options();
	options.set_msl_version(2, 4);
	options.capture_output_to_buffer = capture;
	options.force_native_arrays = native_arrays;
	options.vertex_for_tessellation = compute;
	compiler.set_msl_options(options);
}

static size_t count(const std::string &source, const std::string &needle)
{
	size_t result = 0;
	for (size_t pos = 0; (pos = source.find(needle, pos)) != std::string::npos; pos += needle.size())
		result++;
	return result;
}

static std::string output_struct(const std::string &source)
{
	auto begin = source.find("struct main0_out\n");
	check(begin != std::string::npos, "Missing raster output struct.");
	auto end = source.find("};", begin);
	check(end != std::string::npos, "Unterminated output struct.");
	return source.substr(begin, end + 2 - begin);
}

static void check_interface(const std::string &normal, const std::string &replay)
{
	auto actual = output_struct(replay);
	auto attr = actual.find("[[user(locn15), flat]]");
	check(attr != std::string::npos, "Missing private flat key.");
	auto begin = actual.rfind('\n', attr);
	auto end = actual.find('\n', attr);
	actual.erase(begin, end - begin);
	check(actual == output_struct(normal), "Replay does not preserve the normal raster interface.\n" + actual + "\n" + output_struct(normal));
}

static void write(const std::string &directory, const std::string &name, const std::string &source)
{
	if (directory.empty())
		return;
	std::ofstream file(directory + "/" + name + ".metal");
	file << source;
	check(bool(file), "Cannot write MSL fixture.");
}

static void rejects(const std::function<void()> &operation, const char *diagnostic)
{
	try { operation(); }
	catch (const CompilerError &error)
	{
		check(std::string(error.what()).find(diagnostic) != std::string::npos, std::string("Unexpected diagnostic: ") + error.what());
		return;
	}
	throw std::runtime_error(std::string("Expected diagnostic: ") + diagnostic);
}

static void check_barycentric_replay(const std::vector<uint32_t> &words, const MSLCapturedOutputReplayBinding &binding, const std::string &directory, const std::string &name)
{
	CompilerMSL capture(words);
	configure(capture, true);
	capture.compile();
	auto layout = capture.get_msl_captured_vertex_layout();
	for (uint32_t mode = 1; mode <= 3; mode++)
		for (uint32_t fixups = 0; fixups < 4; fixups++)
		{
			MSLCapturedOutputReplayBarycentricBinding barycentrics;
			barycentrics.corner_buffer_index = 30;
			barycentrics.perspective_location = (mode & 1) ? 12u : ~0u;
			barycentrics.no_perspective_location = (mode & 2) ? 14u : ~0u;
			CompilerMSL replay(words), ordinary(words), disabled(words);
			for (auto compiler : {&replay, &ordinary, &disabled})
			{
				configure(*compiler);
				auto options = compiler->get_msl_options();
				options.emulate_reversed_depth_viewport = true;
				options.reversed_depth_viewport_buffer_index = 29;
				compiler->set_msl_options(options);
				auto common = compiler->get_common_options();
				common.vertex.fixup_clipspace = (fixups & 1) != 0;
				common.vertex.flip_vert_y = (fixups & 2) != 0;
				compiler->set_common_options(common);
			}
			auto original = ordinary.compile_captured_output_replay(layout, binding);
			check(disabled.compile_captured_output_replay(layout, binding, {}) == original, "Default barycentric binding changed legacy replay source.");
			auto msl = replay.compile_captured_output_replay(layout, binding, barycentrics);
			check(count(msl, "[[buffer(") == 5, "Barycentric replay must add exactly one corner buffer.");
			const std::string argument = ", const device uint* spvReplayCorners [[buffer(30)]]";
			check(count(msl, argument) == 1, "Missing explicit corner buffer argument.");
			check(count(msl, "uint spvReplayCorner = spvReplayCorners[spvReplayOccurrence];") == 1, "Corner must use the explicit normalized occurrence address.");
			check(msl.find('%') == std::string::npos && msl.find("barycentric_coord") == std::string::npos, "Replay inferred a corner or used native barycentrics.");
			for (uint32_t bit = 1; bit <= 2; bit++)
			{
				std::string field = bit == 1 ? "spvReplayBarycentric" : "spvReplayBarycentricNoPersp";
				std::string location = bit == 1 ? "12" : "14";
				check(count(msl, "float3 " + field + " [[user(locn" + location + ")]];") == ((mode & bit) ? 1u : 0u), "Incorrect barycentric output type, location or interpolation qualifier.");
				check(count(msl, "out." + field + " = float3(spvReplayCorner == 0u, spvReplayCorner == 1u, spvReplayCorner == 2u);") == ((mode & bit) ? 1u : 0u), "Basis must be unscaled one-hot for the explicit corner.");
				check(!replay.is_msl_shader_output_used(bit == 1 ? 12 : 14), "Private basis leaked into application output reflection.");
			}
			// Removing only the optional interface, argument and basis must recover every
			// captured scalar load, primitive key, Position component and fixup unchanged.
			auto stripped = msl;
			stripped.erase(stripped.find(argument), argument.size());
			for (size_t pos; (pos = stripped.find("spvReplayCorner")) != std::string::npos;)
			{
				auto begin = stripped.rfind('\n', pos);
				stripped.erase(begin, stripped.find('\n', pos) - begin);
			}
			for (size_t pos; (pos = stripped.find("spvReplayBarycentric")) != std::string::npos;)
			{
				auto begin = stripped.rfind('\n', pos);
				stripped.erase(begin, stripped.find('\n', pos) - begin);
			}
			check(stripped == original, "Barycentric output changed captured outputs, Position, primitive key, addressing or fixups.");
			write(directory, name + "-barycentric-" + std::to_string(mode) + "-fixups-" + std::to_string(fixups), msl);
		}
	MSLCapturedOutputReplayBarycentricBinding barycentrics;
	barycentrics.corner_buffer_index = 30;
	barycentrics.perspective_location = 12;
	barycentrics.no_perspective_location = 14;
	for (uint32_t slot : {0u, 1u, 2u, 29u, 31u, ~0u})
	{
		CompilerMSL compiler(words);
		configure(compiler);
		auto options = compiler.get_msl_options();
		options.emulate_reversed_depth_viewport = true;
		options.reversed_depth_viewport_buffer_index = 29;
		compiler.set_msl_options(options);
		auto bad = barycentrics;
		bad.corner_buffer_index = slot;
		rejects([&]() { compiler.compile_captured_output_replay(layout, binding, bad); }, "corner buffer index");
	}
	for (uint32_t failure = 0; failure < 8; failure++)
	{
		CompilerMSL compiler(words);
		configure(compiler);
		auto bad = barycentrics;
		auto bad_layout = layout;
		const char *diagnostic = nullptr;
		switch (failure)
		{
		case 0: bad.perspective_location = bad.no_perspective_location = ~0u; diagnostic = "at least one private Location"; break;
		case 1: bad.no_perspective_location = bad.perspective_location; diagnostic = "distinct private Locations"; break;
		case 2: bad.perspective_location = binding.primitive_index_location; diagnostic = "primitive key"; break;
		case 3: bad.no_perspective_location = binding.primitive_index_location; diagnostic = "primitive key"; break;
		case 4: bad.perspective_location = layout.components.front().location; diagnostic = "captured layout"; break;
		case 5: bad.no_perspective_location = layout.components.front().location; diagnostic = "captured layout"; break;
		case 6:
		{
			MSLShaderInterfaceVariable output;
			output.location = 12;
			output.component = 2;
			compiler.add_msl_shader_output(output);
			diagnostic = "MSL shader output mapping";
			break;
		}
		case 7:
			bad.perspective_location = layout.components.front().location;
			bad_layout.components.clear();
			diagnostic = "raster output interface";
			break;
		}
		rejects([&]() { compiler.compile_captured_output_replay(bad_layout, binding, bad); }, diagnostic);
		rejects([&]() { compiler.compile_captured_output_replay(layout, binding); }, "fresh compiler");
	}
}

int main(int argc, char **argv)
{
	try
	{
		check(argc >= 6, "Usage: test simple.spv complex.spv effects.spv tessellation.spv unsupported.spv [output-directory]");
		auto simple = read_spirv(argv[1]);
		auto complex = read_spirv(argv[2]);
		auto effects = read_spirv(argv[3]);
		auto tessellation = read_spirv(argv[4]);
		auto unsupported = read_spirv(argv[5]);
		std::string directory = argc > 6 ? argv[6] : "";
		std::vector<uint32_t> dense = argc > 7 ? read_spirv(argv[7]) : std::vector<uint32_t>{};
		MSLCapturedOutputReplayBinding binding;
		binding.vertex_buffer_index = 0;
		binding.occurrence_buffer_index = 1;
		binding.draw_parameters_buffer_index = 2;
		binding.primitive_index_location = 15;
		check_barycentric_replay(simple, binding, directory, "simple");
		check_barycentric_replay(complex, binding, directory, "complex");
		check_barycentric_replay(effects, binding, directory, "effects");
		MSLCapturedVertexLayout simple_layout;
		for (uint32_t fixture = 0; fixture < 3; fixture++)
			for (bool native_arrays : {false, true})
				for (bool compute : {false, true})
				{
					const auto &words = fixture == 0 ? simple : fixture == 1 ? complex : effects;
					CompilerMSL capture(words), normal(words), replay(words);
					configure(capture, true, native_arrays, compute);
					configure(normal, false, native_arrays);
					configure(replay, false, native_arrays);
					auto capture_msl = capture.compile();
					auto layout = capture.get_msl_captured_vertex_layout();
					auto normal_msl = normal.compile();
					auto replay_msl = replay.compile_captured_output_replay(layout, binding);
					check_interface(normal_msl, replay_msl);
					check(replay_msl.find("vertex main0_out main0(") != std::string::npos, "Replay is not a normal vertex entry point.");
					check(count(replay_msl, "[[buffer(") == 3 && replay_msl.find("[[stage_in]]") == std::string::npos, "Replay emitted application inputs/resources.");
					check(replay_msl.find("2ul * spvReplayOccurrence + 1ul") != std::string::npos && replay_msl.find("ulong(spvReplayOccurrences[2ul * spvReplayOccurrence]) * " + std::to_string(layout.stride) + "ul") != std::string::npos, "Incorrect record/key addressing.");
					check(replay_msl.find("(ulong(spvReplayInstance) - ulong(spvReplayDraw[2])) * ulong(spvReplayDraw[3])") != std::string::npos && replay_msl.find("(ulong(spvReplayVertex) - ulong(spvReplayDraw[1]))") != std::string::npos, "Missing explicit instance/vertex normalization.");
					if (fixture == 0)
					{
						simple_layout = layout;
						check(count(replay_msl, "reinterpret_cast<const device float*>") == 7, "Simple replay must load color and Position once each.");
						check(replay_msl.find("out.gl_Position[3] = *reinterpret_cast<const device float*>(spvReplayRecord + 28ul)") != std::string::npos, "Incorrect Position offset.");
					}
					if (fixture == 1)
					{
						check(replay_msl.find("out.pair[0] = *reinterpret_cast<const device float*>(spvReplayRecord + 8ul)") != std::string::npos, "Component-packed capture was not unpacked.");
						check(replay_msl.find("out.gl_Layer = *reinterpret_cast<const device uint*>(spvReplayRecord + 92ul)") != std::string::npos, "Layer physical type/offset lost.");
						check(replay_msl.find("out.gl_ClipDistance[1] = *reinterpret_cast<const device float*>(spvReplayRecord + 88ul)") != std::string::npos, "ClipDistance array element lost.");
						check(count(replay_msl, "reinterpret_cast<const device ") == 24, "Complex replay must load all fields, including clip user aliases.");
					}
					if (fixture == 2)
					{
						check(capture_msl.find("atomic_fetch_add_explicit") != std::string::npos, "Side-effect control fixture has no atomic write.");
						check(replay_msl.find("atomic") == std::string::npos && replay_msl.find("AppSideEffects") == std::string::npos && replay_msl.find("app_write_side_effect") == std::string::npos, "Replay contains application side effects or resources.");
						// As in a normal MSL VS, application interpolation is selected by the fragment.
						check(replay_msl.find("[[user(locn0_3)]]") != std::string::npos, "Component linkage qualifier lost.");
					}
					std::string name = std::string(fixture == 0 ? "simple" : fixture == 1 ? "complex" : "effects") + (native_arrays ? "-arrays" : "") + (compute ? "-compute" : "");
					write(directory, name + "-capture", capture_msl);
					write(directory, name + "-normal", normal_msl);
					write(directory, name + "-replay", replay_msl);
				}
		CompilerMSL selected_none(simple), selected_color(simple);
		configure(selected_none);
		configure(selected_color);
		auto position_only = selected_none.compile_captured_output_replay(simple_layout, binding, {}, std::vector<uint32_t>{});
		auto color_and_position = selected_color.compile_captured_output_replay(simple_layout, binding, {}, std::vector<uint32_t>{0});
		check(position_only.find("[[user(locn0)]]") == std::string::npos && position_only.find("out.color") == std::string::npos, "Unconsumed user varying was emitted by selected replay.");
		check(position_only.find("gl_Position [[position]]") != std::string::npos && position_only.find("spvReplayPrimitive [[user(locn15), flat]]") != std::string::npos, "Selected replay lost fixed Position or the private key.");
		check(position_only.find("spvReplayRecord + 16ul") != std::string::npos && position_only.find("spvReplayRecord + 0ul") == std::string::npos, "Selected replay read a discarded user varying or lost the captured Position offset.");
		check(color_and_position.find("float3 color [[user(locn0)]]") != std::string::npos && color_and_position.find("out.color[2]") != std::string::npos, "Selected replay dropped a required user varying.");
		CompilerMSL selected_barycentrics(simple);
		configure(selected_barycentrics);
		MSLCapturedOutputReplayBarycentricBinding selected_basis;
		selected_basis.corner_buffer_index = 3;
		selected_basis.perspective_location = 12;
		selected_basis.no_perspective_location = 14;
		auto basis_only = selected_barycentrics.compile_captured_output_replay(simple_layout, binding, selected_basis, std::vector<uint32_t>{});
		check(basis_only.find("[[user(locn0)]]") == std::string::npos && basis_only.find("[[position]]") != std::string::npos, "Selected barycentric replay retained an ordinary varying or lost Position.");
		check(basis_only.find("spvReplayPrimitive [[user(locn15), flat]]") != std::string::npos && basis_only.find("spvReplayBarycentric [[user(locn12)]]") != std::string::npos && basis_only.find("spvReplayBarycentricNoPersp [[user(locn14)]]") != std::string::npos, "Selected replay lost its private key or barycentric bases.");
		write(directory, "simple-selected-position", position_only);
		write(directory, "simple-selected-barycentrics", basis_only);
		write(directory, "simple-selected-color", color_and_position);
		for (const auto &locations : {std::vector<uint32_t>{1}, std::vector<uint32_t>{0, 0}, std::vector<uint32_t>{~0u}})
		{
			CompilerMSL invalid(simple);
			configure(invalid);
			rejects([&]() { invalid.compile_captured_output_replay(simple_layout, binding, {}, locations); }, "selected replay Location");
		}
		CompilerMSL complex_capture(complex), complex_selected(complex);
		configure(complex_capture, true);
		configure(complex_selected);
		complex_capture.compile();
		auto complex_layout = complex_capture.get_msl_captured_vertex_layout();
		std::vector<uint32_t> complex_locations;
		for (const auto &field : complex_layout.components)
			if (field.location != 7 && std::find(complex_locations.begin(), complex_locations.end(), field.location) == complex_locations.end())
				complex_locations.push_back(field.location);
		auto complex_msl = complex_selected.compile_captured_output_replay(complex_layout, binding, {}, complex_locations);
		check(complex_msl.find("[[user(locn7)]]") == std::string::npos && complex_msl.find("out.later") == std::string::npos, "Selected replay retained an unused Location.");
		for (const char *fixed : {"[[position]]", "[[point_size]]", "[[clip_distance]]", "[[user(clip0)]]", "[[render_target_array_index]]", "[[viewport_array_index]]"})
			check(complex_msl.find(fixed) != std::string::npos, std::string("Selected replay lost fixed raster output: ") + fixed);
		write(directory, "complex-selected", complex_msl);
		if (!dense.empty())
		{
			CompilerMSL dense_capture(dense), dense_full(dense), dense_selected(dense);
			configure(dense_capture, true);
			configure(dense_full);
			configure(dense_selected);
			auto dense_capture_msl = dense_capture.compile();
			auto dense_layout = dense_capture.get_msl_captured_vertex_layout();
			check(dense_layout.components.size() == 120 && dense_layout.builtins.size() == 4, "Dense capture did not retain all fixed Vulkan outputs.");
			auto dense_binding = binding;
			dense_binding.primitive_index_location = 30;
			MSLCapturedOutputReplayBarycentricBinding dense_basis;
			dense_basis.corner_buffer_index = 3;
			dense_basis.perspective_location = 31;
			dense_basis.no_perspective_location = 32;
			auto full_msl = dense_full.compile_captured_output_replay(dense_layout, dense_binding, dense_basis);
			auto selected_msl = dense_selected.compile_captured_output_replay(dense_layout, dense_binding, dense_basis, std::vector<uint32_t>{0});
			check(count(full_msl, "[[user(locn") == 33 && count(selected_msl, "[[user(locn") == 4, "Dense replay did not reduce 30 user varyings to the single requested Location and three private varyings.");
			check(full_msl.find("value29 [[user(locn29)]]") != std::string::npos && selected_msl.find("value29") == std::string::npos, "Dense replay retained an unconsumed output.");
			check(selected_msl.find("value0 [[user(locn0)]]") != std::string::npos && selected_msl.find("[[position]]") != std::string::npos, "Dense replay lost its selected or fixed output.");
			std::map<uint32_t, uint32_t> captured_widths;
			for (const auto &field : dense_layout.components)
				captured_widths[field.location] = std::max(captured_widths[field.location], field.component + 1);
			uint32_t full_components = 7, selected_components = 7; // Primitive key and two float3 barycentric bases.
			for (const auto &width : captured_widths)
			{
				full_components += width.second;
				if (width.first == 0)
					selected_components += width.second;
			}
			check(full_components == 127 && selected_components == 11 && full_components > 124 && selected_components <= 124, "Selected replay did not cross the illustrative 124-component capacity boundary.");
			write(directory, "dense-capture", dense_capture_msl);
			write(directory, "dense-selected", selected_msl);
		}
		for (uint32_t mode = 0; mode < 3; mode++)
		{
			CompilerMSL capture(complex), normal(complex), replay(complex);
			configure(capture, true);
			configure(normal);
			configure(replay);
			for (auto compiler : {&capture, &normal, &replay})
			{
				if (mode == 0)
				{
					compiler->mask_stage_output_by_location(7, 0);
					compiler->mask_stage_output_by_builtin(spv::BuiltInPointSize);
					compiler->mask_stage_output_by_builtin(spv::BuiltInClipDistance);
				}
				else if (mode == 1)
				{
					MSLShaderInterfaceVariable output;
					output.builtin = spv::BuiltInPosition;
					output.location = 1;
					compiler->add_msl_shader_output(output);
				}
				else
				{
					auto options = compiler->get_msl_options();
					options.enable_clip_distance_user_varying = false;
					compiler->set_msl_options(options);
				}
			}
			auto producer = capture.compile();
			auto raster = normal.compile();
			auto msl = replay.compile_captured_output_replay(capture.get_msl_captured_vertex_layout(), binding);
			check_interface(raster, msl);
			if (mode == 0)
				check(msl.find("out.later") == std::string::npos && msl.find("gl_ClipDistance") == std::string::npos && msl.find("gl_PointSize") == std::string::npos, "Masked outputs leaked into replay.");
			std::string name = mode == 0 ? "masked" : mode == 1 ? "builtin-remap" : "no-clip-varying";
			write(directory, name + "-capture", producer);
			write(directory, name + "-normal", raster);
			write(directory, name + "-replay", msl);
		}
		for (bool clip : {false, true})
			for (bool flip : {false, true})
			{
				CompilerMSL capture(simple), replay(simple), normal(simple);
				configure(capture, true);
				configure(replay);
				configure(normal);
				for (auto compiler : {&capture, &replay, &normal})
				{
					auto options = compiler->get_common_options();
					options.vertex.fixup_clipspace = clip;
					options.vertex.flip_vert_y = flip;
					compiler->set_common_options(options);
				}
				auto producer = capture.compile();
				auto msl = replay.compile_captured_output_replay(capture.get_msl_captured_vertex_layout(), binding);
				check(count(producer, "Adjust clip-space") == 0 && count(producer, "Invert Y-axis") == 0, "Capture unexpectedly applies raster fixups.");
				check(count(msl, "Adjust clip-space") == size_t(clip) && count(msl, "Invert Y-axis") == size_t(flip), "Replay fixups missing or repeated.");
				check(count(msl, "out.gl_Position.z = (out.gl_Position.z + out.gl_Position.w) * 0.5;") == size_t(clip) && count(msl, "out.gl_Position.y = -(out.gl_Position.y);") == size_t(flip), "Incorrect raster fixup expression.");
				check_interface(normal.compile(), msl);
				if (clip || flip)
					check(msl.find("out.gl_Position[3] =") < msl.find(clip ? "Adjust clip-space" : "Invert Y-axis"), "Fixup precedes captured Position load.");
				write(directory, std::string("fixups-") + (clip ? "clip" : "no-clip") + (flip ? "-flip" : "-no-flip"), msl);
			}
		for (uint32_t viewport = 0; viewport < 2; viewport++)
			for (bool compute : {false, true})
				for (bool clip : {false, true})
					for (bool flip : {false, true})
					{
						const auto &words = viewport == 0 ? simple : complex;
						CompilerMSL capture(words), normal(words), replay(words);
						configure(capture, true, false, compute);
						configure(normal);
						configure(replay);
						for (auto compiler : {&capture, &normal, &replay})
						{
							auto options = compiler->get_msl_options();
							options.emulate_reversed_depth_viewport = true;
							options.reversed_depth_viewport_buffer_index = 30;
							compiler->set_msl_options(options);
							auto common = compiler->get_common_options();
							common.vertex.fixup_clipspace = clip;
							common.vertex.flip_vert_y = flip;
							compiler->set_common_options(common);
						}
						auto producer = capture.compile();
						auto raster = normal.compile();
						auto msl = replay.compile_captured_output_replay(capture.get_msl_captured_vertex_layout(), binding);
						check_interface(raster, msl);
						const std::string argument = "constant uint& spvEmulatedReversedDepthViewportMask [[buffer(30)]]";
						const std::string guard = viewport == 1 ? "if (((spvEmulatedReversedDepthViewportMask >> uint(out.gl_ViewportIndex)) & 1u) != 0u)" : "if ((spvEmulatedReversedDepthViewportMask & 1u) != 0u)";
						const std::string correction = "out.gl_Position.z = out.gl_Position.w - out.gl_Position.z;    // Emulate reversed-depth viewport";
						check(producer.find("spvEmulatedReversedDepthViewportMask") == std::string::npos && count(producer, "Emulate reversed-depth viewport") == 0 && count(producer, "Adjust clip-space") == 0 && count(producer, "Invert Y-axis") == 0, "Capture must leave Position uncorrected and require no mask.");
						check(count(msl, "[[buffer(") == 4, "Enabled replay must emit exactly four buffers.");
						for (const auto &source : {raster, msl})
						{
							check(count(source, argument) == 1 && count(source, guard) == 1 && count(source, correction) == 1, "Ordinary/replay reversed-depth argument, viewport selection or correction differs.");
							check(count(source, "Adjust clip-space") == size_t(clip) && count(source, "Invert Y-axis") == size_t(flip), "Reversed-depth raster fixups missing or repeated.");
							check(!clip || source.find("Adjust clip-space") < source.find(guard), "Clip conversion must precede reversed depth.");
							check(!flip || source.find(correction) < source.find("Invert Y-axis"), "Y inversion must follow reversed depth.");
						}
						check(msl.rfind("reinterpret_cast<const device ") < msl.find(guard) && msl.find(correction) < msl.find("return out;"), "Replay must correct depth after captured loads and before return.");
						if (viewport == 1)
							check(count(msl, "out.gl_ViewportIndex = *reinterpret_cast<const device uint*>(spvReplayRecord + 96ul)") == 1, "Viewport selection must use the captured ViewportIndex.");
						else
							check(msl.find("gl_ViewportIndex") == std::string::npos, "Absent/masked ViewportIndex must use viewport zero.");
						std::string name = "reversed-" + std::to_string(viewport) + (compute ? "-compute" : "-vertex") + (clip ? "-clip" : "-no-clip") + (flip ? "-flip" : "-no-flip");
						write(directory, name + "-capture", producer);
						write(directory, name + "-normal", raster);
						write(directory, name + "-replay", msl);
					}
		CompilerMSL viewport_capture(complex), masked_viewport_replay(complex);
		configure(viewport_capture, true);
		configure(masked_viewport_replay);
		viewport_capture.compile();
		masked_viewport_replay.mask_stage_output_by_builtin(spv::BuiltInViewportIndex);
		auto masked_options = masked_viewport_replay.get_msl_options();
		masked_options.emulate_reversed_depth_viewport = true;
		masked_options.reversed_depth_viewport_buffer_index = 3;
		masked_viewport_replay.set_msl_options(masked_options);
		auto masked_msl = masked_viewport_replay.compile_captured_output_replay(viewport_capture.get_msl_captured_vertex_layout(), binding);
		check(masked_msl.find("gl_ViewportIndex") == std::string::npos && count(masked_msl, "if ((spvEmulatedReversedDepthViewportMask & 1u) != 0u)") == 1, "Masked ViewportIndex must use viewport zero.");
		write(directory, "reversed-masked-replay", masked_msl);
		CompilerMSL default_replay(simple);
		configure(default_replay);
		auto default_msl = default_replay.compile_captured_output_replay(simple_layout, binding);
		for (uint32_t slot : {0u, 1u, 2u, 31u, ~0u})
			for (bool enabled : {false, true})
			{
				CompilerMSL compiler(simple);
				configure(compiler);
				auto options = compiler.get_msl_options();
				options.emulate_reversed_depth_viewport = enabled;
				options.reversed_depth_viewport_buffer_index = slot;
				compiler.set_msl_options(options);
				if (enabled)
					rejects([&]() { compiler.compile_captured_output_replay(simple_layout, binding); }, "reversed-depth viewport mask buffer index");
				else
					check(compiler.compile_captured_output_replay(simple_layout, binding) == default_msl, "Disabled reversed-depth option must ignore the mask slot and preserve replay ABI/source.");
			}
		auto reject_layout = [&](MSLCapturedVertexLayout layout, const char *diagnostic) {
			CompilerMSL compiler(simple);
			configure(compiler);
			rejects([&]() { compiler.compile_captured_output_replay(layout, binding); }, diagnostic);
		};
		auto bad = simple_layout;
		bad.components.erase(bad.components.begin());
		reject_layout(bad, "missing Location 0 Component 0");
		bad = simple_layout;
		bad.builtins.clear();
		reject_layout(bad, "missing builtin");
		bad = simple_layout;
		bad.components[0].scalar_type = SPIRType::UInt;
		reject_layout(bad, "physical scalar type mismatch");
		bad = simple_layout;
		bad.components[0].byte_offset = 16;
		reject_layout(bad, "overlapping scalar offsets");
		bad = simple_layout;
		bad.builtins.push_back(bad.builtins[0]);
		reject_layout(bad, "duplicate builtin");
		bad = simple_layout;
		bad.components.push_back(bad.components[0]);
		reject_layout(bad, "duplicate Location/Component");
		bad = simple_layout;
		bad.components[0].byte_offset = 2;
		reject_layout(bad, "naturally aligned");
		bad = simple_layout;
		bad.stride = 16;
		reject_layout(bad, "outside the captured stride");
		bad = simple_layout;
		bad.components[0].scalar_type = SPIRType::UInt64;
		reject_layout(bad, "16/32-bit");
		bad = simple_layout;
		bad.builtins[0].builtin = spv::BuiltInCullDistance;
		reject_layout(bad, "does not support builtin");
		bad = simple_layout;
		bad.builtins[0].array_index = 1;
		reject_layout(bad, "unsupported type or shape");
		for (uint32_t mode = 0; mode < 9; mode++)
		{
			CompilerMSL compiler(simple);
			configure(compiler);
			auto options = compiler.get_msl_options();
			auto binds = binding;
			const char *diagnostic;
			switch (mode)
			{
			case 0: binds.occurrence_buffer_index = binds.vertex_buffer_index; diagnostic = "three distinct"; break;
			case 1: binds.draw_parameters_buffer_index = 31; diagnostic = "three distinct"; break;
			case 2: binds.primitive_index_location = 0; diagnostic = "private Location collides"; break;
			case 3: options.capture_output_to_buffer = true; diagnostic = "normal rasterizing"; break;
			case 4: options.vertex_for_tessellation = true; diagnostic = "normal rasterizing"; break;
			case 5: options.multiview = true; diagnostic = "does not support multiview"; break;
			case 6: options.emulate_depth_clip_enable = true; diagnostic = "depth-clip emulation"; break;
			case 7: options.enable_point_size_default = true; diagnostic = "default PointSize"; break;
			default: options.set_msl_version(2, 3); diagnostic = "MSL 2.4"; break;
			}
			compiler.set_msl_options(options);
			rejects([&]() { compiler.compile_captured_output_replay(simple_layout, binds); }, diagnostic);
			rejects([&]() { compiler.compile_captured_output_replay(simple_layout, binding); }, "fresh compiler");
		}
		CompilerMSL reused(simple);
		configure(reused);
		reused.compile();
		rejects([&]() { reused.compile_captured_output_replay(simple_layout, binding); }, "fresh compiler");
		CompilerMSL replayed(simple);
		configure(replayed);
		replayed.compile_captured_output_replay(simple_layout, binding);
		rejects([&]() { replayed.compile(); }, "fresh compiler");
		rejects([&]() { replayed.get_msl_captured_vertex_layout(); }, "successful MSL compilation");
		CompilerMSL no_position(simple);
		configure(no_position);
		no_position.mask_stage_output_by_builtin(spv::BuiltInPosition);
		rejects([&]() { no_position.compile_captured_output_replay(simple_layout, binding); }, "Position output");
		CompilerMSL tese(tessellation);
		configure(tese);
		auto tese_replay = tese.compile_captured_output_replay(simple_layout, binding);
		check(tese_replay.find("vertex ") != std::string::npos && tese_replay.find("[[ patch(") == std::string::npos, "TES replay did not emit a raster vertex function");
		CompilerMSL matrix(unsupported);
		configure(matrix);
		rejects([&]() { matrix.compile_captured_output_replay(simple_layout, binding); }, "physical scalar type mismatch");
		std::cout << "Captured output replay checks passed.\n";
		return 0;
	}
	catch (const std::exception &error)
	{
		std::cerr << error.what() << '\n';
		return 1;
	}
}
