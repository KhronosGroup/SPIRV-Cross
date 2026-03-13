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

	// Rename WorkgroupSize spec constants to "spvWorkgroupSize" so that builtin_to_glsl
	// and the constant declaration both use the same name (Bug B fix for task #13).
	ir.for_each_typed_id<SPIRConstant>(
	    [&](uint32_t id, SPIRConstant &c)
	    {
		    if (c.specialization && has_decoration(c.self, DecorationBuiltIn) &&
		        BuiltIn(get_decoration(c.self, DecorationBuiltIn)) == BuiltInWorkgroupSize)
		    {
			    ir.set_name(id, "spvWorkgroupSize");
		    }
	    });

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
	backend.can_declare_arrays_inline = true;
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

void CompilerOpenCL::compute_kernel_resources()
{
	// Collect all SSBOs/BufferBlocks that get flattened to __global T* kernel parameters.
	flattened_buffer_vars.clear();
	flattened_var_type_decl.clear();

	ir.for_each_typed_id<SPIRVariable>(
	    [&](uint32_t var_id, SPIRVariable &var)
	    {
		    auto &type = get_variable_data_type(var);
		    if (var.storage == StorageClassStorageBuffer || has_decoration(type.self, DecorationBufferBlock))
		    {
			    Bitset flags = ir.get_buffer_block_flags(var);
			    bool is_readonly = flags.get(DecorationNonWritable);

			    // Compute the element type string for __global T* (same logic as entry_point_args).
			    string subtype;
			    if (type.basetype == SPIRType::Struct && type.member_types.size() == 1)
			    {
				    const auto &member0_type = get<SPIRType>(type.member_types.front());
				    subtype = type_to_glsl(member0_type);
			    }
			    else
			    {
				    subtype = type_to_glsl(type);
			    }

			    flattened_buffer_vars.insert(var_id);
			    flattened_var_type_decl[var_id] = join("__global ", is_readonly ? "const " : "", subtype, "* ");
		    }
	    });

	// For each non-entry function, find which flattened buffer vars it accesses (direct + transitive).
	func_flattened_args.clear();

	// First pass: direct accesses.
	unordered_map<uint32_t, unordered_set<uint32_t>> direct_accesses;
	ir.for_each_typed_id<SPIRFunction>(
	    [&](uint32_t func_id, SPIRFunction &func)
	    {
		    if (func_id == ir.default_entry_point)
			    return;

		    auto &accessed = direct_accesses[func_id];
		    for (auto block_id : func.blocks)
		    {
			    auto &block = get<SPIRBlock>(block_id);
			    for (auto &insn : block.ops)
			    {
				    const uint32_t *ops = stream(insn);
				    for (uint32_t i = 0; i < insn.length; i++)
				    {
					    if (flattened_buffer_vars.count(ops[i]))
						    accessed.insert(ops[i]);
				    }
			    }
		    }
	    });

	// Second pass: propagate transitively through function calls.
	bool changed = true;
	while (changed)
	{
		changed = false;
		ir.for_each_typed_id<SPIRFunction>(
		    [&](uint32_t func_id, SPIRFunction &func)
		    {
			    if (func_id == ir.default_entry_point)
				    return;
			    auto &my_accesses = direct_accesses[func_id];
			    for (auto block_id : func.blocks)
			    {
				    auto &block = get<SPIRBlock>(block_id);
				    for (auto &insn : block.ops)
				    {
					    if (static_cast<Op>(insn.op) == OpFunctionCall)
					    {
						    const uint32_t *ops = stream(insn);
						    uint32_t callee_id = ops[2];
						    if (callee_id != ir.default_entry_point && direct_accesses.count(callee_id))
						    {
							    for (auto var_id : direct_accesses[callee_id])
							    {
								    if (!my_accesses.count(var_id))
								    {
									    my_accesses.insert(var_id);
									    changed = true;
								    }
							    }
						    }
					    }
				    }
			    }
		    });
	}

	// Convert to sorted vectors (stable ordering by var ID).
	for (auto &kv : direct_accesses)
	{
		if (!kv.second.empty())
		{
			SmallVector<uint32_t> sorted;
			for (auto var_id : kv.second)
				sorted.push_back(var_id);
			std::sort(sorted.begin(), sorted.end());
			func_flattened_args[kv.first] = sorted;
		}
	}

	// Collect workgroup (StorageClassWorkgroup) and private global (StorageClassPrivate) variables
	// that are accessed in non-entry helper functions.  In OpenCL C 1.2 these cannot be at file
	// scope, so they must be declared in the kernel body and threaded as pointer parameters.
	workgroup_var_ptr_type.clear();
	workgroup_scalar_vars.clear();
	threaded_input_builtins.clear();

	unordered_set<uint32_t> threadable_vars;
	ir.for_each_typed_id<SPIRVariable>(
	    [&](uint32_t var_id, SPIRVariable &var)
	    {
		    if (var.storage == StorageClassWorkgroup || var.storage == StorageClassPrivate)
		    {
			    auto &type = get_variable_data_type(var);
			    bool is_array = !type.array.empty();
			    bool is_workgroup = (var.storage == StorageClassWorkgroup);

			    // Determine element/base type for the pointer parameter.
			    string elem_type_str;
			    if (is_array)
			    {
				    // Strip outermost array dimension to get element type.
				    auto elem_type = type;
				    elem_type.array.pop_back();
				    if (!elem_type.array_size_literal.empty())
					    elem_type.array_size_literal.pop_back();
				    elem_type_str = type_to_glsl(elem_type);
			    }
			    else
			    {
				    elem_type_str = type_to_glsl(type);
			    }

			    string addr_space = is_workgroup ? "__local " : "";
			    workgroup_var_ptr_type[var_id] = addr_space + elem_type_str + "*";
			    if (!is_array)
				    workgroup_scalar_vars.insert(var_id);

			    threadable_vars.insert(var_id);
		    }
		    // UBO (Uniform + Block) and PushConstant variables become kernel params.
		    // Helper functions can't see them, so they must be threaded as value params.
		    else if (var.storage == StorageClassPushConstant ||
		             (var.storage == StorageClassUniform && !is_hidden_variable(var) &&
		              has_decoration(get_variable_data_type(var).self, DecorationBlock) &&
		              !has_decoration(get_variable_data_type(var).self, DecorationBufferBlock)))
		    {
			    auto &type = get_variable_data_type(var);
			    if (type.basetype == SPIRType::Struct)
			    {
				    // Pass by value — no pointer, no #define trick needed.
				    workgroup_var_ptr_type[var_id] = type_to_glsl(type);
				    // NOT added to workgroup_scalar_vars (no #define needed — pass by value)
				    threadable_vars.insert(var_id);
			    }
		    }
		    // Input builtin variables (gl_GlobalInvocationID, etc.) accessed in non-entry functions
		    // need to be materialized as __private local variables and threaded as pointers.
		    else if (var.storage == StorageClassInput && has_decoration(var_id, DecorationBuiltIn))
		    {
			    auto &type = get_variable_data_type(var);
			    workgroup_var_ptr_type[var_id] = join("__private ", type_to_glsl(type), "*");
			    workgroup_scalar_vars.insert(var_id);
			    threadable_vars.insert(var_id);
			    auto builtin = BuiltIn(get_decoration(var_id, DecorationBuiltIn));
			    threaded_input_builtins[static_cast<uint32_t>(builtin)] = var_id;
		    }
	    });

	// Direct accesses of threadable vars in non-entry functions.
	unordered_map<uint32_t, unordered_set<uint32_t>> wg_direct;
	ir.for_each_typed_id<SPIRFunction>(
	    [&](uint32_t func_id, SPIRFunction &func)
	    {
		    if (func_id == ir.default_entry_point)
			    return;
		    auto &accessed = wg_direct[func_id];
		    for (auto block_id : func.blocks)
		    {
			    auto &block = get<SPIRBlock>(block_id);
			    for (auto &insn : block.ops)
			    {
				    const uint32_t *ops = stream(insn);
				    for (uint32_t i = 0; i < insn.length; i++)
				    {
					    if (threadable_vars.count(ops[i]))
						    accessed.insert(ops[i]);
				    }
			    }
		    }
	    });

	// Transitively propagate.
	changed = true;
	while (changed)
	{
		changed = false;
		ir.for_each_typed_id<SPIRFunction>(
		    [&](uint32_t func_id, SPIRFunction &func)
		    {
			    if (func_id == ir.default_entry_point)
				    return;
			    auto &my = wg_direct[func_id];
			    for (auto block_id : func.blocks)
			    {
				    auto &block = get<SPIRBlock>(block_id);
				    for (auto &insn : block.ops)
				    {
					    if (static_cast<Op>(insn.op) == OpFunctionCall)
					    {
						    const uint32_t *ops = stream(insn);
						    uint32_t callee_id = ops[2];
						    if (callee_id != ir.default_entry_point && wg_direct.count(callee_id))
						    {
							    for (auto var_id : wg_direct[callee_id])
							    {
								    if (!my.count(var_id))
								    {
									    my.insert(var_id);
									    changed = true;
								    }
							    }
						    }
					    }
				    }
			    }
		    });
	}

	// Convert to sorted vectors.
	func_workgroup_args.clear();
	for (auto &kv : wg_direct)
	{
		if (!kv.second.empty())
		{
			SmallVector<uint32_t> sorted;
			for (auto var_id : kv.second)
				sorted.push_back(var_id);
			std::sort(sorted.begin(), sorted.end());
			func_workgroup_args[kv.first] = sorted;
		}
	}
}

