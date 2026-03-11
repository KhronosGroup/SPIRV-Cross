/*
 * Copyright 2016-2021 The Brenwill Workshop Ltd.
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * At your option, you may choose to accept this material under either:
 * 1. The Apache License, Version 2.0, found at, or
 * 2. The MIT License, found at.
 */

#include "spirv_opencl.hpp"
#include "GLSL.std.450.h"

#include <algorithm>
#include <iostream>
#include <utility>

using namespace SPIRV_CROSS_SPV_HEADER_NAMESPACE;
using namespace SPIRV_CROSS_NAMESPACE;
using namespace std;

CompilerOpenCL::CompilerOpenCL(vector<uint32_t> spirv_)
    : CompilerGLSL(std::move(spirv_))
{
}

CompilerOpenCL::CompilerOpenCL(const uint32_t *ir_, size_t word_count)
    : CompilerGLSL(ir_, word_count)
{
}

CompilerOpenCL::CompilerOpenCL(const ParsedIR &ir_)
    : CompilerGLSL(ir_)
{
}

CompilerOpenCL::CompilerOpenCL(ParsedIR &&ir_)
    : CompilerGLSL(std::move(ir_))
{
}

string CompilerOpenCL::compile()
{
	if (get_execution_model() != ExecutionModelGLCompute)
		SPIRV_CROSS_THROW("OpenCL backend only supports compute shaders (ExecutionModelGLCompute).");

	ir.fixup_reserved_names();

	options.vulkan_semantics = true;
	options.es = false;
	options.version = 450;

	backend.null_pointer_literal = "NULL";
	backend.float_literal_suffix = true;
	backend.double_literal_suffix = true;
	backend.uint32_t_literal_suffix = true;
	backend.int16_t_literal_suffix = "s";
	backend.uint16_t_literal_suffix = "us";
	backend.basic_int_type = "int";
	backend.basic_uint_type = "uint";
	backend.basic_int8_type = "char";
	backend.basic_uint8_type = "uchar";
	backend.basic_int16_type = "short";
	backend.basic_uint16_type = "ushort";
	backend.boolean_mix_function = "mix";
	backend.swizzle_is_function = false;
	backend.shared_is_implied = false;
	backend.use_initializer_list = true;
	backend.use_typed_initializer_list = true;
	backend.native_row_major_matrix = false;
	backend.unsized_array_supported = false;
	backend.can_declare_arrays_inline = false;
	backend.allow_truncated_access_chain = true;
	backend.comparison_image_samples_scalar = true;
	backend.native_pointers = true;
	backend.nonuniform_qualifier = "";
	backend.supports_empty_struct = true;
	backend.support_64bit_switch = opencl_options.enable_64bit_atomics;
	backend.boolean_in_struct_remapped_type = SPIRType::Boolean;
	backend.discard_literal = "";
	backend.demote_literal = "";
	backend.workgroup_size_is_hidden = false;
	backend.supports_extensions = true;
	backend.force_gl_in_out_block = false;
	backend.force_merged_mesh_block = false;
	backend.array_is_value_type = false;
	backend.array_is_value_type_in_buffer_blocks = false;
	backend.support_pointer_to_pointer = true;
	backend.implicit_c_integer_promotion_rules = true;
	backend.supports_spec_constant_array_size = false;

	fixup_anonymous_struct_names();
	fixup_type_alias();
	replace_illegal_names();
	build_function_control_flow_graphs_and_analyze();
	update_active_builtins();
	analyze_image_and_sampler_usage();
	analyze_interlocked_resource_usage();

	set_enabled_interface_variables(get_active_interface_variables());
	reorder_type_alias();

	uint32_t pass_count = 0;
	do
	{
		reset(pass_count);
		buffer.reset();

		emit_header();
		emit_specialization_constants_and_structs();
		emit_resources();
		emit_function(get<SPIRFunction>(ir.default_entry_point), Bitset());

		pass_count++;
	} while (is_forcing_recompilation());

	return buffer.str();
}

bool CompilerOpenCL::specialization_constant_is_macro(uint32_t const_id) const
{
	return constant_macro_ids.find(const_id) != constant_macro_ids.end();
}

