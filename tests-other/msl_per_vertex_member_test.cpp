// Checks that MSL rejects PerVertexKHR on an input block member, as it already rejects a PerVertexKHR input
// variable, and that the rejection does not reach the cases around it.

#include <spirv_cross_c.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Fragment shader reading member 0 of an input block, an array of 3 vec4 decorated PerVertexKHR.
// OpCapability Shader
// OpCapability FragmentBarycentricKHR
// OpExtension "SPV_KHR_fragment_shader_barycentric"
// OpMemoryModel Logical GLSL450
// OpEntryPoint Fragment %main "main" %blk %o
// OpExecutionMode %main OriginUpperLeft
// OpDecorate %Blk Block
// OpMemberDecorate %Blk 0 PerVertexKHR
// OpDecorate %blk Location 0
// OpDecorate %o Location 0
// %void = OpTypeVoid
// %fn = OpTypeFunction %void
// %float = OpTypeFloat 32
// %v4float = OpTypeVector %float 4
// %int = OpTypeInt 32 1
// %uint = OpTypeInt 32 0
// %uint_3 = OpConstant %uint 3
// %int_0 = OpConstant %int 0
// %int_1 = OpConstant %int 1
// %ptr_out = OpTypePointer Output %v4float
// %o = OpVariable %ptr_out Output
// %arr3 = OpTypeArray %v4float %uint_3
// %Blk = OpTypeStruct %arr3
// %ptr_blk = OpTypePointer Input %Blk
// %blk = OpVariable %ptr_blk Input
// %ptr_in = OpTypePointer Input %v4float
// %main = OpFunction %void None %fn
// %entry = OpLabel
// %p = OpAccessChain %ptr_in %blk %int_0 %int_1
// %v = OpLoad %v4float %p
// OpStore %o %v
// OpReturn
// OpFunctionEnd
static const SpvId member_spirv[] = {
	0x07230203, 0x00010300, 0x00070000, 0x00000015, 0x00000000, 0x00020011, 0x00000001, 0x00020011,
	0x000014a4, 0x000a000a, 0x5f565053, 0x5f52484b, 0x67617266, 0x746e656d, 0x6168735f, 0x5f726564,
	0x79726162, 0x746e6563, 0x00636972, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f, 0x00000004,
	0x00000001, 0x6e69616d, 0x00000000, 0x00000002, 0x00000003, 0x00030010, 0x00000001, 0x00000007,
	0x00030047, 0x00000004, 0x00000002, 0x00040048, 0x00000004, 0x00000000, 0x000014a5, 0x00040047,
	0x00000002, 0x0000001e, 0x00000000, 0x00040047, 0x00000003, 0x0000001e, 0x00000000, 0x00020013,
	0x00000005, 0x00030021, 0x00000006, 0x00000005, 0x00030016, 0x00000007, 0x00000020, 0x00040017,
	0x00000008, 0x00000007, 0x00000004, 0x00040015, 0x00000009, 0x00000020, 0x00000001, 0x00040015,
	0x0000000a, 0x00000020, 0x00000000, 0x0004002b, 0x0000000a, 0x0000000b, 0x00000003, 0x0004002b,
	0x00000009, 0x0000000c, 0x00000000, 0x0004002b, 0x00000009, 0x0000000d, 0x00000001, 0x00040020,
	0x0000000e, 0x00000003, 0x00000008, 0x0004003b, 0x0000000e, 0x00000003, 0x00000003, 0x0004001c,
	0x0000000f, 0x00000008, 0x0000000b, 0x0003001e, 0x00000004, 0x0000000f, 0x00040020, 0x00000010,
	0x00000001, 0x00000004, 0x0004003b, 0x00000010, 0x00000002, 0x00000001, 0x00040020, 0x00000011,
	0x00000001, 0x00000008, 0x00050036, 0x00000005, 0x00000001, 0x00000000, 0x00000006, 0x000200f8,
	0x00000012, 0x00060041, 0x00000011, 0x00000013, 0x00000002, 0x0000000c, 0x0000000d, 0x0004003d,
	0x00000008, 0x00000014, 0x00000013, 0x0003003e, 0x00000003, 0x00000014, 0x000100fd, 0x00010038,
};

