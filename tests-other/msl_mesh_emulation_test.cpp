#include "spirv_msl.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

using namespace spirv_cross;

static std::vector<uint32_t> read_file(const char *path)
{
	FILE *file = fopen(path, "rb");
	if (!file)
		return {};

	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	rewind(file);
	std::vector<uint32_t> buffer(length / sizeof(uint32_t));
	if (fread(buffer.data(), 1, length, file) != size_t(length))
		buffer.clear();
	fclose(file);
	return buffer;
}

int main(int argc, char **argv)
{
	if (argc != 4)
		return EXIT_FAILURE;

	auto spirv = read_file(argv[1]);
	auto compute_spirv = read_file(argv[2]);
	auto fragment_spirv = read_file(argv[3]);
	if (spirv.empty() || compute_spirv.empty() || fragment_spirv.empty())
		return EXIT_FAILURE;

	CompilerMSL mesh(spirv);
	auto options = mesh.get_msl_options();
	options.msl_version = CompilerMSL::Options::make_msl_version(3, 0);
	options.mesh_shader_emulation = true;
	mesh.set_msl_options(options);

	MSLMeshOutputSpillKey key;
	key.location = 0;
	key.component = 0;
	mesh.add_msl_mesh_output_spill(key);
	if (mesh.compile().empty() || !mesh.get_mesh_output_buffer_size() || !mesh.get_mesh_output_buffer_alignment())
		return EXIT_FAILURE;
	if (!mesh.needs_dispatch_base_buffer())
	{
		fprintf(stderr, "Mesh emulation did not request its dispatch buffer.\n");
		return EXIT_FAILURE;
	}

	CompilerMSL native_mesh(spirv);
	options.mesh_shader_emulation = false;
	options.dispatch_base = true;
	native_mesh.set_msl_options(options);
	if (!native_mesh.needs_dispatch_base_buffer())
		return EXIT_FAILURE;

	CompilerMSL compute(compute_spirv);
	compute.set_msl_options(options);
	if (compute.needs_dispatch_base_buffer())
		return EXIT_FAILURE;

	const auto &layout = mesh.get_msl_mesh_output_spill_layout();
	const auto &fields = mesh.get_msl_mesh_output_spill_fields();
	if (layout.version != 2 || !layout.perspective_basis_components || fields.empty())
		return EXIT_FAILURE;

	auto fragment_fields = fields;
	fragment_fields[0].vecsize = 3;
	fragment_fields[0].capture_word_stride = 3;
	CompilerMSL replay(fragment_spirv);
	replay.set_msl_options(options);
	replay.set_msl_mesh_output_spill_layout(layout, fragment_fields);
	auto replay_source = replay.compile();
	if (replay_source.find("spvMeshSpillInterpolateTriangle") == std::string::npos ||
	    replay_source.find(".interpolate_at_center()") == std::string::npos)
		return EXIT_FAILURE;
	const auto &replay_fields = replay.get_msl_mesh_output_spill_fields();
	if (!(replay.get_msl_mesh_output_spill_layout() == layout) || replay_fields.size() != fragment_fields.size())
		return EXIT_FAILURE;
	for (uint32_t i = 0; i < fragment_fields.size(); i++)
		if (!(replay_fields[i] == fragment_fields[i]))
			return EXIT_FAILURE;

#ifndef SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS
	auto rejects_layout = [&](const MSLMeshOutputSpillLayout &invalid_layout,
	                          const SmallVector<MSLMeshOutputSpillField> &invalid_fields)
	{
		CompilerMSL compiler(spirv);
		try
		{
			compiler.set_msl_mesh_output_spill_layout(invalid_layout, invalid_fields);
			return false;
		}
		catch (const CompilerError &)
		{
			return true;
		}
	};
	auto invalid_layout = layout;
	for (uint32_t version : { 1u, 3u })
	{
		invalid_layout.version = version;
		if (!rejects_layout(invalid_layout, fields))
			return EXIT_FAILURE;
	}
	invalid_layout = layout;
	auto invalid_fields = fields;
	invalid_fields[0].capture_word_offset = layout.primitive_index_word_offset;
	if (!rejects_layout(invalid_layout, invalid_fields))
		return EXIT_FAILURE;
	invalid_fields[0] = fields[0];
	invalid_fields[0].capture_word_offset = layout.capture_record_stride / 4u;
	if (!rejects_layout(invalid_layout, invalid_fields))
		return EXIT_FAILURE;

	CompilerMSL mismatch(fragment_spirv);
	mismatch.set_msl_options(options);
	mismatch.set_msl_mesh_output_spill_layout(layout, fields);
	try
	{
		mismatch.compile();
		return EXIT_FAILURE;
	}
	catch (const CompilerError &)
	{
	}
#endif

	return EXIT_SUCCESS;
}