void CompilerOpenCL::emit_header()
{
	statement("// Generated from SPIR-V by SPIRV-Cross (OpenCL backend)");
	statement("");

	if (opencl_options.opencl_version >= 200)
		statement("#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable");
	if (opencl_options.enable_fp64)
		statement("#pragma OPENCL EXTENSION cl_khr_fp64 : enable");
	if (opencl_options.enable_64bit_atomics && opencl_options.opencl_version >= 200)
		statement("#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable");
	statement("");

	for (auto &header : header_lines)
		statement(header);
	if (!header_lines.empty())
		statement("");
}

const char *CompilerOpenCL::to_storage_qualifiers_glsl(const SPIRVariable &)
{
	// OpenCL uses address space in type, not as a separate qualifier like "uniform"
	return "";
}

void CompilerOpenCL::emit_resources()
{
	replace_illegal_names();
}

void CompilerOpenCL::replace_illegal_names()
{
	static const unordered_set<string> keywords = {
		"char",
		"char2",
		"char3",
		"char4",
		"char8",
		"char16",
		"uchar",
		"uchar2",
		"uchar3",
		"uchar4",
		"uchar8",
		"uchar16",
		"short",
		"short2",
		"short3",
		"short4",
		"short8",
		"short16",
		"ushort",
		"ushort2",
		"ushort3",
		"ushort4",
		"ushort8",
		"ushort16",
		"int",
		"int2",
		"int3",
		"int4",
		"int8",
		"int16",
		"uint",
		"uint2",
		"uint3",
		"uint4",
		"uint8",
		"uint16",
		"long",
		"long2",
		"long3",
		"long4",
		"long8",
		"long16",
		"ulong",
		"ulong2",
		"ulong3",
		"ulong4",
		"ulong8",
		"ulong16",
		"float",
		"float2",
		"float3",
		"float4",
		"float8",
		"float16",
		"double",
		"double2",
		"double3",
		"double4",
		"double8",
		"double16",
		"bool",
		"bool2",
		"bool3",
		"bool4",
		"bool8",
		"bool16",
		"half",
		"half2",
		"half3",
		"half4",
		"half8",
		"half16",
		"quad",
		"quad2",
		"quad3",
		"quad4",
		"quad8",
		"quad16",
		"complex",
		"imaginary"
		"__global",
		"global",
		"__local",
		"local",
		"__constant",
		"constant",
		"__private",
		"private",
		"image1d_t",
		"image1d_buffer_t",
		"image1d_array_t",
		"image2d_t",
		"image2d_array_t",
		"image2d_depth_t",
		"image2d_array_depth_t",
		"image3d_t",
		"sampler_t",
		"event_t",
		"clk_event_t",
		"ndrange_t",
		"queue_t",
		"reserve_id_t",
		"__kernel",
		"kernel",
		"__read_only",
		"read_only",
		"__write_only",
		"write_only",
		"__read_write",
		"read_write",
		"atomic",
		"pipe",
		"MAXFLOAT",
		"HUGE_VALF",
		"INFINITY",
		"NAN",
		"HUGE_VAL",
		"M_E_F",
		"M_LOG2E_F",
		"M_LOG10E_F",
		"M_LN2_F",
		"M_LN10_F",
		"M_PI_F",
		"M_PI_2_F",
		"M_PI_4_F",
		"M_1_PI_F",
		"M_2_PI_F",
		"M_2_SQRTPI_F",
		"M_SQRT2_F",
		"M_SQRT1_2_F",
	};

	CompilerGLSL::replace_illegal_names(keywords);
	CompilerGLSL::replace_illegal_names();
}

void CompilerOpenCL::emit_workgroup_size_attribute()
{
	auto &ep = get_entry_point();
	uint32_t x = ep.workgroup_size.x;
	uint32_t y = ep.workgroup_size.y;
	uint32_t z = ep.workgroup_size.z;
	if (x == 0)
		x = 1;
	if (y == 0)
		y = 1;
	if (z == 0)
		z = 1;
	statement("__attribute__((reqd_work_group_size(", x, ", ", y, ", ", z, ")))");
}

