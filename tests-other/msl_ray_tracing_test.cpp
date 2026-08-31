#include "spirv_msl.hpp"
#include <cstring>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <vector>

using namespace spirv_cross;

static std::vector<uint32_t> read_spirv(const char *path)
{
	std::ifstream file(path, std::ios::binary);
	std::vector<char> bytes((std::istreambuf_iterator<char>(file)), {});
	if (!file || bytes.size() % sizeof(uint32_t))
		return {};
	std::vector<uint32_t> spirv(bytes.size() / sizeof(uint32_t));
	std::memcpy(spirv.data(), bytes.data(), bytes.size());
	return spirv;
}

static std::string compile(const char *path, const char *entry = nullptr, bool native_arrays = false,
	                       uint32_t msl_major = 4, uint32_t msl_minor = 0,
	                       bool force_ifb = false, bool procedural_ifb = false,
	                       const char *runtime_abi = "// Test runtime ABI.", bool constexpr_sampler = false,
	                       uint32_t acceleration_structure_count = 0, bool argument_buffers = true,
	                       bool device_argument_buffer = true)
{
	CompilerMSL compiler(read_spirv(path));
	if (entry)
	{
		for (const auto &candidate : compiler.get_entry_points_and_stages())
			if (candidate.name == entry)
				compiler.set_entry_point(candidate.name, candidate.execution_model);
	}
	auto options = compiler.get_msl_options();
	options.msl_version = CompilerMSL::Options::make_msl_version(msl_major, msl_minor);
	options.argument_buffers = argument_buffers;
	options.acceleration_structure_descriptor_as_address = true;
	options.ray_tracing_pipeline = runtime_abi;
	options.ray_tracing_max_hit_attribute_size = 32;
	options.ray_tracing_any_hit_ifb = !procedural_ifb &&
	                                  (force_ifb || compiler.get_execution_model() == spv::ExecutionModelAnyHitKHR);
	options.ray_tracing_intersection_ifb = procedural_ifb;
	options.force_native_arrays = native_arrays;
	compiler.set_msl_options(options);
	if (acceleration_structure_count)
	{
		MSLResourceBinding binding;
		binding.stage = spv::ExecutionModelGLCompute;
		binding.basetype = SPIRType::AccelerationStructure;
		binding.desc_set = 3;
		binding.binding = 5;
		binding.count = acceleration_structure_count;
		compiler.add_msl_resource_binding(binding);
	}
	if (constexpr_sampler)
		compiler.remap_constexpr_sampler_by_binding(8, 0, MSLConstexprSampler{});
	auto model = compiler.get_execution_model();
	if (model >= spv::ExecutionModelRayGenerationKHR && model <= spv::ExecutionModelCallableKHR)
		compiler.add_header_line(runtime_abi ? runtime_abi : "");
	if (device_argument_buffer)
		compiler.set_argument_buffer_device_address_space(3, true);
	if (device_argument_buffer)
		compiler.set_argument_buffer_device_address_space(0, true);
	compiler.set_argument_buffer_device_address_space(8, true);
	return compiler.compile();
}

static bool check(const std::string &source, const char *test, std::initializer_list<const char *> required,
                  std::initializer_list<const char *> forbidden = {})
{
	for (auto text : required)
		if (source.find(text) == std::string::npos)
		{
			std::cerr << test << ": missing " << text << '\n';
			return false;
		}
	for (auto text : forbidden)
		if (source.find(text) != std::string::npos)
		{
			std::cerr << test << ": contains " << text << '\n';
			return false;
		}
	return true;
}

