#include "spirv_msl.hpp"
#include <cstring>
#include <fstream>
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
                           bool enable_ray_tracing = true, uint32_t msl_major = 4, uint32_t msl_minor = 0,
                           bool force_ifb = false)
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
	options.argument_buffers = true;
	options.acceleration_structure_descriptor_as_address = true;
	options.enable_ray_tracing_pipeline = enable_ray_tracing;
	options.ray_tracing_any_hit_ifb = force_ifb || compiler.get_execution_model() == spv::ExecutionModelAnyHitKHR;
	options.force_native_arrays = native_arrays;
	compiler.set_msl_options(options);
	auto model = compiler.get_execution_model();
	if (model >= spv::ExecutionModelRayGenerationKHR && model <= spv::ExecutionModelCallableKHR)
		compiler.add_header_line("// Test runtime ABI.");
	compiler.set_argument_buffer_device_address_space(0, true);
	compiler.set_argument_buffer_device_address_space(8, true);
	return compiler.compile();
}

static bool require(const std::string &source, const char *text, const char *test)
{
	if (source.find(text) != std::string::npos)
		return true;
	std::cerr << test << ": missing " << text << '\n';
	return false;
}

static bool forbid(const std::string &source, const char *text, const char *test)
{
	if (source.find(text) == std::string::npos)
		return true;
	std::cerr << test << ": contains " << text << '\n';
	return false;
}

int main(int argc, char **argv)
{
	if (argc != 22)
		return 1;
	const char *expected[] = {
		"[[visible]]", "spvRayContext.worldRayOrigin", "if (spvRayAction != 0)",
		"spvReportIntersection(", "spvRayData", "spvExecuteCallable("
	};
	for (int i = 0; i < 6; i++)
	{
		try
		{
			auto source = compile(argv[i + 1]);
			if (!require(source, expected[i], argv[i + 1]) ||
			    !require(source, "Test runtime ABI", argv[i + 1]) ||
			    (i == 2 && (!require(source, "spvRayTmax [[distance]]", argv[i + 1]) ||
			                !forbid(source, "spvRayTmax [[max_distance]]", argv[i + 1]))))
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
		auto multi13 = compile(argv[7], "a");
		auto multi14a = compile(argv[8], "a");
		auto multi14b = compile(argv[8], "b");
		if (!require(multi13, "spvIFBPayload<float>", argv[7]) ||
		    !forbid(multi13, "spvIFBPayload<float4>", argv[7]) ||
		    !require(multi14a, "spvIFBPayload<float>", argv[8]) ||
		    !require(multi14b, "spvIFBPayload<float4>", argv[8]))
			return 1;

		auto native_array = compile(argv[9], nullptr, true);
		if (!require(native_array, "accelerationStructureAddressTableAddress", argv[9]) ||
		    !forbid(native_array, "thread & reinterpret_cast", argv[9]))
			return 1;

		auto ray_query = compile(argv[10]);
		if (!require(ray_query, "spvRayQueryMetadata", argv[10]) ||
		    !forbid(ray_query, "struct spvRayQuery {", argv[10]) ||
		    !forbid(ray_query, "const device const", argv[10]) ||
		    !forbid(ray_query, "thread device", argv[10]))
			return 1;

		auto push_constant = compile(argv[11]);
		if (!require(push_constant, "pushConstantsAddress", argv[11]) ||
		    !forbid(push_constant, "constant Registers& (*reinterpret_cast", argv[11]))
			return 1;

		auto set_8 = compile(argv[12], nullptr, false, false);
		if (!require(set_8, "[[buffer(8)]]", argv[12]))
			return 1;

		auto ray_set_8 = compile(argv[13]);
		if (!require(ray_set_8, "spvRayState.dispatch->descriptorSetAddresses[8]", argv[13]))
			return 1;

		auto hit_attribute = compile(argv[14]);
		if (!require(hit_attribute, "packed_float3 a", argv[14]) ||
		    !require(hit_attribute, "packed_float3 b", argv[14]))
			return 1;

		auto ray_query_array = compile(argv[20]);
		if (!require(ray_query_array, "spvRayQueryMetadata queriesMetadata[2][3]", argv[20]) ||
		    !require(ray_query_array, "thread spvRayQueryMetadata (&queriesMetadata)[2][3]", argv[20]) ||
		    !require(ray_query_array, "queriesMetadata[i][j]", argv[20]) ||
		    !forbid(ray_query_array, "queries[i][j]Metadata", argv[20]) ||
		    !forbid(ray_query_array, "thread spvRayQueryMetadata& queriesMetadata", argv[20]))
			return 1;

		auto ordinary_rt = compile(argv[21], nullptr, false, true, 4, 0, true);
		auto ordinary = compile(argv[21], nullptr, false, false);
		if (ordinary_rt != ordinary || !forbid(ordinary_rt, "spvRayState", argv[21]) ||
		    !forbid(ordinary_rt, "Test runtime ABI", argv[21]))
		{
			std::cerr << argv[21] << ": ray profile changed a non-ray shader\n";
			return 1;
		}

		try
		{
			compile(argv[15]);
			std::cerr << argv[15] << ": oversized Boolean hit attribute was accepted\n";
			return 1;
		}
		catch (const CompilerError &error)
		{
			if (std::string(error.what()).find("cannot exceed 32") == std::string::npos)
				throw;
		}

		auto vector_array = compile(argv[16]);
		if (!require(vector_array, "packed_float3 samples[2]", argv[16]) ||
		    !require(vector_array, "[0] = float3((*reinterpret_cast<thread HitData*>", argv[16]) ||
		    !require(vector_array, "[1] = float3((*reinterpret_cast<thread HitData*>", argv[16]) ||
		    !forbid(vector_array, "struct HitData_", argv[16]) ||
		    !forbid(vector_array, "packed_spvUnsafeArray", argv[16]))
			return 1;

		auto unused_payload = compile(argv[17]);
		if (!require(unused_payload, "spvIFBPayload<Payload>", argv[17]) ||
		    !forbid(unused_payload, "spvIFBPayload<uint>", argv[17]))
			return 1;
		auto unused_payload_spv13 = compile(argv[18]);
		if (!require(unused_payload_spv13, "spvIFBPayload<Payload>", argv[18]) ||
		    !forbid(unused_payload_spv13, "spvIFBPayload<uint>", argv[18]))
			return 1;
		auto shared_type = compile(argv[19]);
		if (!require(shared_type, "struct spvRayData_", argv[19]) ||
		    !require(shared_type, "packed_float3 _m0", argv[19]) ||
		    !require(shared_type, "packed_rm_float3x2 _m2", argv[19]) ||
		    !require(shared_type, "._m0 = (*spvDescriptorSet0", argv[19]) ||
		    !forbid(shared_type, "_2 = *spvDescriptorSet0", argv[19]) ||
		    !forbid(shared_type, "spvRayData._m3[0].data", argv[19]))
			return 1;

		try { compile(argv[1], nullptr, false, true, 2, 4); }
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