void CompilerOpenCL::emit_entry_point_declarations()
{
	// Emit local variables for compute builtins so that builtin_to_glsl can return a name
	if (!processing_entry_point)
		return;

	auto &execution = get_entry_point();
	if (execution.model != ExecutionModelGLCompute)
		return;

	bool need_workgroup_id = active_input_builtins.get(BuiltInWorkgroupId);
	bool need_local_id = active_input_builtins.get(BuiltInLocalInvocationId);
	bool need_global_id = active_input_builtins.get(BuiltInGlobalInvocationId);
	bool need_num_workgroups = active_input_builtins.get(BuiltInNumWorkgroups);
	bool need_workgroup_size = active_input_builtins.get(BuiltInWorkgroupSize);
	bool need_local_invocation_index = active_input_builtins.get(BuiltInLocalInvocationIndex);
	bool need_global_size = active_input_builtins.get(BuiltInGlobalSize);

	if (need_workgroup_id)
		statement("uint3 spvWorkgroupId = (uint3)(get_group_id(0), get_group_id(1), get_group_id(2));");
	if (need_local_id)
		statement("uint3 spvLocalInvocationId = (uint3)(get_local_id(0), get_local_id(1), get_local_id(2));");
	if (need_global_id)
		statement("uint3 spvGlobalInvocationId = (uint3)(get_global_id(0), get_global_id(1), get_global_id(2));");
	if (need_num_workgroups)
		statement("uint3 spvNumWorkgroups = (uint3)(get_num_groups(0), get_num_groups(1), get_num_groups(2));");
	if (need_workgroup_size)
		statement("uint3 spvWorkgroupSize = (uint3)(get_local_size(0), get_local_size(1), get_local_size(2));");
	if (need_local_invocation_index)
		statement("uint spvLocalInvocationIndex = get_local_id(2) * get_local_size(0) * get_local_size(1) + "
		          "get_local_id(1) * get_local_size(0) + get_local_id(0);");
	if (need_global_size)
		statement("uint3 spvGlobalSize = (uint3)(get_global_size(0), get_global_size(1), get_global_size(2));");
}

string CompilerOpenCL::builtin_to_glsl(BuiltIn builtin, StorageClass storage)
{
	(void)storage;
	switch (builtin)
	{
	case BuiltInWorkgroupId:
		return "spvWorkgroupId";
	case BuiltInLocalInvocationId:
		return "spvLocalInvocationId";
	case BuiltInGlobalInvocationId:
		return "spvGlobalInvocationId";
	case BuiltInNumWorkgroups:
		return "spvNumWorkgroups";
	case BuiltInWorkgroupSize:
		return "spvWorkgroupSize";
	case BuiltInLocalInvocationIndex:
		return "spvLocalInvocationIndex";
	case BuiltInGlobalSize:
		return "spvGlobalSize";
	case BuiltInNumSubgroups:
	case BuiltInSubgroupId:
	case BuiltInSubgroupSize:
	case BuiltInSubgroupLocalInvocationId:
		SPIRV_CROSS_THROW("OpenCL subgroup builtins not yet implemented.");
	default:
		SPIRV_CROSS_THROW("Unsupported builtin for OpenCL compute shader.");
	}
}

bool CompilerOpenCL::builtin_translates_to_nonarray(BuiltIn builtin) const
{
	(void)builtin;
	return false;
}

// In OpenCL, address space qualifiers are required for all pointer or reference variables
string CompilerOpenCL::get_variable_address_space(const SPIRVariable &argument)
{
	const auto &type = get<SPIRType>(argument.basetype);
	return get_type_address_space(type, argument.self, true);
}

string CompilerOpenCL::get_type_address_space(const SPIRType &type, uint32_t id, bool argument)
{
	// This can be called for variable pointer contexts as well, so be very careful about which method we choose.
	Bitset flags;
	auto *var = maybe_get<SPIRVariable>(id);
	if (var && type.basetype == SPIRType::Struct &&
	    (has_decoration(type.self, DecorationBlock) || has_decoration(type.self, DecorationBufferBlock)))
		flags = get_buffer_block_flags(id);
	else
	{
		flags = get_decoration_bitset(id);

		if (type.basetype == SPIRType::Struct &&
		    (has_decoration(type.self, DecorationBlock) || has_decoration(type.self, DecorationBufferBlock)))
		{
			flags.merge_or(ir.get_buffer_block_type_flags(type));
		}
	}

	const char *addr_space = "";
	switch (type.storage)
	{
	case StorageClassUniform:
	case StorageClassStorageBuffer:
		addr_space = "__global";
		break;
	case StorageClassUniformConstant:
	case StorageClassPushConstant:
		addr_space = "__constant";
		break;
	case StorageClassWorkgroup:
		addr_space = "__local";
		break;
	default:
		// __private is default and would be redundant
		break;
	}
	return addr_space;
}