// The same block without the PerVertexKHR decoration (and without the barycentric capability).
static const SpvId member_plain_spirv[] = {
	0x07230203, 0x00010300, 0x00070000, 0x00000015, 0x00000000, 0x00020011, 0x00000001, 0x0003000e,
	0x00000000, 0x00000001, 0x0007000f, 0x00000004, 0x00000001, 0x6e69616d, 0x00000000, 0x00000002,
	0x00000003, 0x00030010, 0x00000001, 0x00000007, 0x00030047, 0x00000004, 0x00000002, 0x00040047,
	0x00000002, 0x0000001e, 0x00000000, 0x00040047, 0x00000003, 0x0000001e, 0x00000000, 0x00020013,
	0x00000005, 0x00030021, 0x00000006, 0x00000005, 0x00030016, 0x00000007, 0x00000020, 0x00040017,
	0x00000008, 0x00000007, 0x00000004, 0x00040015, 0x00000009, 0x00000020, 0x00000001, 0x00040015,
	0x0000000a, 0x00000020, 0x00000000, 0x0004002b, 0x0000000a, 0x0000000b, 0x00000003, 0x0004002b,
	0x00000009, 0x0000000c, 0x00000000, 0x0004002b, 0x00000009, 0x0000000d, 0x00000001, 0x00040020,
	0x0000000e, 0x00000003, 0x00000008, 0x0004003b, 0x0000000e, 0x00000003, 0x00000003, 0x0004001c,
	0x0000000f, 0x00000008, 0x0000000b, 0x0003001e, 0x00000004, 0x0000000f, 0x00040020, 0x00000010,
	0x00000001, 0x00000004, 0x0004003b, 0x00000010, 0x00000002, 0x00000001, 0x00040020, 0x00000011,
	0x00000001, 0x00000008, 0x00050036, 0x00000005, 0x00000001, 0x00000000, 0x00000006, 0x000200f8,
	0x00000012, 0x00060041, 0x00000011, 0x00000013, 0x00000002, 0x0000000c, 0x0000000d, 0x0004003d,
	0x00000008, 0x00000014, 0x00000013, 0x0003003e, 0x00000003, 0x00000014, 0x000100fd, 0x00010038,
};

// A PerVertexKHR input variable, an array of 3 vec4, instead of a block member.
// OpCapability Shader
// OpCapability FragmentBarycentricKHR
// OpExtension "SPV_KHR_fragment_shader_barycentric"
// OpMemoryModel Logical GLSL450
// OpEntryPoint Fragment %main "main" %vin %o
// OpExecutionMode %main OriginUpperLeft
// OpDecorate %vin PerVertexKHR
// OpDecorate %vin Location 0
// OpDecorate %o Location 0
// %void = OpTypeVoid
// %fn = OpTypeFunction %void
// %float = OpTypeFloat 32
// %v4float = OpTypeVector %float 4
// %int = OpTypeInt 32 1
// %uint = OpTypeInt 32 0
// %uint_3 = OpConstant %uint 3
// %int_1 = OpConstant %int 1
// %ptr_out = OpTypePointer Output %v4float
// %o = OpVariable %ptr_out Output
// %arr3 = OpTypeArray %v4float %uint_3
// %ptr_arr = OpTypePointer Input %arr3
// %vin = OpVariable %ptr_arr Input
// %ptr_in = OpTypePointer Input %v4float
// %main = OpFunction %void None %fn
// %entry = OpLabel
// %p = OpAccessChain %ptr_in %vin %int_1
// %v = OpLoad %v4float %p
// OpStore %o %v
// OpReturn
// OpFunctionEnd
static const SpvId variable_spirv[] = {
	0x07230203, 0x00010300, 0x00070000, 0x00000013, 0x00000000, 0x00020011, 0x00000001, 0x00020011,
	0x000014a4, 0x000a000a, 0x5f565053, 0x5f52484b, 0x67617266, 0x746e656d, 0x6168735f, 0x5f726564,
	0x79726162, 0x746e6563, 0x00636972, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f, 0x00000004,
	0x00000001, 0x6e69616d, 0x00000000, 0x00000002, 0x00000003, 0x00030010, 0x00000001, 0x00000007,
	0x00030047, 0x00000002, 0x000014a5, 0x00040047, 0x00000002, 0x0000001e, 0x00000000, 0x00040047,
	0x00000003, 0x0000001e, 0x00000000, 0x00020013, 0x00000004, 0x00030021, 0x00000005, 0x00000004,
	0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040015,
	0x00000008, 0x00000020, 0x00000001, 0x00040015, 0x00000009, 0x00000020, 0x00000000, 0x0004002b,
	0x00000009, 0x0000000a, 0x00000003, 0x0004002b, 0x00000008, 0x0000000b, 0x00000001, 0x00040020,
	0x0000000c, 0x00000003, 0x00000007, 0x0004003b, 0x0000000c, 0x00000003, 0x00000003, 0x0004001c,
	0x0000000d, 0x00000007, 0x0000000a, 0x00040020, 0x0000000e, 0x00000001, 0x0000000d, 0x0004003b,
	0x0000000e, 0x00000002, 0x00000001, 0x00040020, 0x0000000f, 0x00000001, 0x00000007, 0x00050036,
	0x00000004, 0x00000001, 0x00000000, 0x00000005, 0x000200f8, 0x00000010, 0x00050041, 0x0000000f,
	0x00000011, 0x00000002, 0x0000000b, 0x0004003d, 0x00000007, 0x00000012, 0x00000011, 0x0003003e,
	0x00000003, 0x00000012, 0x000100fd, 0x00010038,
};

