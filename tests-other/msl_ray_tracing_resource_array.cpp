/*
 * Copyright 2026 dttdrv
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <spirv_cross_c.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

static void check(spvc_result result)
{
	if (result != SPVC_SUCCESS)
	{
		fprintf(stderr, "SPIRV-Cross error %d\n", int(result));
		exit(EXIT_FAILURE);
	}
}

static std::vector<SpvId> read_file(const char *path)
{
	FILE *file = fopen(path, "rb");
	if (!file)
		return {};
	if (fseek(file, 0, SEEK_END) != 0)
	{
		fclose(file);
		return {};
	}
	long size = ftell(file);
	if (size < 0 || size % long(sizeof(SpvId)))
	{
		fclose(file);
		return {};
	}
	rewind(file);
	std::vector<SpvId> spirv(size / long(sizeof(SpvId)));
	if (fread(spirv.data(), 1, size, file) != size_t(size))
		spirv.clear();
	fclose(file);
	return spirv;
}

static std::string compile(const std::vector<SpvId> &spirv, bool device, bool discrete, unsigned count,
                           bool decoration = false, bool overlap = false, const char *expected_error = nullptr,
                           unsigned msl_version = SPVC_MAKE_MSL_VERSION(3, 0, 0), bool force_native_arrays = false,
                           const char *entry = nullptr, bool address_table = false)
{
	spvc_context context;
	spvc_parsed_ir ir;
	spvc_compiler compiler;
	spvc_compiler_options options;
	check(spvc_context_create(&context));
	check(spvc_context_parse_spirv(context, spirv.data(), spirv.size(), &ir));
	check(spvc_context_create_compiler(context, SPVC_BACKEND_MSL, ir, SPVC_CAPTURE_MODE_COPY, &compiler));
	if (entry)
		check(spvc_compiler_set_entry_point(compiler, entry, SpvExecutionModelGLCompute));
	check(spvc_compiler_create_compiler_options(compiler, &options));
	check(spvc_compiler_options_set_uint(options, SPVC_COMPILER_OPTION_MSL_VERSION, msl_version));
	check(spvc_compiler_options_set_uint(
	    options, SPVC_COMPILER_OPTION_MSL_RAY_TRACING_ACCELERATION_STRUCTURE_ADDRESS_TABLE_BUFFER_INDEX, 11));
	check(spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_MSL_ARGUMENT_BUFFERS, SPVC_TRUE));
	check(spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_MSL_FORCE_ACTIVE_ARGUMENT_BUFFER_RESOURCES,
	                                     SPVC_TRUE));
	check(spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_MSL_ENABLE_RAY_TRACING_PIPELINE_EMULATION,
	                                     SPVC_FALSE));
	check(spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_MSL_FORCE_NATIVE_ARRAYS,
	                                     force_native_arrays ? SPVC_TRUE : SPVC_FALSE));
	if (decoration)
		check(spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_MSL_ENABLE_DECORATION_BINDING, SPVC_TRUE));
	if (!count)
		check(spvc_compiler_options_set_uint(options, SPVC_COMPILER_OPTION_MSL_ARGUMENT_BUFFERS_TIER, 2));
	check(spvc_compiler_install_compiler_options(compiler, options));
	spvc_msl_resource_binding_2 binding;
	spvc_msl_resource_binding_init_2(&binding);
	binding.stage = SpvExecutionModelGLCompute;
	binding.desc_set = 0;
	binding.binding = 0;
	binding.count = count;
	binding.msl_buffer = 0;
	unsigned metadata_binding = count > 3 ? count : 3;
	binding.msl_texture = overlap ? 1 : metadata_binding;
	if (count)
		check(spvc_compiler_msl_add_resource_binding_2(compiler, &binding));
	if (device)
		check(spvc_compiler_msl_set_argument_buffer_device_address_space(compiler, 0, SPVC_TRUE));
	if (discrete)
		check(spvc_compiler_msl_add_discrete_descriptor_set(compiler, 0));
	const char *source;
	spvc_result compile_result = spvc_compiler_compile(compiler, &source);
	if (expected_error)
	{
		const char *error = spvc_context_get_last_error_string(context);
		if (compile_result == SPVC_SUCCESS || !error || !strstr(error, expected_error))
		{
			fprintf(stderr, "Expected error containing '%s', got '%s'\n%s\n", expected_error, error ? error : "",
			        compile_result == SPVC_SUCCESS ? source : "");
			exit(EXIT_FAILURE);
		}
		spvc_context_destroy(context);
		return {};
	}
	check(compile_result);
	if (spvc_compiler_msl_needs_acceleration_structure_address_table(compiler) !=
	    (address_table ? SPVC_TRUE : SPVC_FALSE))
	{
		fprintf(stderr, "Unexpected acceleration-structure address-table requirement\n");
		exit(EXIT_FAILURE);
	}
	spvc_resources resources;
	const spvc_reflected_resource *scenes;
	size_t scene_count;
	check(spvc_compiler_create_shader_resources(compiler, &resources));
	check(spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_ACCELERATION_STRUCTURE, &scenes,
	                                                &scene_count));
	unsigned primary_binding =
	    scene_count ? spvc_compiler_msl_get_automatic_resource_binding(compiler, scenes[0].id) : ~0u;
	unsigned secondary_binding =
	    scene_count ? spvc_compiler_msl_get_automatic_resource_binding_secondary(compiler, scenes[0].id) : ~0u;
	if (count && (scene_count != 1 || primary_binding != 0 || secondary_binding != metadata_binding))
	{
		fprintf(stderr,
		        "Unexpected reflected acceleration-structure bindings: count %u, scenes %zu, primary %u, "
		        "secondary %u\n",
		        count, scene_count, primary_binding, secondary_binding);
		exit(EXIT_FAILURE);
	}
	std::string result(source);
	spvc_context_destroy(context);
	return result;
}

static bool contains(const std::string &source, const char *text)
{
	return source.find(text) != std::string::npos;
}

int main(int argc, char **argv)
{
	if (argc != 15)
		return EXIT_FAILURE;
	auto spirv = read_file(argv[1]);
	auto multiway_phi = read_file(argv[2]);
	auto array_return = read_file(argv[3]);
	auto fixed_array = read_file(argv[4]);
	auto multidimensional_array = read_file(argv[5]);
	auto loop_phi = read_file(argv[6]);
	auto array_loop_phi = read_file(argv[7]);
	auto phi_copy_swap = read_file(argv[8]);
	auto phi_extract_snapshot = read_file(argv[9]);
	auto composite_insert = read_file(argv[10]);
	auto copy_logical = read_file(argv[11]);
	auto native_array_return_noargs = read_file(argv[12]);
	auto multi_entry = read_file(argv[13]);
	auto multidimensional_resource = read_file(argv[14]);
	if (spirv.empty() || multiway_phi.empty() || array_return.empty() || fixed_array.empty() ||
	    multidimensional_array.empty() || loop_phi.empty() || array_loop_phi.empty() || phi_copy_swap.empty() ||
	    phi_extract_snapshot.empty() || composite_insert.empty() || copy_logical.empty() ||
	    native_array_return_noargs.empty() || multi_entry.empty() || multidimensional_resource.empty())
		return EXIT_FAILURE;
	auto constant = compile(spirv, false, false, 2, false, false, nullptr, SPVC_MAKE_MSL_VERSION(2, 4, 0));
	auto device = compile(spirv, true, false, 2);
	auto discrete = compile(spirv, false, true, 2);
	auto discrete_one = compile(spirv, false, true, 1);
	auto large = compile(spirv, false, false, 1024);
	compile(spirv, false, false, 2, false, false, "Ray queries require MSL 2.4 or later",
	        SPVC_MAKE_MSL_VERSION(2, 3, 0));
	compile(spirv, true, false, 0, false, false, "Runtime acceleration structure array size");
	compile(spirv, false, false, 2, false, true, "Acceleration structure and metadata buffer bindings overlap");
	compile(fixed_array, false, false, 0, true, false,
	        "Acceleration structure metadata requires an explicit MSL resource binding");
	auto multiway = compile(multiway_phi, false, false, 0);
	auto returned_array = compile(array_return, false, false, 0);
	auto multidimensional = compile(multidimensional_array, false, false, 0);
	auto loop = compile(loop_phi, false, false, 0);
	auto array_loop = compile(array_loop_phi, false, false, 0);
	auto phi_swap = compile(phi_copy_swap, false, false, 0);
	auto phi_extract = compile(phi_extract_snapshot, false, false, 0);
	auto inserted = compile(composite_insert, false, false, 0);
	auto copied = compile(copy_logical, false, false, 0);
	auto native_return = compile(native_array_return_noargs, false, false, 0, false, false, nullptr,
	                             SPVC_MAKE_MSL_VERSION(3, 1, 0), true, nullptr, true);
	auto plain_entry =
	    compile(multi_entry, false, false, 0, false, false, nullptr, SPVC_MAKE_MSL_VERSION(2, 3, 0), false, "plain");
	compile(multi_entry, false, false, 0, false, false, "Ray queries require MSL 2.4 or later",
	        SPVC_MAKE_MSL_VERSION(2, 3, 0), false, "ray");
	const char *unsupported_ray_queries[][2] = {
		{ "triangle_positions", "OpRayQueryGetIntersectionTriangleVertexPositionsKHR" },
		{ "cluster_id", "OpRayQueryGetClusterIdNV" },
		{ "sphere_position", "OpRayQueryGetIntersectionSpherePositionNV" },
		{ "sphere_radius", "OpRayQueryGetIntersectionSphereRadiusNV" },
		{ "lss_positions", "OpRayQueryGetIntersectionLSSPositionsNV" },
		{ "lss_radii", "OpRayQueryGetIntersectionLSSRadiiNV" },
		{ "lss_hit_value", "OpRayQueryGetIntersectionLSSHitValueNV" },
		{ "is_sphere_hit", "OpRayQueryIsSphereHitNV" },
		{ "is_lss_hit", "OpRayQueryIsLSSHitNV" },
	};
	for (auto &test : unsupported_ray_queries)
		compile(multi_entry, false, false, 0, false, false, test[1], SPVC_MAKE_MSL_VERSION(3, 1, 0), false,
		        test[0]);
	auto bound_multidimensional = compile(multidimensional_resource, false, false, 4);
	compile(multidimensional_resource, false, false, 3, false, false,
	        "MSL resource binding count does not match the declared multidimensional array size");
	const char *scene_array = "spvUnsafeArray<raytracing::acceleration_structure<raytracing::instancing>, 2>";
	if (!contains(constant, "tlasArray [[id(0)]][2]") || !contains(constant, "template<typename spvRayArray") ||
	    !contains(device, "template<typename spvRayArray") || !contains(constant, "typename spvRayMetadataArray") ||
	    !contains(device, "typename spvRayMetadataArray") ||
	    !contains(constant, "device uint2* tlasArray_spvRayMetadata_") ||
	    !contains(device, "device uint2* tlasArray_spvRayMetadata_") || contains(constant, scene_array) ||
	    contains(device, scene_array) || !contains(discrete, scene_array) ||
	    !contains(discrete, "spvUnsafeArray<device const uint2*, 2> tlasArray_spvRayMetadata = {") ||
	    !contains(discrete, "tlasArray.elements, tlasArray_spvRayMetadata.elements") ||
	    !contains(discrete_one, "spvUnsafeArray<device const uint2*, 1> tlasArray_spvRayMetadata = {") ||
	    !contains(discrete_one, "tlasArray_spvRayMetadata_0 [[buffer(") ||
	    !contains(large, "tlasArray [[id(0)]][1024]") || contains(large, ")[1023]") ||
	    large.size() > constant.size() + 64 ||
	    contains(constant, "spvUnsafeArray<raytracing::acceleration_structure<raytracing::instancing>, 1>") ||
	    contains(device, "spvUnsafeArray<raytracing::acceleration_structure<raytracing::instancing>, 1>") ||
	    contains(discrete, "spvUnsafeArray<raytracing::acceleration_structure<raytracing::instancing>, 1>") ||
	    contains(constant, "spvUnsafeArray<ulong") || contains(device, "spvUnsafeArray<ulong"))
	{
		fprintf(stderr, "Unexpected acceleration-structure array output\n");
		return EXIT_FAILURE;
	}
	if (!contains(plain_entry, "kernel void plain(") || !contains(bound_multidimensional, "scenes [[id(0)]][2][2]"))
	{
		fprintf(stderr, "Unexpected multi-entry or multidimensional output\n");
		return EXIT_FAILURE;
	}
	if (!contains(multiway, "spvRayMetadataPhi_"))
	{
		fprintf(stderr, "Missing multiway-phi metadata propagation\n");
		return EXIT_FAILURE;
	}
	if (!contains(returned_array, "spvUnsafeArray<device const uint2*, 2> spvRayMetadata_") ||
	    !contains(returned_array, "spvRayMetadataReturn[spvRayMetadataIndex_Return_"))
	{
		fprintf(stderr, "Missing array-return metadata propagation\n");
		return EXIT_FAILURE;
	}
	if (!contains(multidimensional, "spvUnsafeArray<spvUnsafeArray<device const uint2*, 2>, 2>") ||
	    !contains(multidimensional, "[1][0]"))
	{
		fprintf(stderr, "Missing multidimensional-array metadata propagation\n");
		return EXIT_FAILURE;
	}
	if (!contains(loop, "spvRayMetadataPhi_"))
	{
		fprintf(stderr, "Missing loop-phi metadata propagation\n");
		return EXIT_FAILURE;
	}
	if (!contains(array_loop, "for (;;)") || !contains(array_loop, "spvRayMetadataIndex_Phi_") ||
	    !contains(array_loop, "= spvRayMetadata_") || contains(array_loop, ", for (") || contains(array_loop, "{,") ||
	    !contains(phi_swap, "_copy = spvRayMetadataPhi_") ||
	    !contains(phi_swap, " = spvRayMetadataPhi_") ||
	    !contains(phi_extract, "= spvRayMetadata_") ||
	    !contains(phi_extract, "_copy = spvRayMetadataPhi_") || !contains(phi_extract, "[0];") ||
	    !contains(phi_extract, " = spvRayMetadataPhi_") || !contains(phi_extract, "_copy;") ||
	    contains(phi_extract, ", for (") || contains(phi_extract, "{,"))
	{
		fprintf(stderr, "Invalid phi metadata propagation\n");
		return EXIT_FAILURE;
	}
	if (!contains(inserted, "spvRayMetadataIndex_CompositeInsertBase_") ||
	    !contains(inserted, "[1] = spvRayMetadata_") ||
	    !contains(copied, "spvRayMetadataIndex_CopyLogical_"))
	{
		fprintf(stderr, "Missing composite metadata propagation\n");
		return EXIT_FAILURE;
	}
	if (!contains(native_return,
	              "(&spvReturnValue)[2], device const ulong2* spvAccelerationStructureAddressTable, thread "
	              "spvUnsafeArray<device const uint2*, 2>& spvRayMetadataReturn)") ||
	    !contains(native_return, "device const ulong2* spvAccelerationStructureAddressTable [[buffer(11)]]") ||
	    !contains(native_return, ", spvAccelerationStructureAddressTable, spvRayMetadata_") ||
	    !contains(native_return, "spvArrayCopyFromStackToStack(spvReturnValue") ||
	    !contains(native_return, "spvRayMetadataIndex_Return_"))
	{
		fprintf(stderr, "Invalid native-array return metadata propagation\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