void CompilerOpenCL::emit_resources()
{
	replace_illegal_names();
	compute_kernel_resources();

	// Task #14: Polyfills for packHalf2x16 / unpackHalf2x16.
	// OpenCL C has vstore_half / vload_half which convert float ↔ float16 in memory.
	// These flags are set by emit_glsl_op and trigger a recompile on the next pass.
	if (needs_half_pack_polyfill)
	{
		statement("uint spvPackHalf2x16(float2 v) {");
		statement("    uint r;");
		statement("    vstore_half(v.x, 0, (__private half *)&r);");
		statement("    vstore_half(v.y, 1, (__private half *)&r);");
		statement("    return r;");
		statement("}");
		statement("");
	}
	if (needs_half_unpack_polyfill)
	{
		statement("float2 spvUnpackHalf2x16(uint u) {");
		statement("    const __private uint *p = &u;");
		statement("    return (float2)(vload_half(0, (const __private half *)p),");
		statement("                   vload_half(1, (const __private half *)p));");
		statement("}");
		statement("");
	}

	// Default sampler for combined image+sampler usage (OpenCL C requires file-scope const sampler_t).
	if (needs_default_sampler)
	{
		statement("const sampler_t spvDefaultSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | "
		          "CLK_FILTER_NEAREST;");
		statement("");
	}
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
		"imaginary",
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

	// Bug A fix (task #13): builtins are now inline calls in builtin_to_glsl, so we only need
	// spvWorkgroupSize when there is no spec-constant version (which lives at file scope).
	// Check whether there is a specialization constant decorated BuiltInWorkgroupSize.
	bool has_spec_workgroup_size = false;
	ir.for_each_typed_id<SPIRConstant>(
	    [&](uint32_t, const SPIRConstant &c)
	    {
		    if (c.specialization && has_decoration(c.self, DecorationBuiltIn) &&
		        BuiltIn(get_decoration(c.self, DecorationBuiltIn)) == BuiltInWorkgroupSize)
			    has_spec_workgroup_size = true;
	    });

	bool need_workgroup_size = active_input_builtins.get(BuiltInWorkgroupSize);
	if (!need_workgroup_size)
	{
		ir.for_each_typed_id<SPIRConstant>(
		    [&](uint32_t, const SPIRConstant &c)
		    {
			    if (has_decoration(c.self, DecorationBuiltIn) &&
			        BuiltIn(get_decoration(c.self, DecorationBuiltIn)) == BuiltInWorkgroupSize)
				    need_workgroup_size = true;
		    });
	}

	// Only emit the kernel-local spvWorkgroupSize variable when there is no file-scope spec constant.
	// When a spec constant exists, it is already emitted as a file-scope `constant uint3 spvWorkgroupSize`.
	if (need_workgroup_size && !has_spec_workgroup_size)
		statement("uint3 spvWorkgroupSize = (uint3)(get_local_size(0), get_local_size(1), get_local_size(2));");

	// Task #6: Emit __local declarations for workgroup (shared) variables.
	// In OpenCL C 1.x, __local variables must be declared inside kernel functions.
	ir.for_each_typed_id<SPIRVariable>(
	    [&](uint32_t, SPIRVariable &var)
	    {
		    if (var.storage == StorageClassWorkgroup && !is_hidden_variable(var))
		    {
			    auto &type = get_variable_data_type(var);
			    statement("__local ", variable_decl(type, to_name(var.self), var.self), ";");
		    }
	    });

	// Emit private global variables as kernel-local variables.
	// OpenCL C 1.x doesn't support __private file-scope variables, so we move them inside.
	for (auto global : global_variables)
	{
		auto &var = get<SPIRVariable>(global);
		if (var.storage == StorageClassPrivate && !is_hidden_variable(var, true))
		{
			add_local_variable_name(var.self);
			string initializer;
			if (var.initializer)
				initializer = join(" = ", to_expression(var.initializer));
			statement(CompilerGLSL::variable_decl(var), initializer, ";");
		}
	}

	// Materialize Input builtin variables as local variables.
	// In OpenCL C, builtins like get_global_id() are function calls, not variables.
	// When code needs variable pointers to these builtins (either threaded to non-entry
	// functions or used in pointer select within the entry point), we must create
	// actual __private local variables.
	//
	// Collect all builtins that need materialization: union of threaded and entry-point sets.
	unordered_map<uint32_t, uint32_t> builtins_to_materialize;
	for (auto &kv : threaded_input_builtins)
	{
		auto var_id = kv.second;
		bool actually_threaded = false;
		for (auto &fa : func_workgroup_args)
		{
			for (auto vid : fa.second)
			{
				if (vid == var_id)
				{
					actually_threaded = true;
					break;
				}
			}
			if (actually_threaded)
				break;
		}
		if (actually_threaded)
			builtins_to_materialize[kv.first] = kv.second;
	}
	for (auto &kv : entry_point_materialized_builtins)
		builtins_to_materialize[kv.first] = kv.second;

	// Use a guard flag so builtin_to_glsl returns the function call form (not the variable name).
	emitting_builtin_materialization = true;
	for (auto &kv : builtins_to_materialize)
	{
		auto var_id = kv.second;
		auto &type = get_variable_data_type(get<SPIRVariable>(var_id));
		auto builtin = BuiltIn(kv.first);
		string rhs = builtin_to_glsl(builtin, StorageClassInput);
		// Builtins return uint3 but the SPIR-V variable may be declared as int3.
		string var_type_str = type_to_glsl(type);
		if (type.basetype == SPIRType::Int && type.vecsize == 3)
			rhs = join("as_int3(", rhs, ")");
		else if (type.basetype == SPIRType::Int && type.vecsize == 2)
			rhs = join("as_int2(", rhs, ")");
		statement(var_type_str, " ", to_name(var_id), " = ", rhs, ";");
	}
	emitting_builtin_materialization = false;
}