int main(int argc, char **argv)
{
	if (argc != 25)
		return 1;
	try
	{
		compile(argv[1], nullptr, false, 4, 0, false, false, nullptr);
		std::cerr << argv[1] << ": missing runtime ABI was accepted\n";
		return 1;
	}
	catch (const CompilerError &)
	{
	}
	const char *expected[] = {
		"[[visible]]", "spvRay.context.WorldRayOriginKHR", "if (spvRayAction != 0)",
		"spvReportIntersection(", "SPV_RAY_INCOMING_DATA", "spvExecuteCallable("
	};
	for (int i = 0; i < 6; i++)
	{
		try
		{
			auto source = compile(argv[i + 1]);
			if (!check(source, argv[i + 1], { expected[i], "Test runtime ABI" }) ||
			    (i == 2 && !check(source, argv[i + 1], { "SPV_RAY_IFB_ENTRY_POINT(" })))
				return 1;
		}
		catch (const std::exception &error)
		{
			std::cerr << argv[i + 1] << ": " << error.what() << '\n';
			return 1;
		}
	}
	try
	{
		auto multi14a = compile(argv[7], "a");
		auto multi14b = compile(argv[7], "b");
		if (!check(multi14a, argv[7], { "SPV_RAY_CONTEXT_ARGS(float)" }) ||
		    !check(multi14b, argv[7], { "SPV_RAY_CONTEXT_ARGS(float4)" }))
			return 1;

		auto native_array = compile(argv[8], nullptr, true);
		if (!check(native_array, argv[8], { "spvRuntimeBuffer<3>(spvRayState)" },
		           { "thread & reinterpret_cast" }))
			return 1;

		auto ray_query = compile(argv[9], nullptr, false, 4, 0, false, false,
		                         "// Test runtime ABI.", false, 2);
		if (!check(ray_query, argv[9],
		           { "spvRayQueryMetadata", "template<typename spvRTASArray", "getScene(",
		             " = getScene(", "trace(spvDescriptorSet3.scene" },
		           { "struct spvRayQuery {", "const device const", "thread device",
		             "spvUnsafeArray<device const ulong*, 1>" }))
			return 1;
		auto ray_query_compat = compile(argv[9], nullptr, false, 3, 0, false, false,
		                                "// Test runtime ABI.", false, 2);
		if (!check(ray_query_compat, argv[9], { "template<typename spvRTASArray", "trace(spvDescriptorSet3.scene" },
		           { "spvUnsafeArray<device const ulong*, 1>" }))
			return 1;
		auto ray_query_constant = compile(argv[9], nullptr, false, 4, 0, false, false,
		                                  "// Test runtime ABI.", false, 2, true, false);
		if (!check(ray_query_constant, argv[9], { "template<typename spvRTASArray", "trace(spvDescriptorSet3.scene" },
		           { "spvUnsafeArray<device const ulong*, 1>" }))
			return 1;
		auto ray_query_discrete = compile(argv[9], nullptr, false, 4, 0, false, false,
		                                  "// Test runtime ABI.", false, 0, false);
		if (!check(ray_query_discrete, argv[9], { "template<typename spvRTASArray", "trace(scene" },
		           { "spvUnsafeArray<device const ulong*, 1>" }))
			return 1;

		auto push_constant = compile(argv[10]);
		if (!check(push_constant, argv[10], { "spvPushConstant<Registers>(spvRayState)" },
		           { "constant Registers& (*reinterpret_cast" }))
			return 1;

		auto set_8 = compile(argv[11]);
		if (!check(set_8, argv[11], { "[[buffer(8)]]" }))
			return 1;

		auto ray_set_8 = compile(argv[12]);
		if (!check(ray_set_8, argv[12], { "spvRayState.dispatch->descriptorSetAddresses[8]" }))
			return 1;
		auto hit_attribute = compile(argv[13]);
		if (!check(hit_attribute, argv[13], { "packed_float3 a", "packed_float3 b" }))
			return 1;

		auto procedural_ifb = compile(argv[3], nullptr, false, 4, 0, false, true);
		if (!check(procedural_ifb, argv[3],
		           { "SPV_RAY_IFB_ENTRY_POINT(" }))
			return 1;

		auto ray_query_array = compile(argv[18]);
		if (!check(ray_query_array, argv[18],
		           { "spvRayQueryMetadata queriesMetadata[2][3]",
		             "thread spvRayQueryMetadata (&queriesMetadata)[2][3]", "queriesMetadata[i][j]" },
		           { "queries[i][j]Metadata", "thread spvRayQueryMetadata& queriesMetadata" }))
			return 1;

		auto unused_position_fetch = compile(argv[19]);
		if (!check(unused_position_fetch, argv[19], { "[[visible]]" }))
			return 1;

		try
		{
			compile(argv[20]);
			std::cerr << argv[20] << ": active position fetch was accepted\n";
			return 1;
		}
		catch (const CompilerError &error)
		{
			if (std::string(error.what()).find("position fetch is not supported") == std::string::npos)
				throw;
		}

		auto constexpr_source = compile(argv[21], nullptr, false, 4, 0, false, false,
		                                "// Test runtime ABI.", true);
		if (!check(constexpr_source, argv[21], { "constexpr sampler" }, { "descriptorSetAddresses[8]" }))
			return 1;

		auto subgroup_helper = compile(argv[22]);
		if (!check(subgroup_helper, argv[22],
		           { "simd_ballot", "spvRayState.SubgroupSize", "spvRayState.SubgroupLocalInvocationId" },
		           { "spvRay.context.Subgroup" }))
			return 1;

		auto branch_query = compile(argv[23]);
		if (!check(branch_query, argv[23],
		           { "\n    raytracing::intersection_query<raytracing::instancing, raytracing::triangle_data> query;" },
		           { "\n        raytracing::intersection_query<raytracing::instancing, raytracing::triangle_data> query;" }))
			return 1;

		auto ordinary = compile(argv[24]);
		if (!check(ordinary, argv[24], {}, { "spvRayState", "Test runtime ABI" }))
		{
			std::cerr << argv[24] << ": ray profile changed a non-ray shader\n";
			return 1;
		}

		try
		{
			compile(argv[14]);
			std::cerr << argv[14] << ": oversized Boolean hit attribute was accepted\n";
			return 1;
		}
		catch (const CompilerError &error)
		{
			if (std::string(error.what()).find("runtime ABI limit") == std::string::npos)
				throw;
		}

		auto vector_array = compile(argv[15]);
		if (!check(vector_array, argv[15],
		           { "packed_float3 samples[2]", "[0] = float3(spvHitAttribute<HitData>(spvRay.context)",
		             "[1] = float3(spvHitAttribute<HitData>(spvRay.context)" },
		           { "struct HitData_", "packed_spvUnsafeArray" }))
			return 1;

		auto unused_payload = compile(argv[16]);
		if (!check(unused_payload, argv[16], { "SPV_RAY_CONTEXT_ARGS(Payload)" }, { "SPV_RAY_CONTEXT_ARGS(uint)" }))
			return 1;
		auto shared_type = compile(argv[17]);
		if (!check(shared_type, argv[17],
		           { "struct spvRayData_", "packed_float3 _m0", "packed_rm_float3x2 _m2",
		             "._m0 = (*spvDescriptorSet0" },
		           { "_2 = *spvDescriptorSet0", "spvRayData._m3[0].data" }))
			return 1;

		try { compile(argv[1], nullptr, false, 2, 4); }
		catch (const CompilerError &error) {
			if (std::string(error.what()).find("require MSL 3.0") != std::string::npos) return 0;
		}
		std::cerr << argv[1] << ": MSL 2.4 ray profile was not rejected\n";
	}
	catch (const std::exception &error)
	{
		std::cerr << error.what() << '\n';
		return 1;
	}
	return 1;
}