const char *CompilerOpenCL::to_restrict(uint32_t id, bool space)
{
	// This can be called for variable pointer contexts as well, so be very careful about which method we choose.
	Bitset flags;
	if (ir.ids[id].get_type() == TypeVariable)
	{
		uint32_t type_id = expression_type_id(id);
		auto &type = expression_type(id);
		if (type.basetype == SPIRType::Struct &&
		    (has_decoration(type_id, DecorationBlock) || has_decoration(type_id, DecorationBufferBlock)))
			flags = get_buffer_block_flags(id);
		else
			flags = get_decoration_bitset(id);
	}
	else
		flags = get_decoration_bitset(id);

	return flags.get(DecorationRestrict) || flags.get(DecorationRestrictPointerEXT) ?
	           (space ? "__restrict " : "__restrict") :
	           "";
}

string CompilerOpenCL::type_to_glsl(const SPIRType &type, uint32_t id, bool member)
{
	string type_name;

	// Pointer?
	if (is_pointer(type) || type_is_array_of_pointers(type))
	{
		assert(type.pointer_depth > 0);

		const char *restrict_kw;

		auto type_address_space = get_type_address_space(type, id);
		const auto *p_parent_type = &get<SPIRType>(type.parent_type);

		// Work around C pointer qualifier rules. If glsl_type is a pointer type as well
		// we'll need to emit the address space to the right.
		// We could always go this route, but it makes the code unnatural.
		// Prefer emitting thread T *foo over T thread* foo since it's more readable,
		// but we'll have to emit thread T * thread * T constant bar; for example.
		if (is_pointer(type) && is_pointer(*p_parent_type))
			type_name = join(type_to_glsl(*p_parent_type, id), " ", type_address_space, " ");
		else
		{
			// Since this is not a pointer-to-pointer, ensure we've dug down to the base type.
			// Some situations chain pointers even though they are not formally pointers-of-pointers.
			while (is_pointer(*p_parent_type))
				p_parent_type = &get<SPIRType>(p_parent_type->parent_type);

			type_name = join(type_address_space, " ", type_to_glsl(*p_parent_type, id));
		}

		switch (type.basetype)
		{
		case SPIRType::Image:
		case SPIRType::SampledImage:
		case SPIRType::Sampler:
			// These are handles.
			break;
		default:
			// Anything else can be a raw pointer.
			type_name += "*";
			restrict_kw = to_restrict(id, false);
			if (*restrict_kw)
			{
				type_name += " ";
				type_name += restrict_kw;
			}
			break;
		}
		return type_name;
	}

	switch (type.basetype)
	{
	case SPIRType::Struct:
		// Need OpName lookup here to get a "sensible" name for a struct.
		type_name = to_name(type.self);
		break;

	case SPIRType::Image:
	case SPIRType::SampledImage:
		return image_type_glsl(type, id, member);

	case SPIRType::Sampler:
		return "sampler_t";

	case SPIRType::Void:
		return "void";

	case SPIRType::AtomicCounter:
		return "atomic_uint";

	// Scalars
	case SPIRType::Boolean:
		type_name = "bool";
		break;

	case SPIRType::Char:
	case SPIRType::SByte:
		type_name = "char";
		break;
	case SPIRType::UByte:
		type_name = "uchar";
		break;
	case SPIRType::Short:
		type_name = "short";
		break;
	case SPIRType::UShort:
		type_name = "ushort";
		break;
	case SPIRType::Int:
		type_name = "int";
		break;
	case SPIRType::UInt:
		type_name = "uint";
		break;
	case SPIRType::Int64:
		type_name = "long";
		break;
	case SPIRType::UInt64:
		type_name = "ulong";
		break;
	case SPIRType::Half:
		type_name = "half";
		break;
	case SPIRType::Float:
		type_name = "float";
		break;
	case SPIRType::Double:
		if (!opencl_options.enable_fp64)
			SPIRV_CROSS_THROW("Double requires cl_khr_fp64.");
		type_name = "double";
		break;

	default:
		return "unknown_type";
	}

	// Vector?
	if (type.vecsize > 1)
		type_name += to_string(type.vecsize);

	return type_name;
}

string CompilerOpenCL::type_to_glsl(const SPIRType &type, uint32_t id)
{
	return type_to_glsl(type, id, false);
}