string CompilerOpenCL::builtin_to_glsl(BuiltIn builtin, StorageClass storage)
{
	(void)storage;
	if (!emitting_builtin_materialization)
	{
		auto key = static_cast<uint32_t>(builtin);
		// If this builtin is threaded as a pointer param to non-entry functions,
		// return the variable name so the #define macro can dereference it.
		if (!processing_entry_point)
		{
			auto it = threaded_input_builtins.find(key);
			if (it != threaded_input_builtins.end())
				return to_name(it->second);
		}
		// If this builtin is materialized as a local variable in the entry point,
		// return the variable name so that &var_name gives a valid lvalue pointer.
		if (processing_entry_point)
		{
			auto it = entry_point_materialized_builtins.find(key);
			if (it != entry_point_materialized_builtins.end())
				return to_name(it->second);
		}
	}
	switch (builtin)
	{
	case BuiltInWorkgroupId:
		return "((uint3)(get_group_id(0), get_group_id(1), get_group_id(2)))";
	case BuiltInLocalInvocationId:
		return "((uint3)(get_local_id(0), get_local_id(1), get_local_id(2)))";
	case BuiltInGlobalInvocationId:
		return "((uint3)(get_global_id(0), get_global_id(1), get_global_id(2)))";
	case BuiltInNumWorkgroups:
		return "((uint3)(get_num_groups(0), get_num_groups(1), get_num_groups(2)))";
	case BuiltInWorkgroupSize:
		// spvWorkgroupSize is either a kernel-local variable or a file-scope spec constant;
		// both are named "spvWorkgroupSize" so returning this name works in both cases.
		return "spvWorkgroupSize";
	case BuiltInLocalInvocationIndex:
		return "((uint)(get_local_id(2) * get_local_size(0) * get_local_size(1) + get_local_id(1) * get_local_size(0) "
		       "+ get_local_id(0)))";
	case BuiltInGlobalSize:
		return "((uint3)(get_global_size(0), get_global_size(1), get_global_size(2)))";
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

string CompilerOpenCL::get_type_address_space(const SPIRType &type, uint32_t id, bool)
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
	case StorageClassInput:
		// Input builtins materialized as __private local variables.
		addr_space = "__private";
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
		// OpenCL C has no bool vector types (bool2/bool4 etc.). Map bool vectors to int.
		// Scalar bool is fine, but vector bool must be int (comparisons return intN in OpenCL).
		type_name = (type.vecsize > 1) ? "int" : "bool";
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
	(void)member;
	if (type.basetype != SPIRType::Image && type.basetype != SPIRType::SampledImage)
		return "";

	// Determine access qualifier.
	// SampledImage or sampled==1 means the image is used with a sampler (read-only).
	// sampled==2 means storage image (check decorations for read/write).
	const char *access;
	if (type.basetype == SPIRType::SampledImage || type.image.sampled == 1)
	{
		access = "read_only";
	}
	else
	{
		auto *var = (id != 0) ? maybe_get<SPIRVariable>(id) : nullptr;
		if (var)
		{
			bool non_readable = has_decoration(id, DecorationNonReadable);
			bool non_writable = has_decoration(id, DecorationNonWritable);
			if (non_readable)
				access = "write_only";
			else if (non_writable)
				access = "read_only";
			else if (opencl_options.opencl_version >= 200)
				access = "read_write";
			else
				access = "write_only"; // OCL 1.2: default to write_only when no decoration
		}
		else
		{
			access = "write_only";
		}
	}

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

// Mirrors CompilerMSL::get_physical_type_id_stride so that OpPtrAccessChain
// (used by VariablePointers) does not throw on the OpenCL backend.
uint32_t CompilerOpenCL::get_physical_type_id_stride(TypeID type_id) const
{
	auto &type = get<SPIRType>(type_id);

	// PhysicalStorageBuffer pointers are 64-bit (8 bytes).
	if (type.pointer && type.storage == StorageClassPhysicalStorageBuffer)
		return 8u;

	switch (type.basetype)
	{
	case SPIRType::Unknown:
	case SPIRType::Void:
	case SPIRType::AtomicCounter:
	case SPIRType::Image:
	case SPIRType::SampledImage:
	case SPIRType::Sampler:
		SPIRV_CROSS_THROW("Querying stride of opaque type.");

	default:
		break;
	}

	if (type.basetype == SPIRType::Struct)
		return (uint32_t)get_declared_struct_size(type);

	// Scalar / vector / matrix: width in bits → bytes, with vec3 padded to vec4.
	uint32_t vecsize = type.vecsize;
	if (vecsize == 3)
		vecsize = 4;
	return vecsize * type.columns * (type.width / 8u);
}

std::string CompilerOpenCL::type_to_glsl_constructor(const SPIRType &type)
{
	string ret = CompilerGLSL::type_to_glsl_constructor(type);
	if (!ret.empty())
		ret = join("(", ret, ")");
	return ret;
}

// OpenCL C requires cast syntax for replicated vector/matrix constants: (float4)(val) not float4(val).
// constant_expression is not virtual in GLSL, so we override it here to fix replicated composites.
std::string CompilerOpenCL::constant_expression(const SPIRConstant &c, bool inside_block_like_struct_scope,
                                                bool inside_struct_scope)
{
	auto &type = get<SPIRType>(c.constant_type);
	if (c.replicated && type.op != OpTypeArray)
	{
		auto sub_expr = to_expression(c.subconstants[0]);
		if (type.op == OpTypeMatrix)
		{
			// OpenCL C has no native matrix type; matrices are represented as their column vector type.
			// For a replicated matrix constant, just use the column value directly.
			return sub_expr;
		}
		else
		{
			// Vector replicate: (float4)(scalar)
			return join(type_to_glsl_constructor(type), "(", sub_expr, ")");
		}
	}
	return CompilerGLSL::constant_expression(c, inside_block_like_struct_scope, inside_struct_scope);
}

// OpenCL C requires cast syntax for vector construction: (float4)(1.0, 2.0, 3.0, 4.0)
// The GLSL base emits: float4(1.0, 2.0, 3.0, 4.0) which is invalid in OpenCL C.
std::string CompilerOpenCL::constant_expression_vector(const SPIRConstant &c, uint32_t vector)
{
	string res = CompilerGLSL::constant_expression_vector(c, vector);

	auto type = get<SPIRType>(c.constant_type);
	type.columns = 1;

	if (type.vecsize > 1)
	{
		// The base class emits: typename(args). OpenCL needs: (typename)(args).
		auto type_name = type_to_glsl(type);
		if (res.size() > type_name.size() + 1 && res.substr(0, type_name.size()) == type_name &&
		    res[type_name.size()] == '(')
		{
			res = "(" + type_name + ")(" + res.substr(type_name.size() + 1);
		}
	}

	return res;
}

// Override GLSLstd450 extension op handling for OpenCL-specific fixes.
void CompilerOpenCL::emit_glsl_op(uint32_t result_type, uint32_t result_id, uint32_t op, const uint32_t *args,
                                  uint32_t count)
{
	auto glsl_op = static_cast<GLSLstd450>(op);

	switch (glsl_op)
	{
	case GLSLstd450Modf:
	{
		// OpenCL modf takes a pointer for the integer part: modf(x, &iptr)
		register_call_out_argument(args[1]);
		forced_temporaries.insert(result_id);
		emit_op(result_type, result_id, join("modf(", to_expression(args[0]), ", &", to_expression(args[1]), ")"),
		        false);
		break;
	}

	case GLSLstd450ModfStruct:
	{
		// OpenCL modf: result._m0 = modf(x, &result._m1)
		auto &type = get<SPIRType>(result_type);
		emit_uninitialized_temporary_expression(result_type, result_id);
		statement(to_expression(result_id), ".", to_member_name(type, 0), " = modf(", to_expression(args[0]), ", &",
		          to_expression(result_id), ".", to_member_name(type, 1), ");");
		break;
	}

	// Task #14: Map GLSL half-precision pack/unpack to OpenCL polyfills.
	// On the first pass the polyfill may not exist yet; set the flag and force a recompile
	// so that emit_resources() will emit the helper functions before they are called.
	case GLSLstd450PackHalf2x16:
		if (!needs_half_pack_polyfill)
		{
			needs_half_pack_polyfill = true;
			force_recompile();
		}
		emit_unary_func_op(result_type, result_id, args[0], "spvPackHalf2x16");
		break;
	case GLSLstd450UnpackHalf2x16:
		if (!needs_half_unpack_polyfill)
		{
			needs_half_unpack_polyfill = true;
			force_recompile();
		}
		emit_unary_func_op(result_type, result_id, args[0], "spvUnpackHalf2x16");
		break;

	default:
		CompilerGLSL::emit_glsl_op(result_type, result_id, op, args, count);
		break;
	}
}

// Tasks #8: Map type-punning builtins to OpenCL as_TYPE() intrinsics.
// Also fix integral bitcasts: int4 → uint4 must use as_uint4(), not uint4().
std::string CompilerOpenCL::bitcast_glsl_op(const SPIRType &out_type, const SPIRType &in_type)
{
	// Same basetype: no-op
	if (out_type.basetype == in_type.basetype)
		return "";

	// All bitcasts (float↔int, int↔uint, half↔short, etc.) use as_TYPE() in OpenCL C.
	// type_to_glsl gives us the full type name including vector size (e.g. "float4", "uint").
	auto out_name = type_to_glsl(out_type);
	return "as_" + out_name;
}

// Task #7: In OpenCL C, atomic functions take a pointer argument.
// Access chain expressions (access_chain = true) may be C lvalues (e.g. ssbo->u32) → need &.
// But single-member flattened SSBOs emit the raw pointer itself (e.g. _48 is __global uint*)
// which doesn't need & even though it has access_chain=true.
std::string CompilerOpenCL::to_atomic_ptr_expression(uint32_t id)
{
	auto *e = maybe_get<SPIRExpression>(id);
	if (e && e->access_chain)
	{
		// For SSBO access chains, we need a pointer.
		// subscripted_deref_exprs marks access chains that are C values (e.g. _48[0]).
		// For those, we need & to get a pointer (which simplifies to the base pointer _48).
		// For non-subscripted access chains (pointer-typed), no & is needed.
		if (subscripted_deref_exprs.count(id))
			return "&(" + to_expression(id) + ")";
		return "&" + to_expression(id);
	}

	// Variable used directly as atomic operand (e.g. shared_u32, a workgroup variable).
	// In C this is an lvalue, so we need & to get a pointer.
	auto *var = maybe_get<SPIRVariable>(id);
	if (var && (var->storage == StorageClassWorkgroup || var->storage == StorageClassStorageBuffer ||
	            var->storage == StorageClassUniform))
	{
		return "&" + to_expression(id);
	}

	return to_expression(id);
}

// Task #3: In OpenCL C, pointer-to-struct member access uses -> instead of .
// ptr_chain_is_resolved == false means this is the first member access from the base.
std::string CompilerOpenCL::to_member_reference(uint32_t base, const SPIRType &type, uint32_t index,
                                                bool ptr_chain_is_resolved)
{
	if (!ptr_chain_is_resolved && !subscripted_deref_exprs.count(base))
	{
		const auto &base_type = expression_type(base);
		if (is_pointer(base_type))
		{
			StorageClass sc = base_type.storage;

			// Function/Private storage: use -> only for actual function pointer parameters
			// (out/inout params represented as __private T* in OpenCL C).
			// Regular local variables (OpVariable Function) are emitted as value types, use '.'.
			if (sc == StorageClassFunction || sc == StorageClassPrivate)
			{
				auto *var = maybe_get<SPIRVariable>(base);
				if (var && var->parameter != nullptr)
					return join("->", to_member_name(type, index));
			}

			// StorageBuffer SSBOs / __global pointers: always use ->.
			// Loaded values (OpLoad result) would have struct type, not pointer type,
			// so is_pointer() above is false — we only reach here with actual pointers.
			// Note: StorageClassWorkgroup is excluded because __local variables are emitted
			// as value types in OpenCL C, so member access uses '.'.
			if (sc == StorageClassStorageBuffer || sc == StorageClassCrossWorkgroup)
			{
				return join("->", to_member_name(type, index));
			}
			// StorageClassUniform (UBO): emitted by value in OpenCL — use '.'
		}
	}
	return join(".", to_member_name(type, index));
}

// Task #4: Emit typedef so structs can be referenced without the 'struct' keyword in OpenCL C.
void CompilerOpenCL::emit_struct(SPIRType &type)
{
	// Check whether the base class will actually emit this struct (it returns early for aliases).
	bool will_emit = type.type_alias == TypeID(0) ||
	                 has_extended_decoration(type.type_alias, SPIRVCrossDecorationBufferBlockRepacked);

	CompilerGLSL::emit_struct(type);

	if (will_emit)
	{
		auto name = to_name(type.self);
		statement("typedef struct ", name, " ", name, ";");
		statement("");
	}
}

// GCC workaround of lambdas calling protected funcs
std::string CompilerOpenCL::variable_decl(const SPIRType &type, const std::string &name, uint32_t id)
{
	return CompilerGLSL::variable_decl(type, name, id);
}

// OpenCL C does not support function overloading. If two functions share a name but differ in
// signature (different type hashes), the GLSL base class would allow both to keep the same name
// (since GLSL allows overloading). Override to always rename when a name is already taken.
void CompilerOpenCL::add_function_overload(const SPIRFunction &func)
{
	// Let the base class do its normal work first.
	CompilerGLSL::add_function_overload(func);

	// After base class runs, check if another function already has our (possibly newly assigned) name.
	// function_overloads maps name → set of type hashes. If this name maps to more than one hash,
	// the base class already handled the conflict. But if this is the SECOND function with the same
	// base name but different hash (GLSL would allow this), we still have a name collision in C.
	// Re-check: if more than one unique-hash entry shares our name, force a rename on this function.
	auto current_name = to_name(func.self);
	auto itr = function_overloads.find(current_name);
	if (itr != end(function_overloads) && itr->second.size() > 1)
	{
		// Two (or more) different signatures share this name. Rename this function.
		add_resource_name(func.self);
		function_overloads[to_name(func.self)].insert(0); // sentinel
	}
}

// For out/inout function parameters (pointer types in SPIR-V), we emit the function parameter as
// '__private T *param'. At call sites we must pass '&arg' (take address) so the pointer is valid.
std::string CompilerOpenCL::to_func_call_arg(const SPIRFunction::Parameter &callee_param, uint32_t id)
{
	// Check if the callee parameter expects a pointer (out/inout).
	auto &param_type = expression_type(callee_param.id);
	if (is_pointer(param_type) && param_type.storage == StorageClassFunction)
	{
		// Pass address of the argument variable.
		return join("&", to_expression(id));
	}

	// Flattened buffer vars are already pointers (__global T*).
	// Don't take their address when passing to functions expecting buffer pointers.
	if (flattened_buffer_vars.count(id))
	{
		auto &arg_type = expression_type(id);
		auto &callee_type = expression_type(callee_param.id);
		if (is_pointer(arg_type) && is_pointer(callee_type) &&
		    (callee_type.storage == StorageClassStorageBuffer || callee_type.storage == StorageClassUniform))
		{
			// The flattened var is __global T* but the callee expects __global struct_type*.
			// Cast to the expected type.
			auto callee_type_name = type_to_glsl(callee_type);
			return join("(", callee_type_name, ")", to_expression(id));
		}
	}

	return CompilerGLSL::to_func_call_arg(callee_param, id);
}

std::string CompilerOpenCL::entry_point_args(bool append_comma)
{
	// Note: flattened_buffer_vars is already populated by compute_kernel_resources() in emit_resources().
	// Only reset push_const_member_map here.
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
		    // Push constants: emit as struct value parameter.
		    if (var.storage == StorageClassPushConstant)
		    {
			    if (!ep_args.empty())
				    ep_args += ", ";
			    ep_args += join(type_to_glsl(type), " ", to_name(var_id));
		    }
		    else if (var.storage == StorageClassStorageBuffer || has_decoration(type.self, DecorationBufferBlock))
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
			    switch (type.basetype)
			    {
			    case SPIRType::Struct:
			    {
				    // UBO (Uniform + Block): emit as value parameter
				    if (var.storage == StorageClassUniform && has_decoration(type.self, DecorationBlock))
				    {
					    if (!ep_args.empty())
						    ep_args += ", ";
					    ep_args += join(type_to_glsl(type), " ", to_name(var_id));
				    }
				    break;
			    }
			    case SPIRType::Sampler:
				    if (!ep_args.empty())
					    ep_args += ", ";
				    ep_args += "sampler_t " + to_name(var_id);
				    break;
			    case SPIRType::Image:
			    case SPIRType::SampledImage:
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

	bool is_entry_point = (func.self == ir.default_entry_point);

	string decl;
	if (is_entry_point)
	{
		// Emit work group size attribute and __kernel qualifier for entry point
		emit_workgroup_size_attribute();
		decl += "__kernel void ";
		decl += get_inner_entry_point_name();
		processing_entry_point = true;
	}
	else
	{
		// Regular helper function
		auto &type = get<SPIRType>(func.return_type);
		decl += type_to_glsl(type) + " ";
		decl += to_name(func.self);
	}
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

		// OpenCL C has no in/out/inout qualifiers — skip direction prefix from argument_decl.
		auto &arg_type = expression_type(arg.id);
		decl += to_qualifiers_glsl(arg.id);
		decl += variable_decl(arg_type, to_name(arg.id), arg.id);

		if (&arg != &func.arguments.back())
			decl += ", ";

		// Hold a pointer to the parameter so we can invalidate the readonly field if needed.
		auto *var = maybe_get<SPIRVariable>(arg.id);
		if (var)
			var->parameter = &arg;
	}

	// For non-entry helper functions: append extra __global T* params for any flattened buffer
	// vars that this function (directly or transitively) accesses. This "threads" kernel resources
	// down through the call chain since OpenCL C has no global address space for buffer pointers.
	if (!is_entry_point)
	{
		bool first_resource = func.arguments.empty();

		auto it = func_flattened_args.find(func.self);
		if (it != func_flattened_args.end())
		{
			for (auto var_id : it->second)
			{
				auto type_it = flattened_var_type_decl.find(var_id);
				if (type_it != flattened_var_type_decl.end())
				{
					if (!first_resource)
						decl += ", ";
					first_resource = false;
					decl += type_it->second + to_name(var_id);
				}
			}
		}

		// Also thread workgroup/private global vars as pointer params.
		auto wg_it = func_workgroup_args.find(func.self);
		if (wg_it != func_workgroup_args.end())
		{
			for (auto var_id : wg_it->second)
			{
				auto type_it = workgroup_var_ptr_type.find(var_id);
				if (type_it != workgroup_var_ptr_type.end())
				{
					if (!first_resource)
						decl += ", ";
					first_resource = false;
					bool is_scalar = workgroup_scalar_vars.count(var_id) != 0;
					string param_name = is_scalar ? (to_name(var_id) + "_ptr") : to_name(var_id);
					decl += type_it->second + " " + param_name;
				}
			}
		}
	}

	decl += ")";
	statement(decl);
}

void CompilerOpenCL::append_global_func_args(const SPIRFunction &func, uint32_t index, SmallVector<string> &arglist)
{
	// First, call the base class to handle combined image samplers and other shadow args.
	CompilerGLSL::append_global_func_args(func, index, arglist);

	// Then append flattened kernel buffer vars threaded through helper functions.
	auto it = func_flattened_args.find(func.self);
	if (it != func_flattened_args.end())
	{
		for (auto var_id : it->second)
		{
			if (flattened_var_type_decl.count(var_id))
				arglist.push_back(to_name(var_id));
		}
	}

	// Thread workgroup/private global vars.
	auto wg_it = func_workgroup_args.find(func.self);
	if (wg_it != func_workgroup_args.end())
	{
		for (auto var_id : wg_it->second)
		{
			if (workgroup_var_ptr_type.count(var_id))
			{
				bool is_scalar = workgroup_scalar_vars.count(var_id) != 0;
				// Arrays decay to pointer; scalars need address-of.
				arglist.push_back(is_scalar ? ("&" + to_name(var_id)) : to_name(var_id));
			}
		}
	}
}

void CompilerOpenCL::emit_function_local_declarations(SPIRFunction &func)
{
	// For helper functions that access workgroup/private global scalar variables via pointer params:
	// emit #define var_name (*var_name_ptr) so that existing expressions (e.g. "u = 50;")
	// transparently dereference the pointer parameter.
	auto wg_it = func_workgroup_args.find(func.self);
	if (wg_it != func_workgroup_args.end())
	{
		for (auto var_id : wg_it->second)
		{
			if (workgroup_scalar_vars.count(var_id))
			{
				auto var_name = to_name(var_id);
				statement("#define ", var_name, " (*", var_name, "_ptr)");
			}
		}
	}
}

void CompilerOpenCL::emit_function_local_epilogue(SPIRFunction &func)
{
	auto wg_it = func_workgroup_args.find(func.self);
	if (wg_it != func_workgroup_args.end())
	{
		for (auto var_id : wg_it->second)
		{
			if (workgroup_scalar_vars.count(var_id))
				statement("#undef ", to_name(var_id));
		}
	}
}

void CompilerOpenCL::emit_specialization_constants_and_structs()
{
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
			else
			{
				// Non-specialization constant arrays need to be declared at file scope
				// because OpenCL C does not support arrays as value types (can't inline them).
				auto &type = get<SPIRType>(c.constant_type);
				if (is_array(type))
				{
					add_resource_name(c.self);
					auto name = to_name(c.self);
					statement("constant ", variable_decl(type, name, c.self), " = ", constant_expression(c), ";");
					emitted = true;
				}
			}
		}
		else if (id.get_type() == TypeConstantOp)
		{
			// OpSpecConstantOp results are derived from spec constants via arithmetic ops.
			// In OpenCL C, "constant T name = expr;" requires a compile-time constant initializer,
			// but expressions like "as_uint(spec_const)" (function calls) and "vec.x" (component
			// access on a constant variable) are NOT constant expressions in OpenCL C.
			// Emit as a #define macro so the expression is inlined at each use site (evaluated at
			// runtime when used in a function body, which is the only valid use location).
			auto &c = id.get<SPIRConstantOp>();
			add_resource_name(c.self);
			auto name = to_name(c.self);
			statement("#define ", name, " (", constant_op_expression(c), ")");
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
			// Emit a zero-initialized constant so composite uses of this undef can compile.
			// OpUndef values are semantically undefined; zero is a safe placeholder.
			add_resource_name(undef.self);
			auto name = to_name(undef.self);
			string zero_expr;
			if (type.basetype == SPIRType::Struct)
				zero_expr = join("(", type_to_glsl(type), "){ 0 }");
			else if (type.vecsize > 1)
				zero_expr = join(type_to_glsl_constructor(type), "(0)");
			else
				zero_expr = "0";
			statement("constant ", type_to_glsl(type), " ", name, " = ", zero_expr, ";");
			emitted = true;
		}
	}

	if (emitted)
		statement("");
}