static const char *const expected_error = "PerVertexKHR decoration is not supported in MSL.";

// Compiles the module with the given backend. Returns the source on success, nullptr on failure, and copies the
// error message into error.
static const char *compile(spvc_context ctx, const SpvId *spirv, size_t count, spvc_backend backend, const char **error)
{
	spvc_parsed_ir ir = nullptr;
	spvc_compiler compiler = nullptr;
	spvc_compiler_options options = nullptr;
	const char *source = nullptr;

	*error = "";
	if (spvc_context_parse_spirv(ctx, spirv, count, &ir) != SPVC_SUCCESS ||
	    spvc_context_create_compiler(ctx, backend, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler) != SPVC_SUCCESS ||
	    spvc_compiler_create_compiler_options(compiler, &options) != SPVC_SUCCESS)
	{
		*error = spvc_context_get_last_error_string(ctx);
		return nullptr;
	}
	if (backend == SPVC_BACKEND_GLSL)
		spvc_compiler_options_set_uint(options, SPVC_COMPILER_OPTION_GLSL_VERSION, 450);
	spvc_compiler_install_compiler_options(compiler, options);
	if (spvc_compiler_compile(compiler, &source) != SPVC_SUCCESS)
	{
		*error = spvc_context_get_last_error_string(ctx);
		return nullptr;
	}
	return source;
}

static int failures = 0;

static void check(bool ok, const char *what, const char *detail)
{
	printf("%s: %s%s%s\n", ok ? "PASS" : "FAIL", what, *detail ? " | " : "", detail);
	if (!ok)
		failures++;
}

int main()
{
	spvc_context ctx = nullptr;
	if (spvc_context_create(&ctx) != SPVC_SUCCESS)
		return EXIT_FAILURE;

	const char *error;
	const char *source;

	source = compile(ctx, member_spirv, sizeof(member_spirv) / sizeof(SpvId), SPVC_BACKEND_MSL, &error);
	check(!source && strstr(error, expected_error), "MSL rejects a PerVertexKHR input block member", error);

	source = compile(ctx, member_plain_spirv, sizeof(member_plain_spirv) / sizeof(SpvId), SPVC_BACKEND_MSL, &error);
	check(source != nullptr, "MSL accepts the same block without PerVertexKHR", error);

	source = compile(ctx, variable_spirv, sizeof(variable_spirv) / sizeof(SpvId), SPVC_BACKEND_MSL, &error);
	check(!source && strstr(error, expected_error), "MSL still rejects a PerVertexKHR input variable", error);

	source = compile(ctx, variable_spirv, sizeof(variable_spirv) / sizeof(SpvId), SPVC_BACKEND_GLSL, &error);
	check(source && strstr(source, "pervertexEXT"), "GLSL still accepts a PerVertexKHR input variable", error);

	spvc_context_destroy(ctx);
	return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