string CompilerOpenCL::image_type_glsl(const SPIRType &type, uint32_t id, bool member)
{
	(void)id;
	(void)member;
	if (type.basetype != SPIRType::Image)
		return "";

	bool readonly = type.image.sampled != 2;
	const char *access = readonly ? "read_only" : "write_only";
	switch (type.image.dim)
	{
	case Dim1D:
		return join(access, " image1d_t");
	case Dim2D:
		return type.image.arrayed ? join(access, " image2d_array_t") : join(access, " image2d_t");
	case Dim3D:
		return join(access, " image3d_t");
	case DimCube:
		return join(access, " image2d_t"); // OpenCL has no cube; use 2D
	case DimBuffer:
		return join(access, " image1d_buffer_t");
	default:
		SPIRV_CROSS_THROW("Unsupported image dimension for OpenCL.");
	}
}

std::string CompilerOpenCL::type_to_glsl_constructor(const SPIRType &type)
{
	string ret = CompilerGLSL::type_to_glsl_constructor(type);
	printf("type_to_glsl_constructor: %s\n", ret.c_str());
	if (!ret.empty())
		ret = join("(", ret, ")");
	return ret;
}

// GCC workaround of lambdas calling protected funcs
std::string CompilerOpenCL::variable_decl(const SPIRType &type, const std::string &name, uint32_t id)
{
	return CompilerGLSL::variable_decl(type, name, id);
}

std::string CompilerOpenCL::entry_point_args(bool append_comma)
{
	// Reset flattening maps for this compilation pass
	flattened_buffer_vars.clear();
	push_const_member_map.clear();

	std::string ep_args;

	struct Resource
	{
		SPIRVariable *var;
		SPIRVariable *discrete_descriptor_alias;
		string name;
		SPIRType::BaseType basetype;
		uint32_t index;
		uint32_t plane;
		uint32_t secondary_index;
	};

	SmallVector<Resource> resources;

	ir.for_each_typed_id<SPIRVariable>(
	    [&](uint32_t var_id, SPIRVariable &var)
	    {
		    auto &type = get_variable_data_type(var);
		    /*
		    if (var.storage == StorageClassPushConstant)
		    {
			    for (uint32_t mbr_idx = 0; mbr_idx < uint32_t(type.member_types.size()); mbr_idx++)
			    {
				    if (!ep_args.empty())
					    ep_args += ", ";

				    auto mbr_name = to_member_name(type, mbr_idx);
					const auto &member_type = this->get<SPIRType>(type.member_types[mbr_idx]);
				    ep_args += join(this->type_to_glsl(member_type), " ", mbr_name);
				    // Record the mapping so emit_instruction can rewrite access chains
				    push_const_member_map[var_id][mbr_idx] = mbr_name;
			    }
		    }
			*/
		    if (var.storage == StorageClassStorageBuffer || has_decoration(type.self, DecorationBufferBlock))
		    {
			    Bitset flags = ir.get_buffer_block_flags(var);
			    bool is_readonly = flags.get(DecorationNonWritable);

			    auto to_structuredbuffer_subtype_name = [this](const SPIRType &parent_type) -> std::string
			    {
				    if (parent_type.basetype == SPIRType::Struct && parent_type.member_types.size() == 1)
				    {
					    // Use type of first struct member as a StructuredBuffer will have only one '._m0' field in SPIR-V
					    const auto &member0_type = this->get<SPIRType>(parent_type.member_types.front());
					    return this->type_to_glsl(member0_type);
				    }
				    else
				    {
					    // Otherwise, this StructuredBuffer only has a basic subtype, e.g. StructuredBuffer<int>
					    return this->type_to_glsl(parent_type);
				    }
			    };
			    if (!ep_args.empty())
				    ep_args += ", ";

			    ep_args += join("__global ", is_readonly ? "const " : "", to_structuredbuffer_subtype_name(type), "* ",
			                    to_name(var_id));
			    // Record so emit_instruction can rewrite OpAccessChain against this var
			    flattened_buffer_vars.insert(var_id);
		    }
		    else if ((var.storage == StorageClassUniform || var.storage == StorageClassUniformConstant ||
		              var.storage == StorageClassPushConstant || var.storage == StorageClassStorageBuffer) &&
		             !is_hidden_variable(var))
		    {
			    switch (var.basetype)
			    {
			    case SPIRType::Struct:
			    {
				    break;
			    }
			    case SPIRType::Sampler:
				    break;
			    case SPIRType::Image:
			    {
				    if (!ep_args.empty())
					    ep_args += ", ";

				    ep_args += type_to_glsl(type, var_id) + " " + to_name(var_id);
				    break;
			    }
			    case SPIRType::AccelerationStructure:
			    {
				    break;
			    }
			    default:
				    if (!ep_args.empty())
					    ep_args += ", ";

				    ep_args += type_to_glsl(type, var_id) + " " + to_name(var_id);
				    break;
			    }
		    }
	    });

	if (!ep_args.empty() && append_comma)
		ep_args += ", ";

	return ep_args;
}