bool CompilerOpenCL::emit_array_copy(const char *expr, uint32_t lhs_id, uint32_t rhs_id, StorageClass, StorageClass)
{
	// OpenCL C does not support array assignment (array_is_value_type = false).
	// Emit element-by-element copy using a for loop.
	string lhs;
	if (expr)
		lhs = expr;
	else
		lhs = to_expression(lhs_id);

	auto rhs_expr = to_expression(rhs_id);
	auto &type = expression_type(rhs_id);

	// Get the array size
	if (!is_array(type) || type.array.empty())
	{
		// Not actually an array; fall back to simple assignment
		statement(lhs, " = ", rhs_expr, ";");
		return true;
	}

	uint32_t array_size = type.array.back();
	if (!type.array_size_literal.back())
	{
		// Spec constant sized array — use simple assignment and hope for the best
		statement(lhs, " = ", rhs_expr, ";");
		return true;
	}

	// Emit element-by-element copy
	for (uint32_t i = 0; i < array_size; i++)
		statement(lhs, "[", i, "] = ", rhs_expr, "[", i, "];");

	return true;
}

void CompilerOpenCL::emit_instruction(const Instruction &instruction)
{
	auto ops = stream(instruction);
	auto opcode = static_cast<Op>(instruction.op);

	// Task #5: Handle barrier/fence ops with OpenCL C equivalents.
	// Returns the CLK_*_MEM_FENCE flags string for the given memory semantics.
	auto opencl_mem_fence_flags = [](uint32_t semantics) -> string
	{
		// We only care about workgroup and uniform/image memory.
		bool local = (semantics & MemorySemanticsWorkgroupMemoryMask) != 0;
		bool global = (semantics & (MemorySemanticsUniformMemoryMask | MemorySemanticsImageMemoryMask |
		                            MemorySemanticsCrossWorkgroupMemoryMask)) != 0;
		if (local && global)
			return "CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE";
		else if (local)
			return "CLK_LOCAL_MEM_FENCE";
		else if (global)
			return "CLK_GLOBAL_MEM_FENCE";
		else
			return "CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE"; // default: fence everything
	};

	// Map buffer atomics to OpenCL C names (atomic_add, atomic_sub, etc.)
	auto opencl_atomic = [this, ops](const char *opencl_op)
	{
		if (check_atomic_image(ops[2]))
			SPIRV_CROSS_THROW("Image atomics not yet implemented for OpenCL.");
		emit_atomic_func_op(ops[0], ops[1], ops[2], ops[5], opencl_op);
	};

	// Helper: cast integer operand to the target signedness using as_TYPE() for OpenCL C.
	// OpenCL C forbids implicit conversion between integer vector types of different signedness.
	auto cast_int_for_icmp = [this](uint32_t id, bool want_signed) -> string
	{
		auto &t = expression_type(id);
		if (type_is_integral(t))
		{
			bool is_signed = t.basetype == SPIRType::SByte || t.basetype == SPIRType::Short ||
			                 t.basetype == SPIRType::Int || t.basetype == SPIRType::Int64;
			if (is_signed != want_signed)
			{
				auto target_type = t;
				target_type.basetype = want_signed ? to_signed_basetype(t.width) : to_unsigned_basetype(t.width);
				return join("as_", type_to_glsl(target_type), "(", to_expression(id), ")");
			}
		}
		return to_enclosed_expression(id);
	};

	// Helper: returns true if 'id' is a function parameter that carries a pointer type.
	// In GLSL, out/inout params are emitted as 'out T', but in OpenCL C they are '__private T *'.
	// Loads and stores through such params need explicit pointer dereference.
	auto is_func_ptr_param = [&](uint32_t id) -> bool
	{
		auto *var = maybe_get<SPIRVariable>(id);
		return var && var->parameter != nullptr && is_pointer(expression_type(id)) &&
		       expression_type(id).storage == StorageClassFunction;
	};

	switch (opcode)
	{
	// OpLoad from an out/inout function parameter pointer: dereference.
	case OpLoad:
	{
		uint32_t ptr = ops[2];
		if (is_func_ptr_param(ptr))
		{
			uint32_t result_type = ops[0];
			uint32_t result_id = ops[1];
			emit_op(result_type, result_id, join("(*", to_name(ptr), ")"), true);
			inherit_expression_dependencies(result_id, ptr);
			break;
		}
		// Loading the whole struct from a flattened buffer pointer (or OpCopyObject of one)
		// needs dereference. Only applies to direct loads from the variable, not access chains.
		if (flattened_buffer_vars.count(ptr) ||
		    (maybe_get<SPIRExpression>(ptr) && !get<SPIRExpression>(ptr).access_chain &&
		     maybe_get_backing_variable(ptr) && flattened_buffer_vars.count(maybe_get_backing_variable(ptr)->self)))
		{
			uint32_t result_type = ops[0];
			uint32_t result_id = ops[1];
			emit_op(result_type, result_id, join("(*", to_expression(ptr), ")"), true);
			inherit_expression_dependencies(result_id, ptr);
			break;
		}
		CompilerGLSL::emit_instruction(instruction);
		break;
	}

	// OpStore to an out/inout function parameter pointer or flattened buffer: dereference.
	case OpStore:
	{
		uint32_t ptr = ops[0];
		if (is_func_ptr_param(ptr))
		{
			statement("*", to_name(ptr), " = ", to_expression(ops[1]), ";");
			register_write(ptr);
			break;
		}
		// Flattened buffer vars are __global T* pointers; storing to them needs dereference.
		if (flattened_buffer_vars.count(ptr))
		{
			statement("*", to_name(ptr), " = ", to_expression(ops[1]), ";");
			register_write(ptr);
			break;
		}
		CompilerGLSL::emit_instruction(instruction);
		break;
	}

	// OpenCL C uses fmod() instead of GLSL's mod().
	case OpFMod:
		emit_binary_func_op(ops[0], ops[1], ops[2], ops[3], "fmod");
		break;

	// SPV_KHR_fma: fused multiply-add — OpenCL C has a native fma() builtin.
	case OpFmaKHR:
		emit_trinary_func_op(ops[0], ops[1], ops[2], ops[3], ops[4], "fma");
		break;

	// SPV_KHR_expect_assume: no equivalent in OpenCL C.
	// OpAssumeTrueKHR: hint that a condition is always true — emit nothing.
	case OpAssumeTrueKHR:
		break;
	// OpExpectKHR: hint that value has an expected value — emit the value unchanged.
	case OpExpectKHR:
		emit_op(ops[0], ops[1], to_expression(ops[2]), should_forward(ops[2]));
		inherit_expression_dependencies(ops[1], ops[2]);
		break;

	// Type conversion ops: use OpenCL C convert_TYPE() for numeric value conversions.
	// The GLSL base class emits (TYPE)(expr) which in OpenCL C is a bitcast for vector types,
	// not a value conversion. convert_TYPE() is correct for both scalar and vector operands.
	case OpConvertUToF:
	case OpConvertSToF:
	case OpConvertFToU:
	case OpConvertFToS:
	case OpFConvert:
	case OpUConvert:
	case OpSConvert:
	{
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		auto convert_func = join("convert_", type_to_glsl(get<SPIRType>(result_type)));
		emit_unary_func_op(result_type, result_id, ops[2], convert_func.c_str());
		break;
	}

	// OpOuterProduct: no OpenCL builtin and no native matrix type.
	// The result matrix type is represented as its column vector type in OpenCL C.
	// Emit only the first column (col_vec * row_vec.x).
	case OpOuterProduct:
	{
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t col_vec = ops[2]; // column vector
		uint32_t row_vec = ops[3]; // row vector
		auto &row_type = expression_type(row_vec);

		// First column of the outer product: col_vec * row_vec.x
		string first_row_elem =
		    row_type.vecsize > 1 ? join(to_expression(row_vec), ".", index_to_swizzle(0)) : to_expression(row_vec);
		string expr = join(to_expression(col_vec), " * ", first_row_elem);
		emit_op(result_type, result_id, expr, should_forward(col_vec) && should_forward(row_vec));
		inherit_expression_dependencies(result_id, col_vec);
		inherit_expression_dependencies(result_id, row_vec);
		break;
	}

	// Task #9: Map GLSL vector comparison functions to OpenCL C operators.
	// GLSL: lessThan(a, b) → OpenCL: (a < b)
	// For integer ops, add explicit as_TYPE() casts so operands match the comparison signedness.
	// OpenCL C does not allow implicit conversion between signed and unsigned vector types.
#define SPIRV_OPENCL_ICMP_OP(signed_op, unsigned_op, float_op_1, float_op_2, op_str) \
	case signed_op:                                                                  \
	{                                                                                \
		auto left = cast_int_for_icmp(ops[2], true);                                 \
		auto right = cast_int_for_icmp(ops[3], true);                                \
		bool fwd = should_forward(ops[2]) && should_forward(ops[3]);                 \
		emit_op(ops[0], ops[1], join(left, " " op_str " ", right), fwd);             \
		inherit_expression_dependencies(ops[1], ops[2]);                             \
		inherit_expression_dependencies(ops[1], ops[3]);                             \
		break;                                                                       \
	}                                                                                \
	case unsigned_op:                                                                \
	{                                                                                \
		auto left = cast_int_for_icmp(ops[2], false);                                \
		auto right = cast_int_for_icmp(ops[3], false);                               \
		bool fwd = should_forward(ops[2]) && should_forward(ops[3]);                 \
		emit_op(ops[0], ops[1], join(left, " " op_str " ", right), fwd);             \
		inherit_expression_dependencies(ops[1], ops[2]);                             \
		inherit_expression_dependencies(ops[1], ops[3]);                             \
		break;                                                                       \
	}                                                                                \
	case float_op_1:                                                                 \
	case float_op_2:                                                                 \
		emit_binary_op(ops[0], ops[1], ops[2], ops[3], op_str);                      \
		break;

		SPIRV_OPENCL_ICMP_OP(OpSLessThan, OpULessThan, OpFOrdLessThan, OpFUnordLessThan, "<")
		SPIRV_OPENCL_ICMP_OP(OpSLessThanEqual, OpULessThanEqual, OpFOrdLessThanEqual, OpFUnordLessThanEqual, "<=")
		SPIRV_OPENCL_ICMP_OP(OpSGreaterThan, OpUGreaterThan, OpFOrdGreaterThan, OpFUnordGreaterThan, ">")
		SPIRV_OPENCL_ICMP_OP(OpSGreaterThanEqual, OpUGreaterThanEqual, OpFOrdGreaterThanEqual, OpFUnordGreaterThanEqual,
		                     ">=")
#undef SPIRV_OPENCL_ICMP_OP

	case OpIEqual:
	case OpFOrdEqual:
	case OpFUnordEqual:
	case OpLogicalEqual:
		emit_binary_op(ops[0], ops[1], ops[2], ops[3], "==");
		break;
	case OpINotEqual:
	case OpFOrdNotEqual:
	case OpFUnordNotEqual:
	case OpLogicalNotEqual:
		emit_binary_op(ops[0], ops[1], ops[2], ops[3], "!=");
		break;

	case OpControlBarrier:
	{
		// ops[0]=execution_scope, ops[1]=memory_scope, ops[2]=semantics
		uint32_t semantics = evaluate_constant_u32(ops[2]);
		semantics = mask_relevant_memory_semantics(semantics);

		flush_control_dependent_expressions(current_emitting_block->self);
		flush_all_active_variables();

		// Emit memory fence before the execution barrier if needed
		string fence_flags = opencl_mem_fence_flags(semantics);
		if (semantics != 0)
		{
			if (opencl_options.supports_opencl_version(2, 0))
				statement("work_group_barrier(", fence_flags, ");");
			else
				statement("barrier(", fence_flags, ");");
		}
		else
		{
			// Execution barrier with default local fence
			if (opencl_options.supports_opencl_version(2, 0))
				statement("work_group_barrier(CLK_LOCAL_MEM_FENCE);");
			else
				statement("barrier(CLK_LOCAL_MEM_FENCE);");
		}
		break;
	}

	case OpMemoryBarrier:
	{
		// ops[0]=memory_scope, ops[1]=semantics
		uint32_t semantics = evaluate_constant_u32(ops[1]);
		semantics = mask_relevant_memory_semantics(semantics);

		flush_control_dependent_expressions(current_emitting_block->self);
		flush_all_active_variables();

		if (semantics != 0)
		{
			string fence_flags = opencl_mem_fence_flags(semantics);
			statement("mem_fence(", fence_flags, ");");
		}
		break;
	}

	case OpAtomicExchange:
		if (check_atomic_image(ops[2]))
			SPIRV_CROSS_THROW("Image atomics not yet implemented for OpenCL.");
		emit_atomic_func_op(ops[0], ops[1], ops[2], ops[5], "atomic_xchg");
		break;
	case OpAtomicCompareExchange:
		if (check_atomic_image(ops[2]))
			SPIRV_CROSS_THROW("Image atomics not yet implemented for OpenCL.");
		// OpenCL atomic_cmpxchg(&ptr, expected, desired)
		forced_temporaries.insert(ops[1]);
		emit_op(ops[0], ops[1],
		        join("atomic_cmpxchg(", to_atomic_ptr_expression(ops[2]), ", ", to_unpacked_expression(ops[7]), ", ",
		             to_unpacked_expression(ops[6]), ")"),
		        false);
		flush_all_atomic_capable_variables();
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
		                  unsigned_type                                   ? "(uint)(-1)" :
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

		if (flattened_buffer_vars.count(base_id))
		{
			// Handle SSBO access chains for buffer vars.
			// Get the original SPIR-V struct type to determine single vs multi-member.
			auto *base_var = maybe_get<SPIRVariable>(base_id);
			const SPIRType *struct_type = base_var ? &get_variable_data_type(*base_var) : nullptr;
			bool is_single_member = struct_type && struct_type->member_types.size() == 1;

			string expr;
			bool handled = false;

			bool is_subscript_deref = false; // result is a C value (subscripted), not a pointer

			if (length >= 5 && is_single_member)
			{
				// Single-member SSBO flattened to __global T*: ptr[element_idx][.member]*
				// ops[3] = struct member index (always 0, skip)
				// ops[4] = element index within the runtime array
				// ops[5+] = optional sub-member indices
				expr = join(to_name(base_id), "[", to_expression(ops[4]), "]");
				is_subscript_deref = true;
				// Walk additional sub-member indices using type info.
				if (length >= 6 && struct_type)
				{
					const SPIRType *cur_type = &get<SPIRType>(struct_type->member_types[0]);
					for (uint32_t i = 5; i < length; i++)
					{
						if (cur_type->basetype == SPIRType::Struct)
						{
							uint32_t mbr_idx = get<SPIRConstant>(ops[i]).scalar();
							expr += join(".", to_member_name(*cur_type, mbr_idx));
							cur_type = &get<SPIRType>(cur_type->member_types[mbr_idx]);
						}
						else
						{
							// Array or other type - fall back to index notation
							expr += join("[", to_expression(ops[i]), "]");
						}
					}
				}
				handled = true;
			}
			else if (length == 5 && !is_single_member && struct_type)
			{
				// Multi-member SSBO: ptr->member_name[element_idx]
				// ops[3] = member index, ops[4] = array element index
				uint32_t mbr_idx = get<SPIRConstant>(ops[3]).scalar();
				auto mbr_name = to_member_name(*struct_type, mbr_idx);
				expr = join(to_name(base_id), "->", mbr_name, "[", to_expression(ops[4]), "]");
				is_subscript_deref = true;
				handled = true;
			}
			else if (length == 4 && is_single_member)
			{
				// Single-member SSBO flattened to T*: accessing the one member gives element 0.
				expr = join(to_name(base_id), "[0]");
				is_subscript_deref = true;
				handled = true;
			}
			else if (length == 4 && !is_single_member && struct_type)
			{
				// Multi-member SSBO: ptr->member_name (lvalue, not address-of)
				uint32_t mbr_idx = get<SPIRConstant>(ops[3]).scalar();
				auto mbr_name = to_member_name(*struct_type, mbr_idx);
				expr = join(to_name(base_id), "->", mbr_name);
				is_subscript_deref = true; // result is a struct value (accessed through ->), use . for children
				handled = true;
			}

			if (handled)
			{
				auto &e = set<SPIRExpression>(result_id, std::move(expr), result_type, true);
				auto *backing_var = maybe_get_backing_variable(base_id);
				e.loaded_from = backing_var ? backing_var->self : ID(base_id);
				e.access_chain = true;
				if (is_subscript_deref)
					subscripted_deref_exprs.insert(result_id);
				forwarded_temporaries.insert(result_id);
				suppressed_usage_tracking.insert(result_id);
				for (uint32_t i = 2; i < length; i++)
					inherit_expression_dependencies(result_id, ops[i]);
				if (get<SPIRExpression>(result_id).expression_dependencies.empty())
					forwarded_temporaries.erase(result_id);
				break;
			}
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

		// If the base expression is already a subscripted/dereferenced C value (e.g. ptr[idx]),
		// the result of further member access is also a C value. Propagate the tracking so
		// to_member_reference continues to use '.' instead of '->'.
		bool base_is_deref = subscripted_deref_exprs.count(base_id) != 0;

		// Fall through to base class for all other access chains
		CompilerGLSL::emit_instruction(instruction);

		if (base_is_deref)
			subscripted_deref_exprs.insert(result_id);
		break;
	}

	case OpSelect:
	{
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t condition = ops[2];
		uint32_t true_val = ops[3];
		uint32_t false_val = ops[4];
		auto &cond_type = expression_type(condition);
		auto &res_type = get<SPIRType>(result_type);

		if (res_type.pointer)
		{
			// If result is a pointer, the pointed-to values may be written through it.
			register_write(true_val);
			register_write(false_val);

			// Pointer select in OpenCL C: need special handling because
			// flattened buffer vars are already pointers (no & needed),
			// Input builtins are function calls (can't take &), and
			// null pointer constants need to be emitted as NULL.
			auto make_ptr_expr = [&](uint32_t val) -> string
			{
				// Null pointer constant
				if (ir.ids[val].get_type() == TypeConstant)
					return "NULL";
				// Flattened buffer var — already a pointer value
				if (flattened_buffer_vars.count(val))
					return to_enclosed_expression(val);
				// Input builtin variable — materialize as local var and take address
				auto *var = maybe_get<SPIRVariable>(val);
				if (var && var->storage == StorageClassInput && has_decoration(val, DecorationBuiltIn))
				{
					if (processing_entry_point)
					{
						// Entry point: materialize the builtin as a local variable.
						auto builtin = BuiltIn(get_decoration(val, DecorationBuiltIn));
						auto key = static_cast<uint32_t>(builtin);
						if (entry_point_materialized_builtins.emplace(key, val).second)
							force_recompile();
						return "&" + to_name(val);
					}
					else
					{
						// Non-entry function: builtins are threaded via #define trick,
						// so to_name(val) is a valid lvalue via the macro.
						return "&" + to_name(val);
					}
				}
				// Default: use base class pointer expression
				return to_enclosed_pointer_expression(val);
			};

			auto expr = join(to_enclosed_expression(condition), " ? ", make_ptr_expr(true_val), " : ",
			                 make_ptr_expr(false_val));
			emit_op(result_type, result_id, expr,
			        should_forward(condition) && should_forward(true_val) && should_forward(false_val));
			inherit_expression_dependencies(result_id, condition);
			inherit_expression_dependencies(result_id, true_val);
			inherit_expression_dependencies(result_id, false_val);
		}
		else if (cond_type.vecsize > 1 && cond_type.basetype == SPIRType::Boolean && res_type.vecsize > 1)
		{
			// In OpenCL C, vector ternary and bool-to-int casts don't work like GLSL.
			// Use OpenCL's select(false_val, true_val, cond) instead.
			emit_trinary_func_op(result_type, result_id, false_val, true_val, condition, "select");
		}
		else
		{
			CompilerGLSL::emit_instruction(instruction);
		}
		break;
	}

	case OpCompositeConstructReplicateEXT:
	{
		// GLSL base uses type(value) for vector splat, but OpenCL C needs (type)(value).
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		auto &type = get<SPIRType>(result_type);
		if (type.op == OpTypeMatrix)
		{
			// OpenCL C has no native matrix type; matrices are represented as their column vector type.
			// Just use the sub-value directly (representing the first/only column).
			emit_op(result_type, result_id, to_expression(ops[2]), should_forward(ops[2]));
			inherit_expression_dependencies(result_id, ops[2]);
		}
		else if (type.op != OpTypeArray && type.vecsize > 1)
		{
			// Vector replicate: (float4)(scalar_value)
			auto rhs = join(type_to_glsl_constructor(type), "(", to_expression(ops[2]), ")");
			emit_op(result_type, result_id, rhs, true);
			inherit_expression_dependencies(result_id, ops[2]);
		}
		else
		{
			// Array replicate: delegate to base
			CompilerGLSL::emit_instruction(instruction);
		}
		break;
	}

	// Map GLSL imulExtended/umulExtended to OpenCL C mul_hi + multiply.
	case OpUMulExtended:
	case OpSMulExtended:
	{
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t op0 = ops[2];
		uint32_t op1 = ops[3];
		auto &type = get<SPIRType>(result_type);
		emit_uninitialized_temporary_expression(result_type, result_id);
		// _m0 = low bits (a * b), _m1 = high bits (mul_hi(a, b))
		statement(to_expression(result_id), ".", to_member_name(type, 0), " = ", to_expression(op0), " * ",
		          to_expression(op1), ";");
		statement(to_expression(result_id), ".", to_member_name(type, 1), " = mul_hi(", to_expression(op0), ", ",
		          to_expression(op1), ");");
		break;
	}

	case OpQuantizeToF16:
	{
		// GLSL emits unpackHalf2x16/packHalf2x16 which aren't OpenCL builtins.
		// Use our polyfill functions instead.
		if (!needs_half_pack_polyfill || !needs_half_unpack_polyfill)
		{
			needs_half_pack_polyfill = true;
			needs_half_unpack_polyfill = true;
			force_recompile();
		}
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t arg = ops[2];
		string op;
		auto &type = get<SPIRType>(result_type);
		switch (type.vecsize)
		{
		case 1:
			op = join("spvUnpackHalf2x16(spvPackHalf2x16((float2)(", to_expression(arg), ", 0.0f))).x");
			break;
		case 2:
			op = join("spvUnpackHalf2x16(spvPackHalf2x16(", to_expression(arg), "))");
			break;
		case 3:
		{
			auto op0 = join("spvUnpackHalf2x16(spvPackHalf2x16(", to_expression(arg), ".xy))");
			auto op1 = join("spvUnpackHalf2x16(spvPackHalf2x16((float2)(", to_expression(arg), ".z, 0.0f))).x");
			op = join("(float3)(", op0, ", ", op1, ")");
			break;
		}
		case 4:
		{
			auto op0 = join("spvUnpackHalf2x16(spvPackHalf2x16(", to_expression(arg), ".xy))");
			auto op1 = join("spvUnpackHalf2x16(spvPackHalf2x16(", to_expression(arg), ".zw))");
			op = join("(float4)(", op0, ", ", op1, ")");
			break;
		}
		default:
			SPIRV_CROSS_THROW("Illegal argument to OpQuantizeToF16.");
		}
		emit_op(result_type, id, op, should_forward(arg));
		inherit_expression_dependencies(id, arg);
		break;
	}

	// Map OpImageSample* (texture sampling) to OpenCL read_image* with sampler.
	case OpImageSampleExplicitLod:
	case OpImageSampleImplicitLod:
	{
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t combined_id = ops[2];
		uint32_t coord_id = ops[3];

		if (!needs_default_sampler)
		{
			needs_default_sampler = true;
			force_recompile();
		}

		auto &result_spirtype = get<SPIRType>(result_type);
		const char *read_func;
		switch (result_spirtype.basetype)
		{
		case SPIRType::UInt:
			read_func = "read_imageui";
			break;
		case SPIRType::Int:
			read_func = "read_imagei";
			break;
		default:
			read_func = "read_imagef";
			break;
		}

		// For combined image+sampler, get the underlying image expression.
		auto img_expr = to_expression(combined_id);

		// Sampler-based read_image* takes float coordinates.
		auto &coord_type = expression_type(coord_id);
		string coord_expr;
		if (coord_type.basetype == SPIRType::Float)
			coord_expr = to_expression(coord_id);
		else
			coord_expr = join("convert_float", coord_type.vecsize > 1 ? to_string(coord_type.vecsize) : "", "(",
			                  to_expression(coord_id), ")");

		auto raw_expr = join(read_func, "(", img_expr, ", spvDefaultSampler, ", coord_expr, ")");
		auto swizzled = remap_swizzle(result_spirtype, 4, raw_expr);

		bool forward = should_forward(combined_id) && should_forward(coord_id);
		emit_op(result_type, result_id, swizzled, forward);
		inherit_expression_dependencies(result_id, combined_id);
		inherit_expression_dependencies(result_id, coord_id);
		break;
	}

	// Task #10: Map image read/write/query ops to OpenCL C equivalents.
	case OpImageRead:
	{
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t image_id = ops[2];
		uint32_t coord_id = ops[3];

		auto &img_type = expression_type(image_id);
		// SubpassData is not supported; fall through to base class.
		if (img_type.image.dim == DimSubpassData)
		{
			CompilerGLSL::emit_instruction(instruction);
			break;
		}

		auto &result_spirtype = get<SPIRType>(result_type);
		const char *read_func;
		switch (result_spirtype.basetype)
		{
		case SPIRType::UInt:
			read_func = "read_imageui";
			break;
		case SPIRType::Int:
			read_func = "read_imagei";
			break;
		default:
			read_func = "read_imagef";
			break;
		}

		// Convert coordinate to int.
		auto coord_type = expression_type(coord_id);
		coord_type.basetype = SPIRType::Int;
		auto coord_expr = bitcast_expression(coord_type, expression_type(coord_id).basetype, to_expression(coord_id));

		// OpenCL read functions always return a vec4; swizzle down to the required vecsize.
		auto raw_expr = join(read_func, "(", to_expression(image_id), ", ", coord_expr, ")");
		// Build a temporary vec4 type for the result of the read function.
		SPIRType vec4_type = result_spirtype;
		vec4_type.vecsize = 4;
		auto swizzled = remap_swizzle(result_spirtype, 4, raw_expr);

		bool forward = should_forward(image_id) && should_forward(coord_id);
		emit_op(result_type, result_id, swizzled, forward);
		inherit_expression_dependencies(result_id, image_id);
		inherit_expression_dependencies(result_id, coord_id);
		break;
	}

	case OpImageWrite:
	{
		uint32_t image_id = ops[0];
		uint32_t coord_id = ops[1];
		uint32_t texel_id = ops[2];

		// Unset NonWritable so the variable can be written (mirroring GLSL backend).
		auto *image_var = maybe_get_backing_variable(image_id);
		if (image_var)
			unset_decoration(image_var->self, DecorationNonWritable);

		auto &value_type = expression_type(texel_id);
		const char *write_func;
		switch (value_type.basetype)
		{
		case SPIRType::UInt:
			write_func = "write_imageui";
			break;
		case SPIRType::Int:
			write_func = "write_imagei";
			break;
		default:
			write_func = "write_imagef";
			break;
		}

		// Convert coordinate to int.
		auto coord_type = expression_type(coord_id);
		coord_type.basetype = SPIRType::Int;
		auto coord_expr = bitcast_expression(coord_type, expression_type(coord_id).basetype, to_expression(coord_id));

		// OpenCL write functions expect a vec4 texel; expand if necessary.
		// Use (vec4_type)(expr) C-style cast which is valid for scalar-to-vector broadcast.
		SPIRType vec4_type = value_type;
		vec4_type.vecsize = 4;
		string texel_raw = to_expression(texel_id);
		string texel_expr;
		if (value_type.vecsize == 4)
			texel_expr = texel_raw;
		else
			texel_expr = join("(", type_to_glsl(vec4_type), ")(", texel_raw, ")");

		statement(write_func, "(", to_expression(image_id), ", ", coord_expr, ", ", texel_expr, ");");

		if (image_var && variable_storage_is_aliased(*image_var))
			flush_all_aliased_variables();
		break;
	}

	case OpImageQuerySize:
	{
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t image_id = ops[2];

		auto &img_type = expression_type(image_id);
		if (img_type.basetype != SPIRType::Image)
		{
			CompilerGLSL::emit_instruction(instruction);
			break;
		}

		auto img_expr = to_expression(image_id);
		string size_expr;
		auto dim = img_type.image.dim;
		bool arrayed = img_type.image.arrayed;

		if (dim == Dim1D || dim == DimBuffer)
		{
			size_expr = join("get_image_width(", img_expr, ")");
		}
		else if (dim == Dim2D || dim == DimCube)
		{
			if (arrayed)
				size_expr = join("(int3)(get_image_width(", img_expr, "), get_image_height(", img_expr,
				                 "), get_image_array_size(", img_expr, "))");
			else
				size_expr = join("(int2)(get_image_width(", img_expr, "), get_image_height(", img_expr, "))");
		}
		else if (dim == Dim3D)
		{
			size_expr = join("(int3)(get_image_width(", img_expr, "), get_image_height(", img_expr,
			                 "), get_image_depth(", img_expr, "))");
		}
		else
		{
			CompilerGLSL::emit_instruction(instruction);
			break;
		}

		emit_op(result_type, result_id, size_expr, true);
		inherit_expression_dependencies(result_id, image_id);
		break;
	}

	default:
		CompilerGLSL::emit_instruction(instruction);
		break;
	}
}