string CompilerOpenCL::get_inner_entry_point_name() const
{
	return "comp_main";
}

void CompilerOpenCL::emit_function_prototype(SPIRFunction &func, const Bitset &return_flags)
{
	(void)return_flags;
	if (func.self != ir.default_entry_point)
		add_function_overload(func);

	// Entry point: __kernel void name(...)
	emit_workgroup_size_attribute();
	string decl;
	decl += "__kernel void ";
	if (func.self == ir.default_entry_point)
	{
		decl += get_inner_entry_point_name();
		processing_entry_point = true;
	}
	else
		decl += to_name(func.self);
	decl += "(";

	if (processing_entry_point)
	{
		decl += entry_point_args(!func.arguments.empty());

		// append entry point args to avoid conflicts in local variable names.
		local_variable_names.insert(resource_names.begin(), resource_names.end());
	}

	for (auto &arg : func.arguments)
	{
		add_local_variable_name(arg.id);

		decl += argument_decl(arg);
		if (&arg != &func.arguments.back())
			decl += ", ";

		// Hold a pointer to the parameter so we can invalidate the readonly field if needed.
		auto *var = maybe_get<SPIRVariable>(arg.id);
		if (var)
			var->parameter = &arg;
	}

	decl += ")";
	statement(decl);
}

void CompilerOpenCL::emit_specialization_constants_and_structs()
{
	SpecializationConstant wg_x, wg_y, wg_z;
	ID workgroup_size_id = get_work_group_size_specialization_constants(wg_x, wg_y, wg_z);

	bool emitted = false;
	unordered_set<uint32_t> declared_structs;
	unordered_set<uint32_t> aligned_structs;

	// Very particular use of the soft loop lock.
	// align_struct may need to create custom types on the fly, but we don't care about
	// these types for purpose of iterating over them in ir.ids_for_type and friends.
	auto loop_lock = ir.create_loop_soft_lock();

	// Physical storage buffer pointers can have cyclical references,
	// so emit forward declarations of them before other structs.
	// Ignore type_id because we want the underlying struct type from the pointer.
	ir.for_each_typed_id<SPIRType>(
	    [&](uint32_t /* type_id */, const SPIRType &type)
	    {
		    if (type.basetype == SPIRType::Struct && type.pointer &&
		        type.storage == StorageClassPhysicalStorageBuffer && declared_structs.count(type.self) == 0)
		    {
			    statement("struct ", to_name(type.self), ";");
			    declared_structs.insert(type.self);
			    emitted = true;
		    }
	    });
	if (emitted)
		statement("");

	emitted = false;
	declared_structs.clear();

	// It is possible to have multiple spec constants that use the same spec constant ID.
	// The most common cause of this is defining spec constants in GLSL while also declaring
	// the workgroup size to use those spec constants. But, Metal forbids declaring more than
	// one variable with the same function constant ID.
	// In this case, we must only declare one variable with the [[function_constant(id)]]
	// attribute, and use its initializer to initialize all the spec constants with
	// that ID.
	std::unordered_map<uint32_t, ConstantID> unique_func_constants;

	for (auto &id_ : ir.ids_for_constant_undef_or_type)
	{
		auto &id = ir.ids[id_];

		if (id.get_type() == TypeConstant)
		{
			auto &c = id.get<SPIRConstant>();

			if (c.specialization)
			{
				auto &type = get<SPIRType>(c.constant_type);
				string sc_type_name = type_to_glsl(type);
				add_resource_name(c.self);
				string sc_name = to_name(c.self);

				// Specialization constants are only supported in SPIR-V not OpenCL C.
				// Just declare the "default" directly.
				if (has_decoration(c.self, DecorationSpecId))
				{
					// Fallback to macro overrides.
					uint32_t constant_id = get_decoration(c.self, DecorationSpecId);
					c.specialization_constant_macro_name = constant_value_macro_name(constant_id);

					statement("#ifndef ", c.specialization_constant_macro_name);
					statement("#define ", c.specialization_constant_macro_name, " ", constant_expression(c));
					statement("#endif");
					statement("constant ", sc_type_name, " ", sc_name, " = ", c.specialization_constant_macro_name,
					          ";");

					// Record the usage of macro
					constant_macro_ids.insert(constant_id);
				}
				else
				{
					// Composite specialization constants must be built from other specialization constants.
					statement("constant ", sc_type_name, " ", sc_name, " = ", constant_expression(c), ";");
				}
				emitted = true;
			}
		}
		else if (id.get_type() == TypeConstantOp)
		{
			auto &c = id.get<SPIRConstantOp>();
			auto &type = get<SPIRType>(c.basetype);
			add_resource_name(c.self);
			auto name = to_name(c.self);
			statement("constant ", variable_decl(type, name), " = ", constant_op_expression(c), ";");
			emitted = true;
		}
		else if (id.get_type() == TypeType)
		{
			// Output non-builtin interface structs. These include local function structs
			// and structs nested within uniform and read-write buffers.
			auto &type = id.get<SPIRType>();
			TypeID type_id = type.self;

			bool is_struct = (type.basetype == SPIRType::Struct) && type.array.empty() && !type.pointer;
			bool is_block =
			    has_decoration(type.self, DecorationBlock) || has_decoration(type.self, DecorationBufferBlock);

			bool is_builtin_block = is_block && is_builtin_type(type);
			bool is_declarable_struct = is_struct && !is_builtin_block;

			// Align and emit declarable structs...but avoid declaring each more than once.
			if (is_declarable_struct && declared_structs.count(type_id) == 0)
			{
				if (emitted)
					statement("");
				emitted = false;

				declared_structs.insert(type_id);

				// Make sure we declare the underlying struct type, and not the "decorated" type with pointers, etc.
				emit_struct(get<SPIRType>(type_id));
			}
		}
		else if (id.get_type() == TypeUndef)
		{
			auto &undef = id.get<SPIRUndef>();
			auto &type = get<SPIRType>(undef.basetype);
			// OpUndef can be void for some reason ...
			if (type.basetype == SPIRType::Void)
				return;
		}
	}

	if (emitted)
		statement("");
}

void CompilerOpenCL::emit_instruction(const Instruction &instruction)
{
	auto ops = stream(instruction);
	auto opcode = static_cast<Op>(instruction.op);

	// Map buffer atomics to OpenCL C names (atomic_add, atomic_sub, etc.)
	auto opencl_atomic = [this, ops](const char *opencl_op)
	{
		if (check_atomic_image(ops[2]))
			SPIRV_CROSS_THROW("Image atomics not yet implemented for OpenCL.");
		emit_atomic_func_op(ops[0], ops[1], ops[2], ops[5], opencl_op);
	};

	switch (opcode)
	{
	case OpAtomicExchange:
		if (check_atomic_image(ops[2]))
			SPIRV_CROSS_THROW("Image atomics not yet implemented for OpenCL.");
		emit_atomic_func_op(ops[0], ops[1], ops[2], ops[5], "atomic_xchg");
		break;
	case OpAtomicCompareExchange:
		if (check_atomic_image(ops[2]))
			SPIRV_CROSS_THROW("Image atomics not yet implemented for OpenCL.");
		// OpenCL atomic_cmpxchg(ptr, expected, desired)
		emit_atomic_func_op(ops[0], ops[1], ops[2], ops[7], ops[6], "atomic_cmpxchg");
		break;
	case OpAtomicIAdd:
	case OpAtomicFAddEXT:
		opencl_atomic("atomic_add");
		break;
	case OpAtomicISub:
	{
		if (check_atomic_image(ops[2]))
			SPIRV_CROSS_THROW("Image atomics not yet implemented for OpenCL.");
		forced_temporaries.insert(ops[1]);
		auto expr = join("atomic_sub(", to_atomic_ptr_expression(ops[2]), ", ", to_enclosed_expression(ops[5]), ")");
		emit_op(ops[0], ops[1], expr, should_forward(ops[2]) && should_forward(ops[5]));
		flush_all_atomic_capable_variables();
		break;
	}
	case OpAtomicSMin:
	case OpAtomicUMin:
		opencl_atomic("atomic_min");
		break;
	case OpAtomicSMax:
	case OpAtomicUMax:
		opencl_atomic("atomic_max");
		break;
	case OpAtomicAnd:
		opencl_atomic("atomic_and");
		break;
	case OpAtomicOr:
		opencl_atomic("atomic_or");
		break;
	case OpAtomicXor:
		opencl_atomic("atomic_xor");
		break;
	case OpAtomicLoad:
	{
		if (check_atomic_image(ops[2]))
			SPIRV_CROSS_THROW("Image atomics not yet implemented for OpenCL.");
		auto &type = expression_type(ops[2]);
		forced_temporaries.insert(ops[1]);
		bool unsigned_type = (type.basetype == SPIRType::UInt);
		const char *increment = unsigned_type ? "0u" : "0";
		emit_op(ops[0], ops[1], join("atomic_add(", to_atomic_ptr_expression(ops[2]), ", ", increment, ")"), false);
		flush_all_atomic_capable_variables();
		break;
	}
	case OpAtomicStore:
	{
		if (check_atomic_image(ops[0]))
			SPIRV_CROSS_THROW("Image atomics not yet implemented for OpenCL.");
		statement("atomic_xchg(", to_atomic_ptr_expression(ops[0]), ", ", to_expression(ops[3]), ");");
		flush_all_atomic_capable_variables();
		break;
	}
	case OpAtomicIIncrement:
	case OpAtomicIDecrement:
	{
		if (check_atomic_image(ops[2]))
			SPIRV_CROSS_THROW("Image atomics not yet implemented for OpenCL.");
		forced_temporaries.insert(ops[1]);
		auto &type = expression_type(ops[2]);
		bool unsigned_type = (type.basetype == SPIRType::UInt);
		const char *inc = (opcode == OpAtomicIIncrement && unsigned_type) ? "1u" :
		                  (opcode == OpAtomicIIncrement)                  ? "1" :
		                  unsigned_type                                   ? "uint(-1)" :
		                                                                    "-1";
		emit_op(ops[0], ops[1], join("atomic_add(", to_atomic_ptr_expression(ops[2]), ", ", inc, ")"), false);
		flush_all_atomic_capable_variables();
		break;
	}
	case OpAccessChain:
	case OpInBoundsAccessChain:
	{
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t base_id = ops[2];
		uint32_t length = instruction.length;

		// SSBO flattened to __global T*: rewrite [base, member_0, element_idx] → base[element_idx]
		if (flattened_buffer_vars.count(base_id) && length >= 5)
		{
			// ops[3] = struct member index (always 0 for single-member SSBO) — skip
			// ops[4] = element index within the runtime array
			auto expr = join(to_name(base_id), "[", to_expression(ops[4]), "]");
			auto &e = set<SPIRExpression>(result_id, std::move(expr), result_type, true);
			auto *backing_var = maybe_get_backing_variable(base_id);
			e.loaded_from = backing_var ? backing_var->self : ID(base_id);
			e.access_chain = true;
			forwarded_temporaries.insert(result_id);
			suppressed_usage_tracking.insert(result_id);
			for (uint32_t i = 2; i < length; i++)
				inherit_expression_dependencies(result_id, ops[i]);
			if (get<SPIRExpression>(result_id).expression_dependencies.empty())
				forwarded_temporaries.erase(result_id);
			break;
		}

		// Push constant expanded to scalar params: rewrite [p_var, member_idx] → scalar param name
		auto push_it = push_const_member_map.find(base_id);
		if (push_it != push_const_member_map.end() && length >= 4)
		{
			uint32_t mbr_idx = get<SPIRConstant>(ops[3]).scalar();
			auto name_it = push_it->second.find(mbr_idx);
			if (name_it != push_it->second.end())
			{
				auto &e = set<SPIRExpression>(result_id, name_it->second, result_type, false);
				e.loaded_from = base_id;
				e.access_chain = true;
				break;
			}
		}

		// Fall through to base class for all other access chains
		CompilerGLSL::emit_instruction(instruction);
		break;
	}

	default:
		CompilerGLSL::emit_instruction(instruction);
		break;
	}
}
