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
	backend.int16_t_literal_suffix = "";
	backend.uint16_t_literal_suffix = "";
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
	backend.c_style_casts = true;
	backend.supports_spec_constant_array_size = false;
	backend.matrix_column_accessor = "columns";

	fixup_anonymous_struct_names();
	fixup_type_alias();
	replace_illegal_names();
	fixup_image_load_store_access();
	build_function_control_flow_graphs_and_analyze();
	update_active_builtins();
	analyze_image_and_sampler_usage();
	analyze_interlocked_resource_usage();

	set_enabled_interface_variables(get_active_interface_variables());
	reorder_type_alias();

	// Pre-scan: discover all matrix types used in the IR so that typedefs
	// and helpers can be emitted in the first pass without forcing a recompile.
	prepass_discover_matrix_types();

	uint32_t pass_count = 0;
	do
	{
		auto prev_matrix_types = used_matrix_types;
		auto prev_helpers = need_mul_mat_vec.size() + need_mul_vec_mat.size() + need_mul_mat_mat.size() +
		                    need_mul_mat_scalar.size() + need_transpose.size() + need_outer_product.size();

		reset(pass_count);
		buffer.reset();

		emit_header();
		emit_matrix_typedefs();
		emit_specialization_constants_and_structs();
		emit_matrix_helpers();
		emit_resources();
		emit_function(get<SPIRFunction>(ir.default_entry_point), Bitset());

		auto new_helpers = need_mul_mat_vec.size() + need_mul_vec_mat.size() + need_mul_mat_mat.size() +
		                   need_mul_mat_scalar.size() + need_transpose.size() + need_outer_product.size();
		if (used_matrix_types != prev_matrix_types || new_helpers != prev_helpers)
			force_recompile();

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
	if (opencl_options.enable_fp16)
		statement("#pragma OPENCL EXTENSION cl_khr_fp16 : enable");
	if (opencl_options.enable_fp64)
		statement("#pragma OPENCL EXTENSION cl_khr_fp64 : enable");
	if (opencl_options.enable_64bit_atomics && opencl_options.opencl_version >= 200)
		statement("#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable");
	if (opencl_options.enable_subgroups)
		statement("#pragma OPENCL EXTENSION cl_khr_subgroups : enable");
	if (needs_subgroup_vote)
		statement("#pragma OPENCL EXTENSION cl_khr_subgroup_non_uniform_vote : enable");
	if (needs_subgroup_ballot)
		statement("#pragma OPENCL EXTENSION cl_khr_subgroup_ballot : enable");
	if (needs_subgroup_arithmetic)
		statement("#pragma OPENCL EXTENSION cl_khr_subgroup_non_uniform_arithmetic : enable");
	if (needs_subgroup_shuffle)
		statement("#pragma OPENCL EXTENSION cl_khr_subgroup_shuffle : enable");
	if (needs_subgroup_shuffle_relative)
		statement("#pragma OPENCL EXTENSION cl_khr_subgroup_shuffle_relative : enable");
	if (needs_subgroup_clustered)
		statement("#pragma OPENCL EXTENSION cl_khr_subgroup_clustered_reduce : enable");
	if (needs_subgroup_rotate)
		statement("#pragma OPENCL EXTENSION cl_khr_subgroup_rotate : enable");
	statement("");

	// Emit FP_CONTRACT pragma based on ContractionOff execution mode and FPFastMathDefault.
	{
		auto &ep = get_entry_point();
		bool contract = true;

		if (ep.flags.get(ExecutionModeContractionOff))
			contract = false;

		for (auto &fp_pair : ep.fp_fast_math_defaults)
		{
			if (fp_pair.second)
			{
				uint32_t flags = get<SPIRConstant>(fp_pair.second).scalar();
				if (!(flags & FPFastMathModeAllowContractMask))
					contract = false;
			}
		}

		if (!contract)
		{
			statement("#pragma OPENCL FP_CONTRACT OFF");
			statement("");
		}
	}

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
	// OpenCL C uses __restrict (after *) instead of GLSL's restrict (before type).
	// Convert DecorationRestrictPointerEXT → DecorationRestrict so the base class
	// flags_to_qualifiers_glsl does not emit "restrict " prefix, and our to_restrict
	// emits "__restrict" after the pointer star instead.
	ir.for_each_typed_id<SPIRVariable>(
	    [&](uint32_t, SPIRVariable &var)
	    {
		    auto &flags = get_decoration_bitset(var.self);
		    if (flags.get(DecorationRestrictPointerEXT))
		    {
			    unset_decoration(var.self, DecorationRestrictPointerEXT);
			    set_decoration(var.self, DecorationRestrict);
		    }
	    });

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
				    // BDA pointer members are stored as ulong in structs.
				    if (is_pointer(member0_type) && member0_type.storage == StorageClassPhysicalStorageBuffer)
					    subtype = "ulong";
				    else
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

	// Polyfill for bitfieldReverse (32-bit scalar only — vectors call per-component).
	if (needs_bitreverse_polyfill)
	{
		statement("uint spvBitReverse(uint v) {");
		statement("    v = ((v >> 1u) & 0x55555555u) | ((v & 0x55555555u) << 1u);");
		statement("    v = ((v >> 2u) & 0x33333333u) | ((v & 0x33333333u) << 2u);");
		statement("    v = ((v >> 4u) & 0x0F0F0F0Fu) | ((v & 0x0F0F0F0Fu) << 4u);");
		statement("    v = ((v >> 8u) & 0x00FF00FFu) | ((v & 0x00FF00FFu) << 8u);");
		statement("    return (v >> 16u) | (v << 16u);");
		statement("}");
		statement("");
	}

	// FindLSB polyfill: GLSL findLSB returns the bit position of the lowest set bit, or -1 if 0.
	// OpenCL 2.0+ has ctz() but OpenCL 1.2 does not. Use (x & -x) to isolate lowest bit,
	// then 31 - clz() to get its position.
	if (needs_findlsb_polyfill)
	{
		statement("static int spvFindLSB(uint x) {");
		statement("    if (x == 0u) return -1;");
		statement("    return 31 - as_int(clz(x & (0u - x)));");
		statement("}");
		statement("");
	}

	// Pack/Unpack Snorm/Unorm polyfills.
	if (needs_pack_snorm_4x8)
	{
		statement("static uint spvPackSnorm4x8(float4 v) {");
		statement("    char4 packed = convert_char4_sat_rte(v * 127.0f);");
		statement("    return as_uint(packed);");
		statement("}");
		statement("");
	}
	if (needs_pack_unorm_4x8)
	{
		statement("static uint spvPackUnorm4x8(float4 v) {");
		statement("    uchar4 packed = convert_uchar4_sat_rte(v * 255.0f);");
		statement("    return as_uint(packed);");
		statement("}");
		statement("");
	}
	if (needs_pack_snorm_2x16)
	{
		statement("static uint spvPackSnorm2x16(float2 v) {");
		statement("    short2 packed = convert_short2_sat_rte(v * 32767.0f);");
		statement("    return as_uint(packed);");
		statement("}");
		statement("");
	}
	if (needs_pack_unorm_2x16)
	{
		statement("static uint spvPackUnorm2x16(float2 v) {");
		statement("    ushort2 packed = convert_ushort2_sat_rte(v * 65535.0f);");
		statement("    return as_uint(packed);");
		statement("}");
		statement("");
	}
	if (needs_unpack_snorm_4x8)
	{
		statement("static float4 spvUnpackSnorm4x8(uint v) {");
		statement("    char4 packed = as_char4(v);");
		statement("    return max(convert_float4(packed) / 127.0f, (float4)(-1.0f));");
		statement("}");
		statement("");
	}
	if (needs_unpack_unorm_4x8)
	{
		statement("static float4 spvUnpackUnorm4x8(uint v) {");
		statement("    uchar4 packed = as_uchar4(v);");
		statement("    return convert_float4(packed) / 255.0f;");
		statement("}");
		statement("");
	}
	if (needs_unpack_snorm_2x16)
	{
		statement("static float2 spvUnpackSnorm2x16(uint v) {");
		statement("    short2 packed = as_short2(v);");
		statement("    return max(convert_float2(packed) / 32767.0f, (float2)(-1.0f));");
		statement("}");
		statement("");
	}
	if (needs_unpack_unorm_2x16)
	{
		statement("static float2 spvUnpackUnorm2x16(uint v) {");
		statement("    ushort2 packed = as_ushort2(v);");
		statement("    return convert_float2(packed) / 65535.0f;");
		statement("}");
		statement("");
	}

	// Determinant polyfills using struct-wrapped matrix types (unique names per size for C).
	if (needs_determinant_2)
	{
		auto mat = opencl_matrix_type_name(SPIRType::Float, 2, 2);
		statement("static float spvDeterminant2(", mat, " m) {");
		statement("    return m.columns[0].x * m.columns[1].y - m.columns[0].y * m.columns[1].x;");
		statement("}");
		statement("");
	}
	if (needs_determinant_3)
	{
		auto mat = opencl_matrix_type_name(SPIRType::Float, 3, 3);
		statement("static float spvDeterminant3(", mat, " m) {");
		statement("    return dot(m.columns[0], (float3)("
		          "m.columns[1].y * m.columns[2].z - m.columns[1].z * m.columns[2].y, "
		          "m.columns[1].z * m.columns[2].x - m.columns[1].x * m.columns[2].z, "
		          "m.columns[1].x * m.columns[2].y - m.columns[1].y * m.columns[2].x));");
		statement("}");
		statement("");
	}
	if (needs_determinant_4)
	{
		auto mat = opencl_matrix_type_name(SPIRType::Float, 4, 4);
		statement("static float spvDeterminant4(", mat, " m) {");
		statement(
		    "    return dot(m.columns[0], (float4)("
		    "m.columns[2].y * m.columns[3].z * m.columns[1].w - m.columns[3].y * m.columns[2].z * m.columns[1].w + "
		    "m.columns[3].y * m.columns[1].z * m.columns[2].w - m.columns[1].y * m.columns[3].z * m.columns[2].w - "
		    "m.columns[2].y * m.columns[1].z * m.columns[3].w + m.columns[1].y * m.columns[2].z * m.columns[3].w, "
		    "m.columns[3].x * m.columns[2].z * m.columns[1].w - m.columns[2].x * m.columns[3].z * m.columns[1].w - "
		    "m.columns[3].x * m.columns[1].z * m.columns[2].w + m.columns[1].x * m.columns[3].z * m.columns[2].w + "
		    "m.columns[2].x * m.columns[1].z * m.columns[3].w - m.columns[1].x * m.columns[2].z * m.columns[3].w, "
		    "m.columns[2].x * m.columns[3].y * m.columns[1].w - m.columns[3].x * m.columns[2].y * m.columns[1].w + "
		    "m.columns[3].x * m.columns[1].y * m.columns[2].w - m.columns[1].x * m.columns[3].y * m.columns[2].w - "
		    "m.columns[2].x * m.columns[1].y * m.columns[3].w + m.columns[1].x * m.columns[2].y * m.columns[3].w, "
		    "m.columns[3].x * m.columns[2].y * m.columns[1].z - m.columns[2].x * m.columns[3].y * m.columns[1].z - "
		    "m.columns[3].x * m.columns[1].y * m.columns[2].z + m.columns[1].x * m.columns[3].y * m.columns[2].z + "
		    "m.columns[2].x * m.columns[1].y * m.columns[3].z - m.columns[1].x * m.columns[2].y * m.columns[3].z));");
		statement("}");
		statement("");
	}

	// Matrix inverse polyfills.
	if (needs_inverse_2)
	{
		auto mat = opencl_matrix_type_name(SPIRType::Float, 2, 2);
		statement("static ", mat, " spvInverse2(", mat, " m) {");
		statement("    float d = 1.0f / (m.columns[0].x * m.columns[1].y - m.columns[1].x * m.columns[0].y);");
		statement("    return (", mat,
		          "){ { (float2)(m.columns[1].y * d, -m.columns[0].y * d), (float2)(-m.columns[1].x * d, "
		          "m.columns[0].x * d) } };");
		statement("}");
		statement("");
	}
	if (needs_inverse_3)
	{
		auto mat = opencl_matrix_type_name(SPIRType::Float, 3, 3);
		statement("static ", mat, " spvInverse3(", mat, " m) {");
		statement("    float3 t = (float3)("
		          "m.columns[1].y * m.columns[2].z - m.columns[1].z * m.columns[2].y, "
		          "m.columns[1].z * m.columns[2].x - m.columns[1].x * m.columns[2].z, "
		          "m.columns[1].x * m.columns[2].y - m.columns[1].y * m.columns[2].x);");
		statement("    float d = 1.0f / dot(m.columns[0], t);");
		statement("    return (", mat,
		          "){ { t * d, "
		          "(float3)(m.columns[0].z * m.columns[2].y - m.columns[0].y * m.columns[2].z, "
		          "m.columns[0].x * m.columns[2].z - m.columns[0].z * m.columns[2].x, "
		          "m.columns[0].y * m.columns[2].x - m.columns[0].x * m.columns[2].y) * d, "
		          "(float3)(m.columns[0].y * m.columns[1].z - m.columns[0].z * m.columns[1].y, "
		          "m.columns[0].z * m.columns[1].x - m.columns[0].x * m.columns[1].z, "
		          "m.columns[0].x * m.columns[1].y - m.columns[0].y * m.columns[1].x) * d } };");
		statement("}");
		statement("");
	}
	if (needs_inverse_4)
	{
		auto mat = opencl_matrix_type_name(SPIRType::Float, 4, 4);
		statement("static ", mat, " spvInverse4(", mat, " m) {");
		statement(
		    "    float4 t = (float4)("
		    "m.columns[2].y * m.columns[3].z * m.columns[1].w - m.columns[3].y * m.columns[2].z * m.columns[1].w + "
		    "m.columns[3].y * m.columns[1].z * m.columns[2].w - m.columns[1].y * m.columns[3].z * m.columns[2].w - "
		    "m.columns[2].y * m.columns[1].z * m.columns[3].w + m.columns[1].y * m.columns[2].z * m.columns[3].w, "
		    "m.columns[3].x * m.columns[2].z * m.columns[1].w - m.columns[2].x * m.columns[3].z * m.columns[1].w - "
		    "m.columns[3].x * m.columns[1].z * m.columns[2].w + m.columns[1].x * m.columns[3].z * m.columns[2].w + "
		    "m.columns[2].x * m.columns[1].z * m.columns[3].w - m.columns[1].x * m.columns[2].z * m.columns[3].w, "
		    "m.columns[2].x * m.columns[3].y * m.columns[1].w - m.columns[3].x * m.columns[2].y * m.columns[1].w + "
		    "m.columns[3].x * m.columns[1].y * m.columns[2].w - m.columns[1].x * m.columns[3].y * m.columns[2].w - "
		    "m.columns[2].x * m.columns[1].y * m.columns[3].w + m.columns[1].x * m.columns[2].y * m.columns[3].w, "
		    "m.columns[3].x * m.columns[2].y * m.columns[1].z - m.columns[2].x * m.columns[3].y * m.columns[1].z - "
		    "m.columns[3].x * m.columns[1].y * m.columns[2].z + m.columns[1].x * m.columns[3].y * m.columns[2].z + "
		    "m.columns[2].x * m.columns[1].y * m.columns[3].z - m.columns[1].x * m.columns[2].y * m.columns[3].z);");
		statement(
		    "    ", mat, " r = (", mat,
		    "){ { "
		    "(float4)(t.x, "
		    "m.columns[3].y * m.columns[2].z * m.columns[0].w - m.columns[2].y * m.columns[3].z * m.columns[0].w - "
		    "m.columns[3].y * m.columns[0].z * m.columns[2].w + m.columns[0].y * m.columns[3].z * m.columns[2].w + "
		    "m.columns[2].y * m.columns[0].z * m.columns[3].w - m.columns[0].y * m.columns[2].z * m.columns[3].w, "
		    "m.columns[1].y * m.columns[3].z * m.columns[0].w - m.columns[3].y * m.columns[1].z * m.columns[0].w + "
		    "m.columns[3].y * m.columns[0].z * m.columns[1].w - m.columns[0].y * m.columns[3].z * m.columns[1].w - "
		    "m.columns[1].y * m.columns[0].z * m.columns[3].w + m.columns[0].y * m.columns[1].z * m.columns[3].w, "
		    "m.columns[2].y * m.columns[1].z * m.columns[0].w - m.columns[1].y * m.columns[2].z * m.columns[0].w - "
		    "m.columns[2].y * m.columns[0].z * m.columns[1].w + m.columns[0].y * m.columns[2].z * m.columns[1].w + "
		    "m.columns[1].y * m.columns[0].z * m.columns[2].w - m.columns[0].y * m.columns[1].z * m.columns[2].w), "
		    "(float4)(t.y, "
		    "m.columns[2].x * m.columns[3].z * m.columns[0].w - m.columns[3].x * m.columns[2].z * m.columns[0].w + "
		    "m.columns[3].x * m.columns[0].z * m.columns[2].w - m.columns[0].x * m.columns[3].z * m.columns[2].w - "
		    "m.columns[2].x * m.columns[0].z * m.columns[3].w + m.columns[0].x * m.columns[2].z * m.columns[3].w, "
		    "m.columns[3].x * m.columns[1].z * m.columns[0].w - m.columns[1].x * m.columns[3].z * m.columns[0].w - "
		    "m.columns[3].x * m.columns[0].z * m.columns[1].w + m.columns[0].x * m.columns[3].z * m.columns[1].w + "
		    "m.columns[1].x * m.columns[0].z * m.columns[3].w - m.columns[0].x * m.columns[1].z * m.columns[3].w, "
		    "m.columns[1].x * m.columns[2].z * m.columns[0].w - m.columns[2].x * m.columns[1].z * m.columns[0].w + "
		    "m.columns[2].x * m.columns[0].z * m.columns[1].w - m.columns[0].x * m.columns[2].z * m.columns[1].w - "
		    "m.columns[1].x * m.columns[0].z * m.columns[2].w + m.columns[0].x * m.columns[1].z * m.columns[2].w), "
		    "(float4)(t.z, "
		    "m.columns[3].x * m.columns[2].y * m.columns[0].w - m.columns[2].x * m.columns[3].y * m.columns[0].w - "
		    "m.columns[3].x * m.columns[0].y * m.columns[2].w + m.columns[0].x * m.columns[3].y * m.columns[2].w + "
		    "m.columns[2].x * m.columns[0].y * m.columns[3].w - m.columns[0].x * m.columns[2].y * m.columns[3].w, "
		    "m.columns[1].x * m.columns[3].y * m.columns[0].w - m.columns[3].x * m.columns[1].y * m.columns[0].w + "
		    "m.columns[3].x * m.columns[0].y * m.columns[1].w - m.columns[0].x * m.columns[3].y * m.columns[1].w - "
		    "m.columns[1].x * m.columns[0].y * m.columns[3].w + m.columns[0].x * m.columns[1].y * m.columns[3].w, "
		    "m.columns[2].x * m.columns[1].y * m.columns[0].w - m.columns[1].x * m.columns[2].y * m.columns[0].w - "
		    "m.columns[2].x * m.columns[0].y * m.columns[1].w + m.columns[0].x * m.columns[2].y * m.columns[1].w + "
		    "m.columns[1].x * m.columns[0].y * m.columns[2].w - m.columns[0].x * m.columns[1].y * m.columns[2].w), "
		    "(float4)(t.w, "
		    "m.columns[2].x * m.columns[3].y * m.columns[0].z - m.columns[3].x * m.columns[2].y * m.columns[0].z + "
		    "m.columns[3].x * m.columns[0].y * m.columns[2].z - m.columns[0].x * m.columns[3].y * m.columns[2].z - "
		    "m.columns[2].x * m.columns[0].y * m.columns[3].z + m.columns[0].x * m.columns[2].y * m.columns[3].z, "
		    "m.columns[3].x * m.columns[1].y * m.columns[0].z - m.columns[1].x * m.columns[3].y * m.columns[0].z - "
		    "m.columns[3].x * m.columns[0].y * m.columns[1].z + m.columns[0].x * m.columns[3].y * m.columns[1].z + "
		    "m.columns[1].x * m.columns[0].y * m.columns[3].z - m.columns[0].x * m.columns[1].y * m.columns[3].z, "
		    "m.columns[1].x * m.columns[2].y * m.columns[0].z - m.columns[2].x * m.columns[1].y * m.columns[0].z + "
		    "m.columns[2].x * m.columns[0].y * m.columns[1].z - m.columns[0].x * m.columns[2].y * m.columns[1].z - "
		    "m.columns[1].x * m.columns[0].y * m.columns[2].z + m.columns[0].x * m.columns[1].y * m.columns[2].z) } "
		    "};");
		statement("    float d = 1.0f / dot(m.columns[0], t);");
		statement("    r.columns[0] *= d; r.columns[1] *= d; r.columns[2] *= d; r.columns[3] *= d;");
		statement("    return r;");
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
			// CompilerGLSL::variable_decl(var) already includes the initializer
			// expression (via to_initializer_expression), so no extra initializer needed.
			statement(CompilerGLSL::variable_decl(var), ";");
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
		if (!opencl_options.enable_subgroups)
			SPIRV_CROSS_THROW("Subgroup builtins require enable_subgroups option.");
		return "get_num_sub_groups()";
	case BuiltInSubgroupId:
		if (!opencl_options.enable_subgroups)
			SPIRV_CROSS_THROW("Subgroup builtins require enable_subgroups option.");
		return "get_sub_group_id()";
	case BuiltInSubgroupSize:
		if (!opencl_options.enable_subgroups)
			SPIRV_CROSS_THROW("Subgroup builtins require enable_subgroups option.");
		return "get_sub_group_size()";
	case BuiltInSubgroupLocalInvocationId:
		if (!opencl_options.enable_subgroups)
			SPIRV_CROSS_THROW("Subgroup builtins require enable_subgroups option.");
		return "get_sub_group_local_id()";
	case BuiltInSubgroupEqMask:
		if (!opencl_options.enable_subgroups)
			SPIRV_CROSS_THROW("Subgroup builtins require enable_subgroups option.");
		if (!needs_subgroup_ballot)
		{
			needs_subgroup_ballot = true;
			force_recompile();
		}
		return "get_sub_group_eq_mask()";
	case BuiltInSubgroupGeMask:
		if (!opencl_options.enable_subgroups)
			SPIRV_CROSS_THROW("Subgroup builtins require enable_subgroups option.");
		if (!needs_subgroup_ballot)
		{
			needs_subgroup_ballot = true;
			force_recompile();
		}
		return "get_sub_group_ge_mask()";
	case BuiltInSubgroupGtMask:
		if (!opencl_options.enable_subgroups)
			SPIRV_CROSS_THROW("Subgroup builtins require enable_subgroups option.");
		if (!needs_subgroup_ballot)
		{
			needs_subgroup_ballot = true;
			force_recompile();
		}
		return "get_sub_group_gt_mask()";
	case BuiltInSubgroupLeMask:
		if (!opencl_options.enable_subgroups)
			SPIRV_CROSS_THROW("Subgroup builtins require enable_subgroups option.");
		if (!needs_subgroup_ballot)
		{
			needs_subgroup_ballot = true;
			force_recompile();
		}
		return "get_sub_group_le_mask()";
	case BuiltInSubgroupLtMask:
		if (!opencl_options.enable_subgroups)
			SPIRV_CROSS_THROW("Subgroup builtins require enable_subgroups option.");
		if (!needs_subgroup_ballot)
		{
			needs_subgroup_ballot = true;
			force_recompile();
		}
		return "get_sub_group_lt_mask()";
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
	case StorageClassPhysicalStorageBuffer:
		addr_space = "__global";
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

	// DecorationRestrictPointerEXT is converted to DecorationRestrict in
	// compute_kernel_resources(), so only check DecorationRestrict here.
	return flags.get(DecorationRestrict) ? (space ? "__restrict " : "__restrict") : "";
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
		if (!opencl_options.enable_fp16)
			SPIRV_CROSS_THROW("Half requires cl_khr_fp16.");
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

	// Matrix? (columns > 1)
	if (type.columns > 1)
	{
		used_matrix_types.insert(make_matrix_key(type));
		return opencl_matrix_type_name(type);
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

CompilerOpenCL::MatrixTypeKey CompilerOpenCL::make_matrix_key(const SPIRType &type)
{
	return { type.basetype, type.vecsize, type.columns };
}

string CompilerOpenCL::opencl_column_type_name(SPIRType::BaseType basetype, uint32_t vecsize)
{
	string name;
	switch (basetype)
	{
	case SPIRType::Float:
		name = "float";
		break;
	case SPIRType::Double:
		name = "double";
		break;
	case SPIRType::Half:
		name = "half";
		break;
	default:
		name = "float";
		break;
	}
	if (vecsize > 1)
		name += to_string(vecsize);
	return name;
}

string CompilerOpenCL::opencl_matrix_type_name(SPIRType::BaseType basetype, uint32_t vecsize, uint32_t columns)
{
	string prefix = "spv";
	if (basetype == SPIRType::Double)
		prefix += "D";
	else if (basetype == SPIRType::Half)
		prefix += "H";
	prefix += "Mat";
	if (columns == vecsize)
		return prefix + to_string(columns);
	return prefix + to_string(columns) + "x" + to_string(vecsize);
}

string CompilerOpenCL::opencl_matrix_type_name(const SPIRType &type)
{
	return opencl_matrix_type_name(type.basetype, type.vecsize, type.columns);
}

string CompilerOpenCL::opencl_matrix_short_name(SPIRType::BaseType basetype, uint32_t vecsize, uint32_t columns)
{
	// Returns e.g. "Mat4", "DMat4", "HMat4", "Mat4x3", "DMat4x3"
	string prefix;
	if (basetype == SPIRType::Double)
		prefix = "D";
	else if (basetype == SPIRType::Half)
		prefix = "H";
	prefix += "Mat";
	if (columns == vecsize)
		return prefix + to_string(columns);
	return prefix + to_string(columns) + "x" + to_string(vecsize);
}

string CompilerOpenCL::opencl_vector_short_name(SPIRType::BaseType basetype, uint32_t vecsize)
{
	// Returns e.g. "Vec4", "DVec4", "HVec4", "Scalar" for vecsize 1
	if (vecsize == 1)
	{
		if (basetype == SPIRType::Double)
			return "DScalar";
		if (basetype == SPIRType::Half)
			return "HScalar";
		return "Scalar";
	}
	string prefix;
	if (basetype == SPIRType::Double)
		prefix = "D";
	else if (basetype == SPIRType::Half)
		prefix = "H";
	return prefix + "Vec" + to_string(vecsize);
}

void CompilerOpenCL::prepass_discover_matrix_types()
{
	used_matrix_types.clear();
	need_mul_mat_vec.clear();
	need_mul_vec_mat.clear();
	need_mul_mat_mat.clear();
	need_mul_mat_scalar.clear();
	need_transpose.clear();
	need_outer_product.clear();

	// Scan all types for matrix members.
	ir.for_each_typed_id<SPIRType>(
	    [&](uint32_t, SPIRType &type)
	    {
		    if (type.columns > 1 && type.basetype != SPIRType::Struct)
			    used_matrix_types.insert(make_matrix_key(type));
		    for (auto &member_type_id : type.member_types)
		    {
			    auto &member_type = get<SPIRType>(member_type_id);
			    if (member_type.columns > 1)
				    used_matrix_types.insert(make_matrix_key(member_type));
		    }
	    });

	// Scan all instructions for matrix operations to discover helpers needed.
	// We can resolve the matrix type from the SPIR-V type of operands at pre-scan time.
	auto get_id_type = [&](uint32_t id) -> const SPIRType &
	{
		// For value IDs, look up the type from variable, constant, or the instruction result.
		auto *var = maybe_get<SPIRVariable>(id);
		if (var)
			return get_variable_data_type(*var);
		auto *c = maybe_get<SPIRConstant>(id);
		if (c)
			return get<SPIRType>(c->constant_type);
		// For instruction results, the type is stored in the expression or type_id.
		if (ir.ids[id].get_type() == TypeExpression)
			return get<SPIRType>(get<SPIRExpression>(id).expression_type);
		// For types themselves
		if (ir.ids[id].get_type() == TypeType)
			return get<SPIRType>(id);
		// Fallback: check if there's a result type mapping
		return get<SPIRType>(id);
	};

	ir.for_each_typed_id<SPIRFunction>(
	    [&](uint32_t, SPIRFunction &f)
	    {
		    for (auto &block_id : f.blocks)
		    {
			    auto &block = get<SPIRBlock>(block_id);
			    for (auto &instruction : block.ops)
			    {
				    auto ops = stream(instruction);
				    auto opcode = static_cast<Op>(instruction.op);

				    // Helper lambda to resolve the type of a SPIR-V value ID from the instruction.
				    // For OpMatrixTimesVector etc., ops[2] and ops[3] are value IDs whose types
				    // may not be directly available at pre-scan time. Instead, we check the
				    // instruction result type to infer what's needed.
				    switch (opcode)
				    {
				    case OpMatrixTimesVector:
				    {
					    // ops[0] = result type (vector), ops[2] = matrix, ops[3] = vector
					    // The matrix type is not directly available from ops[2] here.
					    // We infer from the result: result is vec(vecsize), matrix has same vecsize.
					    // But we need the column count too. Let's look up the variable type.
					    // At pre-scan time, not all IDs have resolved types, so we'll rely on
					    // the recompile mechanism for helpers that can't be pre-discovered.
					    break;
				    }
				    case OpOuterProduct:
				    {
					    auto &res_type = get<SPIRType>(ops[0]);
					    if (res_type.columns > 1)
					    {
						    used_matrix_types.insert(make_matrix_key(res_type));
						    auto col_short = opencl_vector_short_name(res_type.basetype, res_type.vecsize);
						    auto row_short = opencl_vector_short_name(res_type.basetype, res_type.columns);
						    (void)col_short;
						    (void)row_short;
						    need_outer_product.insert(make_matrix_key(res_type));
					    }
					    break;
				    }
				    case OpTranspose:
				    {
					    auto &res_type = get<SPIRType>(ops[0]);
					    if (res_type.columns > 1)
					    {
						    used_matrix_types.insert(make_matrix_key(res_type));
						    // The input type has swapped dimensions.
						    MatrixTypeKey input_key = { res_type.basetype, res_type.columns, res_type.vecsize };
						    used_matrix_types.insert(input_key);
						    need_transpose.insert(input_key);
					    }
					    break;
				    }
				    case OpMatrixTimesScalar:
				    case OpMatrixTimesMatrix:
				    case OpVectorTimesMatrix:
					    // These will be discovered during emit_instruction and trigger recompile if needed.
					    break;
				    default:
					    break;
				    }
			    }
		    }
	    });
}

void CompilerOpenCL::emit_matrix_typedefs()
{
	if (used_matrix_types.empty())
		return;

	for (auto &key : used_matrix_types)
	{
		auto col_type = opencl_column_type_name(key.basetype, key.vecsize);
		auto mat_name = opencl_matrix_type_name(key.basetype, key.vecsize, key.columns);
		statement("typedef struct { ", col_type, " columns[", key.columns, "]; } ", mat_name, ";");
	}
	statement("");
}

void CompilerOpenCL::emit_matrix_helpers()
{
	for (auto &key : need_mul_mat_vec)
		emit_mul_mat_vec_helper(key.basetype, key.vecsize, key.columns);
	for (auto &key : need_mul_vec_mat)
		emit_mul_vec_mat_helper(key.basetype, key.vecsize, key.columns);
	for (auto &key : need_mul_mat_mat)
		emit_mul_mat_mat_helper(key.first, key.second);
	for (auto &key : need_mul_mat_scalar)
		emit_mul_mat_scalar_helper(key.basetype, key.vecsize, key.columns);
	for (auto &key : need_transpose)
		emit_transpose_helper(key.basetype, key.vecsize, key.columns);
	for (auto &key : need_outer_product)
		emit_outer_product_helper(key.basetype, key.vecsize, key.columns);
}

void CompilerOpenCL::emit_mul_mat_vec_helper(SPIRType::BaseType basetype, uint32_t vecsize, uint32_t columns)
{
	auto mat_type = opencl_matrix_type_name(basetype, vecsize, columns);
	auto vec_result = opencl_column_type_name(basetype, vecsize);
	auto vec_arg = opencl_column_type_name(basetype, columns);
	auto mat_short = opencl_matrix_short_name(basetype, vecsize, columns);
	auto vec_short = opencl_vector_short_name(basetype, columns);
	string func_name = "spvMul" + mat_short + vec_short;

	statement("static ", vec_result, " ", func_name, "(", mat_type, " m, ", vec_arg, " v)");
	begin_scope();
	string expr = "return ";
	const char *swizzles[] = { "x",  "y",  "z",  "w",  "s4", "s5", "s6", "s7",
		                       "s8", "s9", "sa", "sb", "sc", "sd", "se", "sf" };
	for (uint32_t i = 0; i < columns; i++)
	{
		if (i > 0)
			expr += " + ";
		expr += "m.columns[" + to_string(i) + "]";
		if (columns > 1)
			expr += string(" * v.") + swizzles[i];
	}
	expr += ";";
	statement(expr);
	end_scope();
	statement("");
}

void CompilerOpenCL::emit_mul_vec_mat_helper(SPIRType::BaseType basetype, uint32_t vecsize, uint32_t columns)
{
	auto mat_type = opencl_matrix_type_name(basetype, vecsize, columns);
	auto in_vec = opencl_column_type_name(basetype, vecsize);
	auto out_vec = opencl_column_type_name(basetype, columns);
	auto vec_short = opencl_vector_short_name(basetype, vecsize);
	auto mat_short = opencl_matrix_short_name(basetype, vecsize, columns);
	string func_name = "spvMul" + vec_short + mat_short;

	statement("static ", out_vec, " ", func_name, "(", in_vec, " v, ", mat_type, " m)");
	begin_scope();
	string expr = "return (" + out_vec + ")(";
	for (uint32_t i = 0; i < columns; i++)
	{
		if (i > 0)
			expr += ", ";
		expr += "dot(v, m.columns[" + to_string(i) + "])";
	}
	expr += ");";
	statement(expr);
	end_scope();
	statement("");
}

void CompilerOpenCL::emit_mul_mat_mat_helper(const MatrixTypeKey &a, const MatrixTypeKey &b)
{
	auto mat_a_type = opencl_matrix_type_name(a.basetype, a.vecsize, a.columns);
	auto mat_b_type = opencl_matrix_type_name(b.basetype, b.vecsize, b.columns);
	auto result_type = opencl_matrix_type_name(a.basetype, a.vecsize, b.columns);
	auto mat_a_short = opencl_matrix_short_name(a.basetype, a.vecsize, a.columns);
	auto mat_b_short = opencl_matrix_short_name(b.basetype, b.vecsize, b.columns);
	string func_name = "spvMul" + mat_a_short + mat_b_short;

	auto mv_vec_short = opencl_vector_short_name(a.basetype, a.columns);
	string mul_mv_func = "spvMul" + mat_a_short + mv_vec_short;

	statement("static ", result_type, " ", func_name, "(", mat_a_type, " a, ", mat_b_type, " b)");
	begin_scope();
	statement(result_type, " r;");
	for (uint32_t i = 0; i < b.columns; i++)
		statement("r.columns[", i, "] = ", mul_mv_func, "(a, b.columns[", i, "]);");
	statement("return r;");
	end_scope();
	statement("");
}

void CompilerOpenCL::emit_mul_mat_scalar_helper(SPIRType::BaseType basetype, uint32_t vecsize, uint32_t columns)
{
	auto mat_type = opencl_matrix_type_name(basetype, vecsize, columns);
	auto scalar_type = opencl_column_type_name(basetype, 1);
	auto mat_short = opencl_matrix_short_name(basetype, vecsize, columns);
	string func_name = "spvMul" + mat_short + "Scalar";

	statement("static ", mat_type, " ", func_name, "(", mat_type, " m, ", scalar_type, " s)");
	begin_scope();
	statement(mat_type, " r;");
	for (uint32_t i = 0; i < columns; i++)
		statement("r.columns[", i, "] = m.columns[", i, "] * s;");
	statement("return r;");
	end_scope();
	statement("");
}

void CompilerOpenCL::emit_transpose_helper(SPIRType::BaseType basetype, uint32_t vecsize, uint32_t columns)
{
	auto in_type = opencl_matrix_type_name(basetype, vecsize, columns);
	auto out_type = opencl_matrix_type_name(basetype, columns, vecsize);
	auto in_short = opencl_matrix_short_name(basetype, vecsize, columns);
	string func_name = "spvTranspose" + in_short;
	const char *swizzles[] = { "x", "y", "z", "w" };

	statement("static ", out_type, " ", func_name, "(", in_type, " m)");
	begin_scope();
	statement(out_type, " r;");
	for (uint32_t i = 0; i < vecsize; i++)
	{
		string expr = "r.columns[" + to_string(i) + "] = (" + opencl_column_type_name(basetype, columns) + ")(";
		for (uint32_t j = 0; j < columns; j++)
		{
			if (j > 0)
				expr += ", ";
			expr += "m.columns[" + to_string(j) + "]." + swizzles[i];
		}
		expr += ");";
		statement(expr);
	}
	statement("return r;");
	end_scope();
	statement("");
}

void CompilerOpenCL::emit_outer_product_helper(SPIRType::BaseType basetype, uint32_t vecsize, uint32_t columns)
{
	auto mat_type = opencl_matrix_type_name(basetype, vecsize, columns);
	auto col_type = opencl_column_type_name(basetype, vecsize);
	auto row_type = opencl_column_type_name(basetype, columns);
	auto col_short = opencl_vector_short_name(basetype, vecsize);
	auto row_short = opencl_vector_short_name(basetype, columns);
	string func_name = "spvOuterProduct" + col_short + row_short;
	const char *swizzles[] = { "x", "y", "z", "w" };

	statement("static ", mat_type, " ", func_name, "(", col_type, " c, ", row_type, " r)");
	begin_scope();
	statement(mat_type, " m;");
	for (uint32_t i = 0; i < columns; i++)
		statement("m.columns[", i, "] = c * r.", swizzles[i], ";");
	statement("return m;");
	end_scope();
	statement("");
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

bool CompilerOpenCL::member_is_non_native_row_major_matrix(const SPIRType &type, uint32_t index)
{
	// OpenCL backend uses struct-wrapped matrices with transpose helpers,
	// so we can handle non-square row-major matrices (unlike the base GLSL class).
	if (!has_member_decoration(type.self, index, DecorationRowMajor))
		return false;

	const auto mbr_type = get<SPIRType>(type.member_types[index]);
	if (mbr_type.columns <= 1)
		return false;

	return true;
}

string CompilerOpenCL::convert_row_major_matrix(string exp_str, const SPIRType &exp_type, uint32_t physical_type_id,
                                                bool is_packed, bool relaxed)
{
	strip_enclosed_expression(exp_str);
	if (!is_matrix(exp_type))
	{
		// Column access from a row-major matrix — delegate to base class unrolling.
		return CompilerGLSL::convert_row_major_matrix(std::move(exp_str), exp_type, physical_type_id, is_packed,
		                                              relaxed);
	}

	// Full matrix transpose: use our spvTranspose helper.
	// The expression string is in the physical (transposed) layout.
	// exp_type is the SPIR-V logical type. The physical type has swapped dimensions.
	// We transpose FROM physical TO logical: spvTranspose_PhysType_(phys_expr) -> logical_type
	uint32_t phys_vecsize = exp_type.columns;
	uint32_t phys_columns = exp_type.vecsize;
	auto phys_short = opencl_matrix_short_name(exp_type.basetype, phys_vecsize, phys_columns);
	MatrixTypeKey phys_key = { exp_type.basetype, phys_vecsize, phys_columns };
	need_transpose.insert(phys_key);
	used_matrix_types.insert(phys_key);
	used_matrix_types.insert(make_matrix_key(exp_type));

	return join("spvTranspose", phys_short, "(", exp_str, ")");
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

	// Matrix constant: emit as struct compound literal.
	if (type.columns > 1)
	{
		auto mat_name = opencl_matrix_type_name(type);
		string expr = "(" + mat_name + "){ { ";
		if (c.replicated)
		{
			auto sub_expr = to_expression(c.subconstants[0]);
			for (uint32_t i = 0; i < type.columns; i++)
			{
				if (i > 0)
					expr += ", ";
				expr += sub_expr;
			}
		}
		else
		{
			for (uint32_t i = 0; i < type.columns; i++)
			{
				if (i > 0)
					expr += ", ";
				expr += constant_expression_vector(c, i);
			}
		}
		expr += " } }";
		return expr;
	}

	if (c.replicated && type.op != OpTypeArray)
	{
		auto sub_expr = to_expression(c.subconstants[0]);
		// Vector replicate: (float4)(scalar)
		return join(type_to_glsl_constructor(type), "(", sub_expr, ")");
	}
	return CompilerGLSL::constant_expression(c, inside_block_like_struct_scope, inside_struct_scope);
}

std::string CompilerOpenCL::to_initializer_expression(const SPIRVariable &var)
{
	// OpenCL C does not support initializing arrays from non-constant expressions
	// (e.g., `float a[5] = ssbo->b;` is not valid C).
	// For array variables with non-constant initializers, emit zero init `{ 0 }` and
	// schedule element-by-element copy after the declaration.
	// SPIR-V spec only allows constant initializers on OpVariable, so array
	// initializers are always constants and valid as-is in OpenCL C.
	// Non-constant array copies are handled by emit_store_statement (OpStore).
	return CompilerGLSL::to_initializer_expression(var);
}

// OpenCL C requires cast syntax for vector construction: (float4)(1.0, 2.0, 3.0, 4.0)
// The GLSL base emits: float4(1.0, 2.0, 3.0, 4.0) which is invalid in OpenCL C.
std::string CompilerOpenCL::constant_expression_vector(const SPIRConstant &c, uint32_t vector)
{
	string res = CompilerGLSL::constant_expression_vector(c, vector);

	auto type = get<SPIRType>(c.constant_type);
	type.columns = 1;

	// The base class emits GLSL constructor-style casts: typename(args).
	// OpenCL C requires C-style casts: (typename)(args).
	// This applies to both vector types (e.g. float4(x)) and scalar casts
	// (e.g. int(0x80000000), long(0x8000000000000000ul), uchar(0)).
	auto scalar_type = type;
	scalar_type.vecsize = 1;
	auto type_name = (type.vecsize > 1) ? type_to_glsl(type) : type_to_glsl(scalar_type);
	if (!type_name.empty() && res.size() > type_name.size() + 1 && res.substr(0, type_name.size()) == type_name &&
	    res[type_name.size()] == '(')
	{
		res = "(" + type_name + ")(" + res.substr(type_name.size() + 1);
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

	case GLSLstd450SAbs:
	{
		// OpenCL abs() on integer types returns unsigned. Need bitcast back to signed if result is signed.
		auto &out_type = get<SPIRType>(result_type);
		auto &expr_type = expression_type(args[0]);

		// Cast input to signed if needed.
		string input_expr;
		auto expected_basetype = to_signed_basetype(expr_type.width);
		if (expr_type.basetype != expected_basetype)
			input_expr = bitcast_expression(expected_basetype, args[0]);
		else
			input_expr = to_expression(args[0]);

		string expr = join("abs(", input_expr, ")");

		// abs() returns unsigned in OpenCL. Cast to result type if it's signed.
		auto unsigned_basetype = to_unsigned_basetype(expr_type.width);
		if (out_type.basetype != unsigned_basetype)
		{
			// Build the unsigned return type to bitcast from.
			SPIRType abs_ret_type = out_type;
			abs_ret_type.basetype = unsigned_basetype;
			expr = join(bitcast_glsl_op(out_type, abs_ret_type), "(", expr, ")");
		}

		emit_op(result_type, result_id, expr, should_forward(args[0]));
		inherit_expression_dependencies(result_id, args[0]);
		break;
	}

	case GLSLstd450SSign:
	{
		// OpenCL has no integer sign(). Use clamp(x, -1, 1).
		auto &expr_type = expression_type(args[0]);
		auto &out_type = get<SPIRType>(result_type);

		auto expected_basetype = to_signed_basetype(expr_type.width);
		string input_expr;
		if (expr_type.basetype != expected_basetype)
			input_expr = bitcast_expression(expected_basetype, args[0]);
		else
			input_expr = to_expression(args[0]);

		string expr = join("clamp(", input_expr, ", -1, 1)");

		// Cast to result type if needed (e.g. result is unsigned).
		if (out_type.basetype != expected_basetype)
		{
			SPIRType signed_type = out_type;
			signed_type.basetype = expected_basetype;
			expr = join(bitcast_glsl_op(out_type, signed_type), "(", expr, ")");
		}

		emit_op(result_type, result_id, expr, should_forward(args[0]));
		inherit_expression_dependencies(result_id, args[0]);
		break;
	}

	case GLSLstd450FindSMsb:
	{
		// GLSL findMSB for signed: position of highest bit that differs from sign bit.
		// OpenCL: (W-1) - clz(x ^ (x >> (W-1)))
		// x >> (W-1) is arithmetic shift: 0 for positive, -1 for negative.
		// x ^ -1 = ~x, x ^ 0 = x. So this gives clz(x) for positive, clz(~x) for negative.
		auto &expr_type = expression_type(args[0]);
		auto &out_type = get<SPIRType>(result_type);
		uint32_t width = expr_type.width;

		// Input must be signed for arithmetic right shift.
		auto signed_basetype = to_signed_basetype(width);
		SPIRType signed_type = expr_type;
		signed_type.basetype = signed_basetype;

		string input_expr;
		if (expr_type.basetype != signed_basetype)
			input_expr = bitcast_expression(signed_basetype, args[0]);
		else
			input_expr = to_enclosed_expression(args[0]);

		string xor_expr = join(input_expr, " ^ (", input_expr, " >> ", width - 1, ")");
		string expr = join(width - 1, " - clz(", xor_expr, ")");

		// clz on signed type returns signed, so result is signed. Cast if output is unsigned.
		if (out_type.basetype != signed_basetype)
			expr = join(bitcast_glsl_op(out_type, signed_type), "(", expr, ")");

		emit_op(result_type, result_id, expr, should_forward(args[0]));
		inherit_expression_dependencies(result_id, args[0]);
		break;
	}

	case GLSLstd450FindUMsb:
	{
		// GLSL findMSB for unsigned: position of highest set bit, -1 for 0.
		// OpenCL: (W-1) - clz(x). clz(0) = W, so result = -1 for 0.
		auto &expr_type = expression_type(args[0]);
		auto &out_type = get<SPIRType>(result_type);
		uint32_t width = expr_type.width;

		auto unsigned_basetype = to_unsigned_basetype(width);
		string input_expr;
		if (expr_type.basetype != unsigned_basetype)
			input_expr = bitcast_expression(unsigned_basetype, args[0]);
		else
			input_expr = to_expression(args[0]);

		// Cast to signed for the subtraction so result can be -1.
		auto signed_basetype = to_signed_basetype(width);
		SPIRType signed_type = out_type;
		signed_type.basetype = signed_basetype;
		string clz_expr = join("as_", type_to_glsl(signed_type), "(clz(", input_expr, "))");

		string expr = join(width - 1, " - ", clz_expr);

		// findMSB returns int (signed). Cast if output type differs.
		if (out_type.basetype != signed_basetype)
		{
			expr = join(bitcast_glsl_op(out_type, signed_type), "(", expr, ")");
		}

		emit_op(result_type, result_id, expr, should_forward(args[0]));
		inherit_expression_dependencies(result_id, args[0]);
		break;
	}

	case GLSLstd450InverseSqrt:
		emit_unary_func_op(result_type, result_id, args[0], "rsqrt");
		break;

	case GLSLstd450RoundEven:
		emit_unary_func_op(result_type, result_id, args[0], "rint");
		break;

	case GLSLstd450Fract:
	{
		// OpenCL fract() requires a pointer argument. Use (x - floor(x)) inline.
		auto expr = join("(", to_expression(args[0]), " - floor(", to_expression(args[0]), "))");
		emit_op(result_type, result_id, expr, should_forward(args[0]));
		inherit_expression_dependencies(result_id, args[0]);
		break;
	}

	case GLSLstd450Atan2:
		emit_binary_func_op(result_type, result_id, args[0], args[1], "atan2");
		break;

	case GLSLstd450Radians:
		emit_unary_func_op(result_type, result_id, args[0], "radians");
		break;

	case GLSLstd450Degrees:
		emit_unary_func_op(result_type, result_id, args[0], "degrees");
		break;

	case GLSLstd450FindILsb:
	{
		if (!needs_findlsb_polyfill)
		{
			needs_findlsb_polyfill = true;
			force_recompile();
		}
		auto &input_type = expression_type(args[0]);
		auto &out_type = get<SPIRType>(result_type);
		// spvFindLSB takes uint. Cast input to uint if signed, and handle vector by component.
		if (input_type.vecsize > 1)
		{
			// Vector: apply per-component using .s0, .s1, etc.
			string expr = "(" + type_to_glsl(out_type) + ")(";
			const char *swizzles[] = { "x", "y", "z", "w" };
			for (uint32_t i = 0; i < input_type.vecsize; i++)
			{
				if (i > 0)
					expr += ", ";
				if (input_type.basetype == SPIRType::Int)
					expr += join("spvFindLSB(as_uint(", to_expression(args[0]), ".", swizzles[i], "))");
				else
					expr += join("spvFindLSB(", to_expression(args[0]), ".", swizzles[i], ")");
			}
			expr += ")";
			emit_op(result_type, result_id, expr, should_forward(args[0]));
			inherit_expression_dependencies(result_id, args[0]);
		}
		else
		{
			string input_expr;
			if (input_type.basetype == SPIRType::Int)
				input_expr = join("spvFindLSB(as_uint(", to_expression(args[0]), "))");
			else
				input_expr = join("spvFindLSB(", to_expression(args[0]), ")");
			emit_op(result_type, result_id, input_expr, should_forward(args[0]));
			inherit_expression_dependencies(result_id, args[0]);
		}
		break;
	}

	case GLSLstd450FaceForward:
	{
		// OpenCL C has no faceforward(). Implement inline.
		// faceforward(N, I, Nref) = dot(Nref, I) < 0 ? N : -N
		auto &type = get<SPIRType>(result_type);
		if (type.vecsize == 1)
		{
			auto expr = join("(", to_expression(args[2]), " * ", to_expression(args[1]), " < 0.0f ? ",
			                 to_expression(args[0]), " : -", to_expression(args[0]), ")");
			emit_op(result_type, result_id, expr,
			        should_forward(args[0]) && should_forward(args[1]) && should_forward(args[2]));
		}
		else
		{
			auto expr = join("(dot(", to_expression(args[2]), ", ", to_expression(args[1]), ") < 0.0f ? ",
			                 to_expression(args[0]), " : -", to_expression(args[0]), ")");
			emit_op(result_type, result_id, expr,
			        should_forward(args[0]) && should_forward(args[1]) && should_forward(args[2]));
		}
		for (uint32_t i = 0; i < 3; i++)
			inherit_expression_dependencies(result_id, args[i]);
		break;
	}

	case GLSLstd450Reflect:
	{
		// OpenCL C has no reflect(). Implement inline.
		// reflect(I, N) = I - 2 * dot(N, I) * N
		auto &type = get<SPIRType>(result_type);
		if (type.vecsize == 1)
		{
			auto expr = join(to_enclosed_expression(args[0]), " - 2.0f * ", to_enclosed_expression(args[1]), " * ",
			                 to_enclosed_expression(args[0]), " * ", to_enclosed_expression(args[1]));
			emit_op(result_type, result_id, expr, should_forward(args[0]) && should_forward(args[1]));
		}
		else
		{
			auto expr = join(to_expression(args[0]), " - 2.0f * dot(", to_expression(args[1]), ", ",
			                 to_expression(args[0]), ") * ", to_expression(args[1]));
			emit_op(result_type, result_id, expr, should_forward(args[0]) && should_forward(args[1]));
		}
		inherit_expression_dependencies(result_id, args[0]);
		inherit_expression_dependencies(result_id, args[1]);
		break;
	}

	case GLSLstd450Refract:
	{
		// OpenCL C has no refract(). Implement inline.
		// refract(I, N, eta): k = 1 - eta^2*(1 - dot(N,I)^2); k < 0 ? 0 : eta*I - (eta*dot(N,I)+sqrt(k))*N
		auto &type = get<SPIRType>(result_type);
		forced_temporaries.insert(result_id);
		auto type_name = type_to_glsl(type);
		emit_op(result_type, result_id, join("(", type_name, ")(0.0f)"), false);
		auto I = to_expression(args[0]);
		auto N = to_expression(args[1]);
		auto eta = to_expression(args[2]);
		auto res = to_expression(result_id);
		statement("{");
		if (type.vecsize == 1)
		{
			statement("    float spv_NdotI = ", N, " * ", I, ";");
		}
		else
		{
			statement("    float spv_NdotI = dot(", N, ", ", I, ");");
		}
		statement("    float spv_k = 1.0f - ", eta, " * ", eta, " * (1.0f - spv_NdotI * spv_NdotI);");
		statement("    if (spv_k >= 0.0f)");
		statement("        ", res, " = ", eta, " * ", I, " - (", eta, " * spv_NdotI + sqrt(spv_k)) * ", N, ";");
		statement("}");
		break;
	}

	case GLSLstd450Length:
	{
		auto &type = expression_type(args[0]);
		if (type.vecsize == 1)
			emit_unary_func_op(result_type, result_id, args[0], "fabs");
		else
			emit_unary_func_op(result_type, result_id, args[0], "length");
		break;
	}

	case GLSLstd450Distance:
	{
		auto &type = expression_type(args[0]);
		if (type.vecsize == 1)
		{
			auto expr = join("fabs(", to_expression(args[0]), " - ", to_expression(args[1]), ")");
			emit_op(result_type, result_id, expr, should_forward(args[0]) && should_forward(args[1]));
			inherit_expression_dependencies(result_id, args[0]);
			inherit_expression_dependencies(result_id, args[1]);
		}
		else
			emit_binary_func_op(result_type, result_id, args[0], args[1], "distance");
		break;
	}

	case GLSLstd450Normalize:
	{
		auto &type = expression_type(args[0]);
		if (type.vecsize == 1)
			emit_unary_func_op(result_type, result_id, args[0], "sign");
		else
			emit_unary_func_op(result_type, result_id, args[0], "normalize");
		break;
	}

	case GLSLstd450PackSnorm4x8:
		if (!needs_pack_snorm_4x8)
		{
			needs_pack_snorm_4x8 = true;
			force_recompile();
		}
		emit_unary_func_op(result_type, result_id, args[0], "spvPackSnorm4x8");
		break;
	case GLSLstd450PackUnorm4x8:
		if (!needs_pack_unorm_4x8)
		{
			needs_pack_unorm_4x8 = true;
			force_recompile();
		}
		emit_unary_func_op(result_type, result_id, args[0], "spvPackUnorm4x8");
		break;
	case GLSLstd450PackSnorm2x16:
		if (!needs_pack_snorm_2x16)
		{
			needs_pack_snorm_2x16 = true;
			force_recompile();
		}
		emit_unary_func_op(result_type, result_id, args[0], "spvPackSnorm2x16");
		break;
	case GLSLstd450PackUnorm2x16:
		if (!needs_pack_unorm_2x16)
		{
			needs_pack_unorm_2x16 = true;
			force_recompile();
		}
		emit_unary_func_op(result_type, result_id, args[0], "spvPackUnorm2x16");
		break;
	case GLSLstd450UnpackSnorm4x8:
		if (!needs_unpack_snorm_4x8)
		{
			needs_unpack_snorm_4x8 = true;
			force_recompile();
		}
		emit_unary_func_op(result_type, result_id, args[0], "spvUnpackSnorm4x8");
		break;
	case GLSLstd450UnpackUnorm4x8:
		if (!needs_unpack_unorm_4x8)
		{
			needs_unpack_unorm_4x8 = true;
			force_recompile();
		}
		emit_unary_func_op(result_type, result_id, args[0], "spvUnpackUnorm4x8");
		break;
	case GLSLstd450UnpackSnorm2x16:
		if (!needs_unpack_snorm_2x16)
		{
			needs_unpack_snorm_2x16 = true;
			force_recompile();
		}
		emit_unary_func_op(result_type, result_id, args[0], "spvUnpackSnorm2x16");
		break;
	case GLSLstd450UnpackUnorm2x16:
		if (!needs_unpack_unorm_2x16)
		{
			needs_unpack_unorm_2x16 = true;
			force_recompile();
		}
		emit_unary_func_op(result_type, result_id, args[0], "spvUnpackUnorm2x16");
		break;

	case GLSLstd450Determinant:
	{
		auto *e = maybe_get<SPIRExpression>(args[0]);
		bool old_transpose = e && e->need_transpose;
		if (old_transpose)
			e->need_transpose = false;

		auto &type = expression_type(args[0]);
		assert(type.vecsize == type.columns);
		const char *func = "spvDeterminant2";
		if (type.vecsize == 2)
		{
			if (!needs_determinant_2)
			{
				needs_determinant_2 = true;
				force_recompile();
			}
		}
		else if (type.vecsize == 3)
		{
			func = "spvDeterminant3";
			if (!needs_determinant_3)
			{
				needs_determinant_3 = true;
				force_recompile();
			}
		}
		else if (type.vecsize == 4)
		{
			func = "spvDeterminant4";
			if (!needs_determinant_4)
			{
				needs_determinant_4 = true;
				force_recompile();
			}
		}

		emit_unary_func_op(result_type, result_id, args[0], func);

		if (old_transpose)
			e->need_transpose = true;
		break;
	}

	case GLSLstd450MatrixInverse:
	{
		auto *a = maybe_get<SPIRExpression>(args[0]);
		bool old_transpose = a && a->need_transpose;
		if (old_transpose)
			a->need_transpose = false;

		auto &type = get<SPIRType>(result_type);
		assert(type.vecsize == type.columns);
		const char *inv_func = "spvInverse2";
		if (type.vecsize == 2)
		{
			if (!needs_inverse_2)
			{
				needs_inverse_2 = true;
				force_recompile();
			}
		}
		else if (type.vecsize == 3)
		{
			inv_func = "spvInverse3";
			if (!needs_inverse_3)
			{
				needs_inverse_3 = true;
				force_recompile();
			}
		}
		else if (type.vecsize == 4)
		{
			inv_func = "spvInverse4";
			if (!needs_inverse_4)
			{
				needs_inverse_4 = true;
				force_recompile();
			}
		}

		bool forward = should_forward(args[0]);
		auto &expr_out =
		    emit_op(result_type, result_id, join(inv_func, "(", to_unpacked_expression(args[0]), ")"), forward);
		inherit_expression_dependencies(result_id, args[0]);

		if (old_transpose)
		{
			expr_out.need_transpose = true;
			a->need_transpose = true;
		}
		break;
	}

	// NMin / NMax / NClamp: OpenCL fmin/fmax propagate NaN correctly, use them directly.
	case GLSLstd450NMin:
		emit_binary_func_op(result_type, result_id, args[0], args[1], "fmin");
		break;
	case GLSLstd450NMax:
		emit_binary_func_op(result_type, result_id, args[0], args[1], "fmax");
		break;
	case GLSLstd450NClamp:
		emit_trinary_func_op(result_type, result_id, args[0], args[1], args[2], "clamp");
		break;

	case GLSLstd450Frexp:
	{
		// OpenCL frexp signature matches GLSL: frexp(x, &exp)
		register_call_out_argument(args[1]);
		forced_temporaries.insert(result_id);
		emit_op(result_type, result_id, join("frexp(", to_expression(args[0]), ", &", to_expression(args[1]), ")"),
		        false);
		break;
	}

	case GLSLstd450FrexpStruct:
	{
		auto &type = get<SPIRType>(result_type);
		emit_uninitialized_temporary_expression(result_type, result_id);
		statement(to_expression(result_id), ".", to_member_name(type, 0), " = frexp(", to_expression(args[0]), ", &",
		          to_expression(result_id), ".", to_member_name(type, 1), ");");
		break;
	}

	case GLSLstd450Ldexp:
		emit_binary_func_op(result_type, result_id, args[0], args[1], "ldexp");
		break;

	case GLSLstd450Cross:
		emit_binary_func_op(result_type, result_id, args[0], args[1], "cross");
		break;

	case GLSLstd450FSign:
		emit_unary_func_op(result_type, result_id, args[0], "sign");
		break;

	case GLSLstd450FAbs:
		emit_unary_func_op(result_type, result_id, args[0], "fabs");
		break;

	case GLSLstd450FMin:
		emit_binary_func_op(result_type, result_id, args[0], args[1], "fmin");
		break;
	case GLSLstd450FMax:
		emit_binary_func_op(result_type, result_id, args[0], args[1], "fmax");
		break;
	case GLSLstd450FClamp:
		emit_trinary_func_op(result_type, result_id, args[0], args[1], args[2], "clamp");
		break;
	case GLSLstd450SMin:
	{
		auto int_type = to_signed_basetype(get_integer_width_for_glsl_instruction(glsl_op, args, count));
		emit_binary_func_op_cast(result_type, result_id, args[0], args[1], "min", int_type, false);
		break;
	}
	case GLSLstd450SMax:
	{
		auto int_type = to_signed_basetype(get_integer_width_for_glsl_instruction(glsl_op, args, count));
		emit_binary_func_op_cast(result_type, result_id, args[0], args[1], "max", int_type, false);
		break;
	}
	case GLSLstd450UMin:
	{
		auto uint_type = to_unsigned_basetype(get_integer_width_for_glsl_instruction(glsl_op, args, count));
		emit_binary_func_op_cast(result_type, result_id, args[0], args[1], "min", uint_type, false);
		break;
	}
	case GLSLstd450UMax:
	{
		auto uint_type = to_unsigned_basetype(get_integer_width_for_glsl_instruction(glsl_op, args, count));
		emit_binary_func_op_cast(result_type, result_id, args[0], args[1], "max", uint_type, false);
		break;
	}
	case GLSLstd450SClamp:
	{
		auto int_type = to_signed_basetype(get_integer_width_for_glsl_instruction(glsl_op, args, count));
		emit_trinary_func_op_cast(result_type, result_id, args[0], args[1], args[2], "clamp", int_type);
		break;
	}
	case GLSLstd450UClamp:
	{
		auto uint_type = to_unsigned_basetype(get_integer_width_for_glsl_instruction(glsl_op, args, count));
		emit_trinary_func_op_cast(result_type, result_id, args[0], args[1], args[2], "clamp", uint_type);
		break;
	}

	case GLSLstd450FMix:
	case GLSLstd450IMix:
		emit_trinary_func_op(result_type, result_id, args[0], args[1], args[2], "mix");
		break;
	case GLSLstd450Step:
		emit_binary_func_op(result_type, result_id, args[0], args[1], "step");
		break;
	case GLSLstd450SmoothStep:
		emit_trinary_func_op(result_type, result_id, args[0], args[1], args[2], "smoothstep");
		break;
	case GLSLstd450Fma:
		emit_trinary_func_op(result_type, result_id, args[0], args[1], args[2], "fma");
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

	// Pointer types are handled by emit_instruction for OpBitcast.
	// If we get here as a fallback, use a simple C-style cast.
	if (is_pointer(out_type))
		return join("(", type_to_glsl(out_type), ")");
	if (is_pointer(in_type))
		return "as_ulong";

	// All bitcasts (float↔int, int↔uint, half↔short, etc.) use as_TYPE() in OpenCL C.
	// type_to_glsl gives us the full type name including vector size (e.g. "float4", "uint").
	auto out_name = type_to_glsl(out_type);
	return "as_" + out_name;
}

bool CompilerOpenCL::emit_complex_bitcast(uint32_t result_type, uint32_t id, uint32_t op0)
{
	auto &output_type = get<SPIRType>(result_type);
	auto &input_type = expression_type(op0);
	string expr;

	// float → half2 bitcast: as_half2(expr)
	if (output_type.basetype == SPIRType::Half && input_type.basetype == SPIRType::Float && input_type.vecsize == 1)
		expr = join("as_half2(", to_unpacked_expression(op0), ")");
	// half2 → float bitcast: as_float(expr)
	else if (output_type.basetype == SPIRType::Float && input_type.basetype == SPIRType::Half &&
	         input_type.vecsize == 2)
		expr = join("as_float(", to_unpacked_expression(op0), ")");
	else
		return false;

	emit_op(result_type, id, expr, should_forward(op0));
	return true;
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

bool CompilerOpenCL::prepare_access_chain_for_scalar_access(std::string &expr, const SPIRType &type,
                                                            StorageClass storage, bool &is_packed)
{
	// In OpenCL C, you cannot take the address of a vector component (e.g. &vec.x is invalid).
	// Cast the vector expression to a scalar pointer so that element access uses array indexing.
	if (storage == StorageClassStorageBuffer || storage == StorageClassWorkgroup)
	{
		const char *addr_space = storage == StorageClassWorkgroup ? "__local" : "__global";
		expr = join("((", addr_space, " ", type_to_glsl(type), "*)&", enclose_expression(expr), ")");
		is_packed = true;
		return true;
	}
	else
		return false;
}

void CompilerOpenCL::emit_binary_ptr_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1,
                                        const char *op)
{
	bool forward = should_forward(op0) && should_forward(op1);
	emit_op(result_type, result_id, join(to_ptr_expression(op0), " ", op, " ", to_ptr_expression(op1)), forward);
	inherit_expression_dependencies(result_id, op0);
	inherit_expression_dependencies(result_id, op1);
}

string CompilerOpenCL::to_ptr_expression(uint32_t id, bool register_expression_read)
{
	auto *e = maybe_get<SPIRExpression>(id);
	// If need_transpose is set, bypass the transpose wrapper and use the raw expression,
	// since we're taking the address and comparing pointers, not values.
	auto expr =
	    enclose_expression(e && e->need_transpose ? e->expression : to_expression(id, register_expression_read));
	if (!should_dereference(id))
		expr = address_of_expression(expr);
	return expr;
}

// Task #3: In OpenCL C, pointer-to-struct member access uses -> instead of .
// ptr_chain_is_resolved == false means this is the first member access from the base.
bool CompilerOpenCL::should_dereference(uint32_t id)
{
	// In OpenCL C, function parameters with StorageClassFunction pointer types
	// are emitted as actual pointers (T*), so they need dereferencing for
	// member/component access (e.g., (*a).x instead of a.x).
	const auto &type = expression_type(id);
	if (is_pointer(type) && type.storage == StorageClassFunction)
	{
		auto *var = maybe_get<SPIRVariable>(id);
		if (var && var->parameter != nullptr)
			return true;
	}
	return CompilerGLSL::should_dereference(id);
}

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
			if (sc == StorageClassStorageBuffer || sc == StorageClassCrossWorkgroup ||
			    sc == StorageClassPhysicalStorageBuffer)
			{
				return join("->", to_member_name(type, index));
			}
			// StorageClassUniform with BufferBlock decoration is a legacy SSBO (GLSL 430 style),
			// emitted as __global T* in OpenCL C — use ->.
			// Plain Uniform with Block decoration is a UBO, emitted by value — use '.'.
			if (sc == StorageClassUniform)
			{
				auto *var = maybe_get_backing_variable(base);
				if (var)
				{
					auto &var_type = get<SPIRType>(var->basetype);
					if (has_decoration(var_type.self, DecorationBufferBlock))
						return join("->", to_member_name(type, index));
				}
			}
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
					    // If the sole member is a BDA pointer, type_to_glsl would return
					    // `__global Ptr*` which, wrapped in `__global const X*`, yields
					    // double `__global` and pointer-to-pointer. Flatten to `ulong`
					    // instead, matching how emit_struct_member stores BDA pointers.
					    if (is_pointer(member0_type) && member0_type.storage == StorageClassPhysicalStorageBuffer)
						    return std::string("ulong");
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

	// Emit #define macros right before the function prototype for workgroup scalar pointer aliasing.
	// This must happen here (not in emit_function) because CompilerGLSL::emit_function recursively
	// emits callee functions before reaching emit_function_prototype, so #define in emit_function
	// would be undone by callee #undef before this function's body is emitted.
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

void CompilerOpenCL::emit_function(SPIRFunction &func, const Bitset &return_flags)
{
	CompilerGLSL::emit_function(func, return_flags);

	// Emit #undef after the function body.
	// The matching #define is emitted in emit_function_prototype.
	auto wg_it = func_workgroup_args.find(func.self);
	if (wg_it != func_workgroup_args.end())
	{
		bool has_defines = false;
		for (auto var_id : wg_it->second)
		{
			if (workgroup_scalar_vars.count(var_id))
			{
				statement("#undef ", to_name(var_id));
				has_defines = true;
			}
		}
		if (has_defines)
			statement("");
	}
}

void CompilerOpenCL::emit_store_statement(uint32_t lhs_expression, uint32_t rhs_expression)
{
	auto &type = expression_type(rhs_expression);
	auto *lhs_e = maybe_get<SPIRExpression>(lhs_expression);

	// In OpenCL C, we cannot assign to a function return value (rvalue).
	// The base class wraps the LHS in convert_row_major_matrix() which produces
	// spvTranspose(lhs) = rhs, which is invalid C.
	// Instead, transpose the RHS and store directly to the LHS.
	if (is_matrix(type) && lhs_e && lhs_e->need_transpose)
	{
		lhs_e->need_transpose = false;

		auto *rhs_e = maybe_get<SPIRExpression>(rhs_expression);
		if (rhs_e && rhs_e->need_transpose)
		{
			// Both sides need transpose — they cancel out.
			rhs_e->need_transpose = false;
			statement(to_expression(lhs_expression), " = ", to_unpacked_row_major_matrix_expression(rhs_expression),
			          ";");
			rhs_e->need_transpose = true;
		}
		else
		{
			// Transpose the RHS before storing.
			auto &rhs_type = expression_type(rhs_expression);
			auto rhs_short = opencl_matrix_short_name(rhs_type.basetype, rhs_type.vecsize, rhs_type.columns);
			MatrixTypeKey rhs_key = { rhs_type.basetype, rhs_type.vecsize, rhs_type.columns };
			need_transpose.insert(rhs_key);
			used_matrix_types.insert(rhs_key);
			// The LHS is in physical (transposed) layout, so we transpose the logical RHS to physical.
			statement(to_expression(lhs_expression), " = spvTranspose", rhs_short, "(",
			          to_unpacked_expression(rhs_expression), ");");
		}

		lhs_e->need_transpose = true;
		register_write(lhs_expression);
	}
	else if (lhs_e && lhs_e->need_transpose)
	{
		// Storing a column to a row-major matrix. Unroll the write.
		lhs_e->need_transpose = false;
		for (uint32_t c = 0; c < type.vecsize; c++)
		{
			auto lhs_expr = to_dereferenced_expression(lhs_expression);
			auto column_index = lhs_expr.find_last_of('[');
			if (column_index != string::npos)
			{
				statement(lhs_expr.insert(column_index, join('[', c, ']')), " = ",
				          to_extract_component_expression(rhs_expression, c), ";");
			}
		}
		lhs_e->need_transpose = true;
		register_write(lhs_expression);
	}
	else
	{
		// Check if storing an array type — C does not allow `array = expr;`.
		auto &rhs_type_raw = expression_type(rhs_expression);
		auto &rhs_type = is_pointer(rhs_type_raw) ? get_pointee_type(rhs_type_raw) : rhs_type_raw;
		if (is_array(rhs_type))
		{
			auto *var = maybe_get<SPIRVariable>(lhs_expression);
			// For deferred declarations where the RHS is a composite construct
			// (not loaded from memory), C99 allows `T arr[N] = { ... };`.
			// Let the base class handle that case — it merges decl + init correctly.
			auto *rhs_expr_node = maybe_get<SPIRExpression>(rhs_expression);
			bool rhs_from_memory = rhs_expr_node && rhs_expr_node->loaded_from;
			if (var && var->deferred_declaration && !rhs_from_memory)
			{
				// Base class will emit `T arr[N] = { ... };`
				CompilerGLSL::emit_store_statement(lhs_expression, rhs_expression);
				return;
			}

			// Flush deferred declaration so we don't get "float a[5] = rhs".
			if (var && var->deferred_declaration)
			{
				var->deferred_declaration = false;
				statement(variable_decl_function_local(*var), ";");
			}
			auto lhs_expr = to_dereferenced_expression(lhs_expression);
			emit_array_copy(lhs_expr.c_str(), 0, rhs_expression, StorageClassFunction, StorageClassFunction);
			register_write(lhs_expression);
			return;
		}
		CompilerGLSL::emit_store_statement(lhs_expression, rhs_expression);
	}
}

void CompilerOpenCL::emit_struct_member(const SPIRType &type, uint32_t member_type_id, uint32_t index,
                                        const string &qualifier, uint32_t)
{
	auto &membertype = get<SPIRType>(member_type_id);
	// OpenCL C does not use GLSL layout qualifiers or interpolation qualifiers.
	// PhysicalStorageBuffer pointers in structs must be emitted as ulong since
	// OpenCL C does not allow pointer types in kernel parameter structs.
	// Walk through array dimensions to find the inner element type, so that
	// array-of-pointer members (e.g. `Ptr* ptrs[2]`) are also caught.
	auto *inner = &membertype;
	while (is_array(*inner))
		inner = &get<SPIRType>(inner->parent_type);
	if (is_pointer(*inner) && inner->storage == StorageClassPhysicalStorageBuffer)
	{
		statement(qualifier, "ulong ", to_member_name(type, index), type_to_array_glsl(membertype, 0), ";");
	}
	else if (has_member_decoration(type.self, index, DecorationRowMajor))
	{
		// Row-major matrix: the physical layout has transposed dimensions.
		// Emit the member with the physical (transposed) type so struct layout matches buffer.
		// `inner` already points to the innermost non-array type from the BDA check above.
		if (inner->columns > 1)
		{
			auto phys_type_name = opencl_matrix_type_name(inner->basetype, inner->columns, inner->vecsize);
			MatrixTypeKey phys_key = { inner->basetype, inner->columns, inner->vecsize };
			used_matrix_types.insert(phys_key);

			statement(qualifier, phys_type_name, " ", to_member_name(type, index), type_to_array_glsl(membertype, 0),
			          ";");
		}
		else
		{
			// Not actually a matrix member, fall through to default.
			statement(qualifier, variable_decl(membertype, to_member_name(type, index)), ";");
		}
	}
	else
	{
		statement(qualifier, variable_decl(membertype, to_member_name(type, index)), ";");
	}
}

void CompilerOpenCL::emit_block_hints(const SPIRBlock &)
{
	// OpenCL C has no control-flow hint attributes; suppress SPIRV_CROSS_BRANCH/FLATTEN etc.
}

// Emit a unary subgroup op, decomposing vectors into per-component calls.
// For scalars, emits: func(val)
// For vectors, emits: (vectype)(func(val.x), func(val.y), ...)
void CompilerOpenCL::emit_subgroup_op_vec(uint32_t result_type, uint32_t id, uint32_t value_id, const char *func_name)
{
	auto &type = expression_type(value_id);
	if (type.vecsize > 1)
	{
		auto &out_type = get<SPIRType>(result_type);
		string expr = "(" + type_to_glsl(out_type) + ")(";
		for (uint32_t c = 0; c < type.vecsize; c++)
		{
			if (c > 0)
				expr += ", ";
			expr += join(func_name, "(", to_enclosed_expression(value_id), ".", "xyzw"[c], ")");
		}
		expr += ")";
		emit_op(result_type, id, expr, should_forward(value_id));
		inherit_expression_dependencies(id, value_id);
	}
	else
	{
		emit_unary_func_op(result_type, id, value_id, func_name);
	}
}

// Emit a binary subgroup op (value + extra arg like cluster size), decomposing vectors.
// For scalars, emits: func(val, extra)
// For vectors, emits: (vectype)(func(val.x, extra), func(val.y, extra), ...)
void CompilerOpenCL::emit_subgroup_op_vec_binary(uint32_t result_type, uint32_t id, uint32_t value_id,
                                                 uint32_t extra_id, const char *func_name)
{
	auto &type = expression_type(value_id);
	if (type.vecsize > 1)
	{
		auto &out_type = get<SPIRType>(result_type);
		string extra_expr = to_expression(extra_id);
		string expr = "(" + type_to_glsl(out_type) + ")(";
		for (uint32_t c = 0; c < type.vecsize; c++)
		{
			if (c > 0)
				expr += ", ";
			expr += join(func_name, "(", to_enclosed_expression(value_id), ".", "xyzw"[c], ", ", extra_expr, ")");
		}
		expr += ")";
		emit_op(result_type, id, expr, should_forward(value_id));
		inherit_expression_dependencies(id, value_id);
	}
	else
	{
		emit_binary_func_op(result_type, id, value_id, extra_id, func_name);
	}
}

void CompilerOpenCL::emit_subgroup_op(const Instruction &i)
{
	const uint32_t *ops = stream(i);
	auto op = static_cast<Op>(i.op);

	if (!opencl_options.enable_subgroups)
		SPIRV_CROSS_THROW("Subgroup operations require enable_subgroups option.");

	// Validate scope is Subgroup
	if (op != OpGroupNonUniformQuadAllKHR && op != OpGroupNonUniformQuadAnyKHR)
	{
		auto scope = static_cast<Scope>(evaluate_constant_u32(ops[2]));
		if (scope != ScopeSubgroup)
			SPIRV_CROSS_THROW("Only subgroup scope is supported.");
	}

	uint32_t result_type = ops[0];
	uint32_t id = ops[1];

	// If we need to do implicit bitcasts, make sure we do it with the correct type.
	uint32_t integer_width = get_integer_width_for_instruction(i);
	auto int_type = to_signed_basetype(integer_width);
	auto uint_type = to_unsigned_basetype(integer_width);

	// Helper to set an extension flag and trigger recompile if newly needed.
	auto require_extension = [this](bool &flag)
	{
		if (!flag)
		{
			flag = true;
			force_recompile();
		}
	};

	switch (op)
	{
		// === Task 5: cl_khr_subgroup_non_uniform_vote ===

	case OpGroupNonUniformElect:
		require_extension(needs_subgroup_vote);
		emit_op(result_type, id, "sub_group_elect()", true);
		break;

	case OpGroupNonUniformAllEqual:
	{
		require_extension(needs_subgroup_vote);
		auto &type = expression_type(ops[3]);
		if (type.vecsize > 1)
		{
			// OpenCL sub_group_non_uniform_all_equal only accepts scalars.
			// For vectors, decompose into per-component calls combined with &&.
			string expr;
			for (uint32_t c = 0; c < type.vecsize; c++)
			{
				if (c > 0)
					expr += " && ";
				string component = join(to_enclosed_expression(ops[3]), ".", "xyzw"[c]);
				expr += join("sub_group_non_uniform_all_equal(", component, ")");
			}
			emit_op(result_type, id, expr, should_forward(ops[3]));
			inherit_expression_dependencies(id, ops[3]);
		}
		else
		{
			emit_unary_func_op(result_type, id, ops[3], "sub_group_non_uniform_all_equal");
		}
		break;
	}

		// === Task 4: cl_khr_subgroups (base) — vote/broadcast ===

	case OpGroupNonUniformAll:
		emit_unary_func_op(result_type, id, ops[3], "sub_group_all");
		break;

	case OpGroupNonUniformAny:
		emit_unary_func_op(result_type, id, ops[3], "sub_group_any");
		break;

	case OpGroupNonUniformBroadcast:
		emit_subgroup_op_vec_binary(result_type, id, ops[3], ops[4], "sub_group_broadcast");
		break;

		// === Task 6: cl_khr_subgroup_ballot ===

	case OpGroupNonUniformBroadcastFirst:
		require_extension(needs_subgroup_ballot);
		emit_subgroup_op_vec(result_type, id, ops[3], "sub_group_broadcast_first");
		break;

	case OpGroupNonUniformBallot:
		require_extension(needs_subgroup_ballot);
		emit_unary_func_op(result_type, id, ops[3], "sub_group_ballot");
		break;

	case OpGroupNonUniformInverseBallot:
		require_extension(needs_subgroup_ballot);
		emit_unary_func_op(result_type, id, ops[3], "sub_group_inverse_ballot");
		break;

	case OpGroupNonUniformBallotBitExtract:
		require_extension(needs_subgroup_ballot);
		emit_binary_func_op(result_type, id, ops[3], ops[4], "sub_group_ballot_bit_extract");
		break;

	case OpGroupNonUniformBallotFindLSB:
		require_extension(needs_subgroup_ballot);
		emit_unary_func_op(result_type, id, ops[3], "sub_group_ballot_find_lsb");
		break;

	case OpGroupNonUniformBallotFindMSB:
		require_extension(needs_subgroup_ballot);
		emit_unary_func_op(result_type, id, ops[3], "sub_group_ballot_find_msb");
		break;

	case OpGroupNonUniformBallotBitCount:
	{
		require_extension(needs_subgroup_ballot);
		auto operation = static_cast<GroupOperation>(ops[3]);
		if (operation == GroupOperationReduce)
			emit_unary_func_op(result_type, id, ops[4], "sub_group_ballot_bit_count");
		else if (operation == GroupOperationInclusiveScan)
			emit_unary_func_op(result_type, id, ops[4], "sub_group_ballot_inclusive_scan");
		else if (operation == GroupOperationExclusiveScan)
			emit_unary_func_op(result_type, id, ops[4], "sub_group_ballot_exclusive_scan");
		else
			SPIRV_CROSS_THROW("Invalid group operation for BallotBitCount.");
		break;
	}

	// === Tasks 4/7/10: Arithmetic ops (Reduce/Scan/Clustered) ===
	// The same SPIR-V opcodes are used for base cl_khr_subgroups (Reduce/InclusiveScan/ExclusiveScan
	// with add/min/max), cl_khr_subgroup_non_uniform_arithmetic (all ops with Reduce/Scan),
	// and cl_khr_subgroup_clustered_reduce (ClusteredReduce).

	// clang-format off
	// OpenCL subgroup functions are scalar-only; vectors are decomposed per-component
	// via emit_subgroup_op_vec / emit_subgroup_op_vec_binary.

#define OPENCL_SUBGROUP_ARITH(spirv_op, base_name, nu_name) \
	case OpGroupNonUniform##spirv_op: \
	{ \
		auto operation = static_cast<GroupOperation>(ops[3]); \
		if (operation == GroupOperationReduce) \
			emit_subgroup_op_vec(result_type, id, ops[4], "sub_group_reduce_" base_name); \
		else if (operation == GroupOperationInclusiveScan) \
			emit_subgroup_op_vec(result_type, id, ops[4], "sub_group_scan_inclusive_" base_name); \
		else if (operation == GroupOperationExclusiveScan) \
			emit_subgroup_op_vec(result_type, id, ops[4], "sub_group_scan_exclusive_" base_name); \
		else if (operation == GroupOperationClusteredReduce) \
		{ \
			require_extension(needs_subgroup_clustered); \
			emit_subgroup_op_vec_binary(result_type, id, ops[4], ops[5], "sub_group_clustered_reduce_" base_name); \
		} \
		else \
			SPIRV_CROSS_THROW("Unsupported group operation."); \
		break; \
	}

#define OPENCL_SUBGROUP_ARITH_CAST(spirv_op, base_name, nu_name, cast_type) \
	case OpGroupNonUniform##spirv_op: \
	{ \
		auto operation = static_cast<GroupOperation>(ops[3]); \
		if (operation == GroupOperationReduce) \
			emit_unary_func_op_cast(result_type, id, ops[4], "sub_group_reduce_" base_name, cast_type, cast_type); \
		else if (operation == GroupOperationInclusiveScan) \
			emit_unary_func_op_cast(result_type, id, ops[4], "sub_group_scan_inclusive_" base_name, cast_type, cast_type); \
		else if (operation == GroupOperationExclusiveScan) \
			emit_unary_func_op_cast(result_type, id, ops[4], "sub_group_scan_exclusive_" base_name, cast_type, cast_type); \
		else if (operation == GroupOperationClusteredReduce) \
		{ \
			require_extension(needs_subgroup_clustered); \
			emit_subgroup_op_vec_binary(result_type, id, ops[4], ops[5], "sub_group_clustered_reduce_" base_name); \
		} \
		else \
			SPIRV_CROSS_THROW("Unsupported group operation."); \
		break; \
	}

	// Non-uniform arithmetic extension ops (mul, bitwise, logical) — always require the extension
#define OPENCL_SUBGROUP_ARITH_NU(spirv_op, nu_name) \
	case OpGroupNonUniform##spirv_op: \
	{ \
		auto operation = static_cast<GroupOperation>(ops[3]); \
		if (operation == GroupOperationReduce) \
		{ \
			require_extension(needs_subgroup_arithmetic); \
			emit_subgroup_op_vec(result_type, id, ops[4], "sub_group_non_uniform_reduce_" nu_name); \
		} \
		else if (operation == GroupOperationInclusiveScan) \
		{ \
			require_extension(needs_subgroup_arithmetic); \
			emit_subgroup_op_vec(result_type, id, ops[4], "sub_group_non_uniform_scan_inclusive_" nu_name); \
		} \
		else if (operation == GroupOperationExclusiveScan) \
		{ \
			require_extension(needs_subgroup_arithmetic); \
			emit_subgroup_op_vec(result_type, id, ops[4], "sub_group_non_uniform_scan_exclusive_" nu_name); \
		} \
		else if (operation == GroupOperationClusteredReduce) \
		{ \
			require_extension(needs_subgroup_clustered); \
			emit_subgroup_op_vec_binary(result_type, id, ops[4], ops[5], "sub_group_clustered_reduce_" nu_name); \
		} \
		else \
			SPIRV_CROSS_THROW("Unsupported group operation."); \
		break; \
	}

	// add/min/max: base cl_khr_subgroups for Reduce/Scan, clustered for ClusteredReduce
	OPENCL_SUBGROUP_ARITH(FAdd, "add", "add")
	OPENCL_SUBGROUP_ARITH(IAdd, "add", "add")
	OPENCL_SUBGROUP_ARITH(FMin, "min", "min")
	OPENCL_SUBGROUP_ARITH(FMax, "max", "max")
	OPENCL_SUBGROUP_ARITH_CAST(SMin, "min", "min", int_type)
	OPENCL_SUBGROUP_ARITH_CAST(SMax, "max", "max", int_type)
	OPENCL_SUBGROUP_ARITH_CAST(UMin, "min", "min", uint_type)
	OPENCL_SUBGROUP_ARITH_CAST(UMax, "max", "max", uint_type)

	// mul/bitwise/logical: always require cl_khr_subgroup_non_uniform_arithmetic (or clustered)
	OPENCL_SUBGROUP_ARITH_NU(FMul, "mul")
	OPENCL_SUBGROUP_ARITH_NU(IMul, "mul")
	OPENCL_SUBGROUP_ARITH_NU(BitwiseAnd, "and")
	OPENCL_SUBGROUP_ARITH_NU(BitwiseOr, "or")
	OPENCL_SUBGROUP_ARITH_NU(BitwiseXor, "xor")
	OPENCL_SUBGROUP_ARITH_NU(LogicalAnd, "logical_and")
	OPENCL_SUBGROUP_ARITH_NU(LogicalOr, "logical_or")
	OPENCL_SUBGROUP_ARITH_NU(LogicalXor, "logical_xor")

#undef OPENCL_SUBGROUP_ARITH
#undef OPENCL_SUBGROUP_ARITH_CAST
#undef OPENCL_SUBGROUP_ARITH_NU
		// clang-format on

		// === Task 8: cl_khr_subgroup_shuffle ===

	case OpGroupNonUniformShuffle:
		require_extension(needs_subgroup_shuffle);
		emit_binary_func_op(result_type, id, ops[3], ops[4], "sub_group_shuffle");
		break;

	case OpGroupNonUniformShuffleXor:
		require_extension(needs_subgroup_shuffle);
		emit_binary_func_op(result_type, id, ops[3], ops[4], "sub_group_shuffle_xor");
		break;

		// === Task 9: cl_khr_subgroup_shuffle_relative ===

	case OpGroupNonUniformShuffleUp:
		require_extension(needs_subgroup_shuffle_relative);
		emit_binary_func_op(result_type, id, ops[3], ops[4], "sub_group_shuffle_up");
		break;

	case OpGroupNonUniformShuffleDown:
		require_extension(needs_subgroup_shuffle_relative);
		emit_binary_func_op(result_type, id, ops[3], ops[4], "sub_group_shuffle_down");
		break;

		// === Task 11: cl_khr_subgroup_rotate ===

	case OpGroupNonUniformRotateKHR:
		require_extension(needs_subgroup_rotate);
		if (i.length > 5)
			emit_trinary_func_op(result_type, id, ops[3], ops[4], ops[5], "sub_group_clustered_rotate");
		else
			emit_binary_func_op(result_type, id, ops[3], ops[4], "sub_group_rotate");
		break;

	default:
		SPIRV_CROSS_THROW("Unsupported subgroup op for OpenCL.");
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
	auto &raw_type = expression_type(rhs_id);
	// If the RHS is a pointer (e.g., from OpLoad source), use the pointee type.
	auto &type = is_pointer(raw_type) ? get_pointee_type(raw_type) : raw_type;

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

	// For constant RHS, `to_expression` returns `{ 1.0f, 2.0f, ... }` and
	// subscripting that (`{ ... }[0]`) is not valid C. Extract sub-constants.
	auto *constant = maybe_get<SPIRConstant>(rhs_id);
	if (constant && !constant->subconstants.empty())
	{
		for (uint32_t i = 0; i < array_size; i++)
			statement(lhs, "[", i, "] = ", to_expression(constant->subconstants[i]), ";");
	}
	else
	{
		for (uint32_t i = 0; i < array_size; i++)
			statement(lhs, "[", i, "] = ", rhs_expr, "[", i, "];");
	}

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
			SPIRV_CROSS_THROW("Image atomics are not supported in OpenCL.");
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
		// When loading a PhysicalStorageBuffer pointer from a struct member that was
		// emitted as ulong (because OpenCL doesn't allow pointer types in kernel struct params),
		// cast the loaded ulong value to the typed pointer.
		{
			auto &result_type_obj = get<SPIRType>(ops[0]);
			if (is_pointer(result_type_obj) && result_type_obj.storage == StorageClassPhysicalStorageBuffer)
			{
				auto *expr = maybe_get<SPIRExpression>(ptr);
				if (expr && expr->access_chain)
				{
					uint32_t result_type = ops[0];
					uint32_t result_id = ops[1];
					auto ptr_type_str = type_to_glsl(result_type_obj);
					emit_op(result_type, result_id, join("((", ptr_type_str, ")(", to_expression(ptr), "))"),
					        should_forward(ptr));
					inherit_expression_dependencies(result_id, ptr);
					break;
				}
			}
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

	// OpOuterProduct: use struct-wrapped matrix helper.
	case OpOuterProduct:
	{
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t col_vec = ops[2]; // column vector
		uint32_t row_vec = ops[3]; // row vector
		auto &res_type = get<SPIRType>(result_type);
		auto &col_type = expression_type(col_vec);
		auto &row_type = expression_type(row_vec);

		need_outer_product.insert(make_matrix_key(res_type));
		// Ensure the result matrix type is registered.
		used_matrix_types.insert(make_matrix_key(res_type));

		auto col_short = opencl_vector_short_name(col_type.basetype, col_type.vecsize);
		auto row_short = opencl_vector_short_name(row_type.basetype, row_type.vecsize);
		string func_name = "spvOuterProduct" + col_short + row_short;

		emit_binary_func_op(result_type, result_id, col_vec, row_vec, func_name.c_str());
		break;
	}

	// Matrix arithmetic operations using struct-wrapped matrix helpers.
	case OpMatrixTimesVector:
	{
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t mat_id = ops[2];
		uint32_t vec_id = ops[3];
		auto &mat_type = expression_type(mat_id);

		auto *e = maybe_get<SPIRExpression>(mat_id);
		if (e && e->need_transpose)
		{
			// Transposed M * v = v * M_untransposed.
			// mat_type is the SPIR-V type (e.g., mat2x3 = 2 cols, vecsize=3).
			// The untransposed (physical) matrix is mat3x2 = 3 cols, vecsize=2.
			e->need_transpose = false;
			uint32_t phys_cols = mat_type.vecsize;
			uint32_t phys_rows = mat_type.columns;
			MatrixTypeKey phys_key = { mat_type.basetype, phys_rows, phys_cols };
			need_mul_vec_mat.insert(phys_key);
			used_matrix_types.insert(phys_key);

			auto vec_short = opencl_vector_short_name(mat_type.basetype, phys_rows);
			auto mat_short = opencl_matrix_short_name(mat_type.basetype, phys_rows, phys_cols);
			string func_name = "spvMul" + vec_short + mat_short;

			string expr =
			    join(func_name, "(", to_expression(vec_id), ", ", to_unpacked_row_major_matrix_expression(mat_id), ")");
			bool forward = should_forward(mat_id) && should_forward(vec_id);
			emit_op(result_type, result_id, expr, forward);
			e->need_transpose = true;
		}
		else
		{
			auto key = make_matrix_key(mat_type);
			need_mul_mat_vec.insert(key);

			auto mat_short = opencl_matrix_short_name(mat_type.basetype, mat_type.vecsize, mat_type.columns);
			auto vec_short = opencl_vector_short_name(mat_type.basetype, mat_type.columns);
			string func_name = "spvMul" + mat_short + vec_short;

			emit_binary_func_op(result_type, result_id, mat_id, vec_id, func_name.c_str());
		}
		inherit_expression_dependencies(result_id, mat_id);
		inherit_expression_dependencies(result_id, vec_id);
		break;
	}

	case OpVectorTimesMatrix:
	{
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t vec_id = ops[2];
		uint32_t mat_id = ops[3];
		auto &mat_type = expression_type(mat_id);

		auto *e = maybe_get<SPIRExpression>(mat_id);
		if (e && e->need_transpose)
		{
			// v * M^T = M_untransposed * v.
			// mat_type is the SPIR-V type (e.g., mat2x3 = 2 cols, vecsize=3).
			// The untransposed (physical) matrix is mat3x2 = 3 cols, vecsize=2.
			e->need_transpose = false;
			uint32_t phys_cols = mat_type.vecsize;
			uint32_t phys_rows = mat_type.columns;
			MatrixTypeKey phys_key = { mat_type.basetype, phys_rows, phys_cols };
			need_mul_mat_vec.insert(phys_key);
			used_matrix_types.insert(phys_key);

			auto mat_short = opencl_matrix_short_name(mat_type.basetype, phys_rows, phys_cols);
			auto vec_short = opencl_vector_short_name(mat_type.basetype, phys_rows);
			string func_name = "spvMul" + mat_short + vec_short;

			string expr =
			    join(func_name, "(", to_unpacked_row_major_matrix_expression(mat_id), ", ", to_expression(vec_id), ")");
			bool forward = should_forward(mat_id) && should_forward(vec_id);
			emit_op(result_type, result_id, expr, forward);
			e->need_transpose = true;
		}
		else
		{
			auto key = make_matrix_key(mat_type);
			need_mul_vec_mat.insert(key);

			auto vec_short = opencl_vector_short_name(mat_type.basetype, mat_type.vecsize);
			auto mat_short = opencl_matrix_short_name(mat_type.basetype, mat_type.vecsize, mat_type.columns);
			string func_name = "spvMul" + vec_short + mat_short;

			emit_binary_func_op(result_type, result_id, vec_id, mat_id, func_name.c_str());
		}
		inherit_expression_dependencies(result_id, vec_id);
		inherit_expression_dependencies(result_id, mat_id);
		break;
	}

	case OpMatrixTimesMatrix:
	{
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t a_id = ops[2];
		uint32_t b_id = ops[3];
		auto &a_type = expression_type(a_id);
		auto &b_type = expression_type(b_id);

		auto *ea = maybe_get<SPIRExpression>(a_id);
		auto *eb = maybe_get<SPIRExpression>(b_id);

		if (ea && eb && ea->need_transpose && eb->need_transpose)
		{
			// (A^T * B^T) = (B * A)^T
			// Physical (untransposed) matrices have swapped dimensions.
			ea->need_transpose = false;
			eb->need_transpose = false;

			MatrixTypeKey phys_b = { b_type.basetype, b_type.columns, b_type.vecsize };
			MatrixTypeKey phys_a = { a_type.basetype, a_type.columns, a_type.vecsize };
			need_mul_mat_mat.insert({ phys_b, phys_a });
			need_mul_mat_vec.insert(phys_b);
			used_matrix_types.insert(phys_b);
			used_matrix_types.insert(phys_a);

			auto mat_b_short = opencl_matrix_short_name(phys_b.basetype, phys_b.vecsize, phys_b.columns);
			auto mat_a_short = opencl_matrix_short_name(phys_a.basetype, phys_a.vecsize, phys_a.columns);
			string func_name = "spvMul" + mat_b_short + mat_a_short;

			string expr = join(func_name, "(", to_unpacked_row_major_matrix_expression(b_id), ", ",
			                   to_unpacked_row_major_matrix_expression(a_id), ")");
			bool forward = should_forward(a_id) && should_forward(b_id);
			emit_transposed_op(result_type, result_id, expr, forward);

			ea->need_transpose = true;
			eb->need_transpose = true;
		}
		else
		{
			auto key_a = make_matrix_key(a_type);
			auto key_b = make_matrix_key(b_type);
			need_mul_mat_mat.insert({ key_a, key_b });
			// Also need the MatVec helper for the inner multiplication.
			need_mul_mat_vec.insert(key_a);

			auto mat_a_short = opencl_matrix_short_name(a_type.basetype, a_type.vecsize, a_type.columns);
			auto mat_b_short = opencl_matrix_short_name(b_type.basetype, b_type.vecsize, b_type.columns);
			string func_name = "spvMul" + mat_a_short + mat_b_short;

			emit_binary_func_op(result_type, result_id, a_id, b_id, func_name.c_str());
		}
		inherit_expression_dependencies(result_id, a_id);
		inherit_expression_dependencies(result_id, b_id);
		break;
	}

	case OpMatrixTimesScalar:
	{
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t mat_id = ops[2];
		uint32_t scalar_id = ops[3];
		auto &mat_type = expression_type(mat_id);

		auto *e = maybe_get<SPIRExpression>(mat_id);
		if (e && e->need_transpose)
		{
			// Physical (untransposed) matrix has swapped dimensions.
			e->need_transpose = false;
			MatrixTypeKey phys_key = { mat_type.basetype, mat_type.columns, mat_type.vecsize };
			need_mul_mat_scalar.insert(phys_key);
			used_matrix_types.insert(phys_key);

			auto mat_short = opencl_matrix_short_name(phys_key.basetype, phys_key.vecsize, phys_key.columns);
			string func_name = "spvMul" + mat_short + "Scalar";

			string expr = join(func_name, "(", to_unpacked_row_major_matrix_expression(mat_id), ", ",
			                   to_expression(scalar_id), ")");
			bool forward = should_forward(mat_id) && should_forward(scalar_id);
			emit_transposed_op(result_type, result_id, expr, forward);
			e->need_transpose = true;
		}
		else
		{
			auto key = make_matrix_key(mat_type);
			need_mul_mat_scalar.insert(key);

			auto mat_short = opencl_matrix_short_name(mat_type.basetype, mat_type.vecsize, mat_type.columns);
			string func_name = "spvMul" + mat_short + "Scalar";

			emit_binary_func_op(result_type, result_id, mat_id, scalar_id, func_name.c_str());
		}
		inherit_expression_dependencies(result_id, mat_id);
		inherit_expression_dependencies(result_id, scalar_id);
		break;
	}

	case OpTranspose:
	{
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t input_id = ops[2];
		auto &in_type = expression_type(input_id);
		auto &res_type = get<SPIRType>(result_type);

		auto key = make_matrix_key(in_type);
		need_transpose.insert(key);
		// Ensure both input and output matrix types are registered.
		used_matrix_types.insert(key);
		used_matrix_types.insert(make_matrix_key(res_type));

		auto in_short = opencl_matrix_short_name(in_type.basetype, in_type.vecsize, in_type.columns);
		string func_name = "spvTranspose" + in_short;

		emit_unary_func_op(result_type, result_id, input_id, func_name.c_str());
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
		uint32_t execution_scope = evaluate_constant_u32(ops[0]);
		uint32_t semantics = evaluate_constant_u32(ops[2]);
		semantics = mask_relevant_memory_semantics(semantics);

		flush_control_dependent_expressions(current_emitting_block->self);
		flush_all_active_variables();

		if (execution_scope == ScopeSubgroup)
		{
			if (!opencl_options.enable_subgroups)
				SPIRV_CROSS_THROW("Subgroup barriers require enable_subgroups option.");

			// Subgroup barrier with memory fence flags
			const uint32_t all_barriers =
			    MemorySemanticsWorkgroupMemoryMask | MemorySemanticsUniformMemoryMask | MemorySemanticsImageMemoryMask;

			if (semantics == 0 || (semantics & all_barriers) == all_barriers)
			{
				statement("sub_group_barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);");
			}
			else
			{
				string fence_flags = opencl_mem_fence_flags(semantics);
				statement("sub_group_barrier(", fence_flags, ");");
			}
		}
		else
		{
			// Workgroup barrier
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
				if (opencl_options.supports_opencl_version(2, 0))
					statement("work_group_barrier(CLK_LOCAL_MEM_FENCE);");
				else
					statement("barrier(CLK_LOCAL_MEM_FENCE);");
			}
		}
		break;
	}

	case OpMemoryBarrier:
	{
		// ops[0]=memory_scope, ops[1]=semantics
		uint32_t memory_scope = evaluate_constant_u32(ops[0]);
		uint32_t semantics = evaluate_constant_u32(ops[1]);
		semantics = mask_relevant_memory_semantics(semantics);

		flush_control_dependent_expressions(current_emitting_block->self);
		flush_all_active_variables();

		if (semantics != 0)
		{
			if (memory_scope == ScopeSubgroup)
			{
				if (!opencl_options.enable_subgroups)
					SPIRV_CROSS_THROW("Subgroup memory barriers require enable_subgroups option.");

				const uint32_t all_barriers = MemorySemanticsWorkgroupMemoryMask | MemorySemanticsUniformMemoryMask |
				                              MemorySemanticsImageMemoryMask;

				if ((semantics & all_barriers) == all_barriers ||
				    (semantics & (MemorySemanticsCrossWorkgroupMemoryMask | MemorySemanticsSubgroupMemoryMask)))
				{
					statement("sub_group_barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);");
				}
				else
				{
					string fence_flags = opencl_mem_fence_flags(semantics);
					statement("sub_group_barrier(", fence_flags, ");");
				}
			}
			else
			{
				string fence_flags = opencl_mem_fence_flags(semantics);
				statement("mem_fence(", fence_flags, ");");
			}
		}
		break;
	}

	case OpAtomicExchange:
		if (check_atomic_image(ops[2]))
			SPIRV_CROSS_THROW("Image atomics are not supported in OpenCL.");
		emit_atomic_func_op(ops[0], ops[1], ops[2], ops[5], "atomic_xchg");
		break;
	case OpAtomicCompareExchange:
		if (check_atomic_image(ops[2]))
			SPIRV_CROSS_THROW("Image atomics are not supported in OpenCL.");
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
			SPIRV_CROSS_THROW("Image atomics are not supported in OpenCL.");
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
			SPIRV_CROSS_THROW("Image atomics are not supported in OpenCL.");
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
			SPIRV_CROSS_THROW("Image atomics are not supported in OpenCL.");
		statement("atomic_xchg(", to_atomic_ptr_expression(ops[0]), ", ", to_expression(ops[3]), ");");
		flush_all_atomic_capable_variables();
		break;
	}
	case OpAtomicIIncrement:
	case OpAtomicIDecrement:
	{
		if (check_atomic_image(ops[2]))
			SPIRV_CROSS_THROW("Image atomics are not supported in OpenCL.");
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
	case OpBitCount:
	{
		// GLSL bitCount → OpenCL popcount.
		// popcount returns the same type as its input in OpenCL (unlike GLSL which returns int).
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t arg = ops[2];
		auto &in_type = expression_type(arg);
		auto &out_type = get<SPIRType>(result_type);

		string expr = join("popcount(", to_expression(arg), ")");

		// Cast result if types differ (e.g. popcount(int4) → uint4 needs as_uint4).
		if (out_type.basetype != in_type.basetype)
		{
			expr = join(bitcast_glsl_op(out_type, in_type), "(", expr, ")");
		}

		emit_op(result_type, result_id, expr, should_forward(arg));
		inherit_expression_dependencies(result_id, arg);
		break;
	}

	case OpBitReverse:
	{
		// GLSL bitfieldReverse → no OpenCL builtin.
		// Use scalar polyfill, call per-component for vectors.
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t arg = ops[2];
		auto &type = get<SPIRType>(result_type);

		if (!needs_bitreverse_polyfill)
		{
			needs_bitreverse_polyfill = true;
			force_recompile();
		}

		auto unsigned_basetype = to_unsigned_basetype(type.width);
		string input_expr = bitcast_expression(unsigned_basetype, arg);

		string expr;
		if (type.vecsize > 1)
		{
			// Call scalar polyfill per component.
			SPIRType uint_type = type;
			uint_type.basetype = unsigned_basetype;
			expr = join("(", type_to_glsl(uint_type), ")(");
			for (uint32_t i = 0; i < type.vecsize; i++)
			{
				if (i > 0)
					expr += ", ";
				expr += join("spvBitReverse(", input_expr, ".s", i, ")");
			}
			expr += ")";
		}
		else
			expr = join("spvBitReverse(", input_expr, ")");

		// Cast back to signed if needed.
		if (type.basetype != unsigned_basetype)
		{
			SPIRType uint_type = type;
			uint_type.basetype = unsigned_basetype;
			expr = join(bitcast_glsl_op(type, uint_type), "(", expr, ")");
		}

		emit_op(result_type, result_id, expr, should_forward(arg));
		inherit_expression_dependencies(result_id, arg);
		break;
	}

	case OpBitFieldSExtract:
	case OpBitFieldUExtract:
	{
		// GLSL bitfieldExtract(value, offset, bits) → OpenCL: manual extraction.
		// Unsigned: (value >> offset) & ((1u << bits) - 1u)
		// Signed:   (int)((value >> offset) << (W - bits)) >> (W - bits)  [arithmetic shift for sign-extend]
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t value = ops[2];
		uint32_t offset_id = ops[3];
		uint32_t bits_id = ops[4];
		auto &type = get<SPIRType>(result_type);
		uint32_t width = type.width;

		bool is_signed_extract = (opcode == OpBitFieldSExtract);

		if (is_signed_extract)
		{
			auto signed_basetype = to_signed_basetype(width);
			string val_expr = bitcast_expression(signed_basetype, value);
			// Sign-extending extract: shift left to put field at MSB, then arithmetic shift right.
			// result = (val << (W - bits - offset)) >> (W - bits)
			// Simplified: extract bits, then sign-extend.
			string expr = join("(", val_expr, " << (", width, " - ", to_expression(bits_id), " - ",
			                   to_expression(offset_id), ")) >> (", width, " - ", to_expression(bits_id), ")");

			if (type.basetype != signed_basetype)
			{
				SPIRType signed_type = type;
				signed_type.basetype = signed_basetype;
				expr = join(bitcast_glsl_op(type, signed_type), "(", expr, ")");
			}

			emit_op(result_type, result_id, expr, should_forward(value));
		}
		else
		{
			auto unsigned_basetype = to_unsigned_basetype(width);
			string val_expr = bitcast_expression(unsigned_basetype, value);
			SPIRType uint_type = type;
			uint_type.basetype = unsigned_basetype;
			auto utype = type_to_glsl(uint_type);
			string expr = join("(", val_expr, " >> ", to_expression(offset_id), ") & ((", utype, ")(1u << ",
			                   to_expression(bits_id), ") - (", utype, ")1u)");

			if (type.basetype != unsigned_basetype)
				expr = join(bitcast_glsl_op(type, uint_type), "(", expr, ")");

			emit_op(result_type, result_id, expr, should_forward(value));
		}
		inherit_expression_dependencies(result_id, value);
		inherit_expression_dependencies(result_id, offset_id);
		inherit_expression_dependencies(result_id, bits_id);
		break;
	}

	case OpBitFieldInsert:
	{
		// GLSL bitfieldInsert(base, insert, offset, bits) → OpenCL: manual insertion.
		// mask = ((1u << bits) - 1u) << offset
		// result = (base & ~mask) | ((insert << offset) & mask)
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t base_id = ops[2];
		uint32_t insert_id = ops[3];
		uint32_t offset_id = ops[4];
		uint32_t bits_id = ops[5];
		auto &type = get<SPIRType>(result_type);

		auto unsigned_basetype = to_unsigned_basetype(type.width);
		string base_expr = bitcast_expression(unsigned_basetype, base_id);
		string insert_expr = bitcast_expression(unsigned_basetype, insert_id);

		SPIRType uint_type = type;
		uint_type.basetype = unsigned_basetype;
		auto utype = type_to_glsl(uint_type);

		string mask =
		    join("((", utype, ")(1u << ", to_expression(bits_id), ") - (", utype, ")1u) << ", to_expression(offset_id));
		string expr = join("(", base_expr, " & ~(", mask, ")) | ((", insert_expr, " << ", to_expression(offset_id),
		                   ") & (", mask, "))");

		if (type.basetype != unsigned_basetype)
			expr = join(bitcast_glsl_op(type, uint_type), "(", expr, ")");

		emit_op(result_type, result_id, expr, should_forward(base_id) && should_forward(insert_id));
		inherit_expression_dependencies(result_id, base_id);
		inherit_expression_dependencies(result_id, insert_id);
		inherit_expression_dependencies(result_id, offset_id);
		inherit_expression_dependencies(result_id, bits_id);
		break;
	}

	// BDA pointer casts: emit C-style casts instead of GLSL constructor-style.
	case OpConvertUToPtr:
	{
		auto &type = get<SPIRType>(ops[0]);
		auto &in_type = expression_type(ops[2]);
		auto ptr_type_str = type_to_glsl(type);
		string expr;
		if (in_type.vecsize > 1)
			expr = join("((", ptr_type_str, ")as_ulong(", to_expression(ops[2]), "))");
		else
			expr = join("((", ptr_type_str, ")(", to_expression(ops[2]), "))");
		emit_op(ops[0], ops[1], std::move(expr), should_forward(ops[2]));
		inherit_expression_dependencies(ops[1], ops[2]);
		break;
	}

	case OpConvertPtrToU:
	{
		auto &type = get<SPIRType>(ops[0]);
		string expr;
		if (type.vecsize > 1)
			expr = join("as_", type_to_glsl(type), "((ulong)(", to_expression(ops[2]), "))");
		else
			expr = join("(", type_to_glsl(type), ")(", to_expression(ops[2]), ")");
		emit_op(ops[0], ops[1], std::move(expr), should_forward(ops[2]));
		inherit_expression_dependencies(ops[1], ops[2]);
		break;
	}

	case OpBitcast:
	{
		auto &out_type = get<SPIRType>(ops[0]);
		auto &in_type = expression_type(ops[2]);

		// Bitcast involving pointer types needs special handling in OpenCL C.
		if (is_pointer(out_type) || is_pointer(in_type))
		{
			uint32_t result_type = ops[0];
			uint32_t result_id = ops[1];
			uint32_t arg = ops[2];

			string expr;
			if (is_pointer(out_type) && !is_pointer(in_type))
			{
				// Non-pointer → pointer: cast via ulong if input is a vector (e.g. uvec2).
				auto ptr_type_str = type_to_glsl(out_type);
				if (in_type.vecsize > 1)
					expr = join("((", ptr_type_str, ")as_ulong(", to_expression(arg), "))");
				else
					expr = join("((", ptr_type_str, ")(", to_expression(arg), "))");
			}
			else if (!is_pointer(out_type) && is_pointer(in_type))
			{
				// Pointer → non-pointer: cast to ulong, then to target type.
				if (out_type.vecsize > 1)
					expr = join("as_", type_to_glsl(out_type), "((ulong)(", to_expression(arg), "))");
				else
					expr = join("(", type_to_glsl(out_type), ")((ulong)(", to_expression(arg), "))");
			}
			else
			{
				// Pointer → pointer: direct C-style cast.
				expr = join("((", type_to_glsl(out_type), ")(", to_expression(arg), "))");
			}

			emit_op(result_type, result_id, std::move(expr), should_forward(arg));
			inherit_expression_dependencies(result_id, arg);
			break;
		}

		CompilerGLSL::emit_instruction(instruction);
		break;
	}

	case OpPtrAccessChain:
	{
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t base_id = ops[2];

		auto &base_type = expression_type(base_id);
		TypeID base_type_id = expression_type_id(base_id);

		// Check if custom stride pointer arithmetic is needed.
		if (has_decoration(base_type_id, DecorationArrayStride))
		{
			TypeID pointee_type_id = get_pointee_type_id(base_type_id);
			uint32_t physical_stride = get_physical_type_id_stride(pointee_type_id);
			uint32_t requested_stride = get_decoration(base_type_id, DecorationArrayStride);

			if (physical_stride != requested_stride)
			{
				// Custom stride: use pointer arithmetic via ulong cast.
				// *((__global T*)((ulong)ptr + index * stride))
				uint32_t index_id = ops[3];
				auto &pointee_type = get<SPIRType>(pointee_type_id);
				auto &ptr_type = get<SPIRType>(base_type_id);
				auto addr_space = get_type_address_space(ptr_type, 0);

				string base_expr = to_enclosed_expression(base_id);
				string intptr_expr =
				    join("(ulong)(", base_expr, ") + ", to_enclosed_expression(index_id), " * ", requested_stride);
				string ptr_cast = join("(", addr_space, " ", type_to_glsl(pointee_type), "*)(", intptr_expr, ")");
				string expr = join("*(", ptr_cast, ")");

				auto &e = set<SPIRExpression>(result_id, std::move(expr), result_type, should_forward(base_id));
				auto *backing_var = maybe_get_backing_variable(base_id);
				e.loaded_from = backing_var ? backing_var->self : ID(base_id);
				e.access_chain = true;
				forwarded_temporaries.insert(result_id);
				suppressed_usage_tracking.insert(result_id);
				inherit_expression_dependencies(result_id, base_id);
				inherit_expression_dependencies(result_id, index_id);

				// Mark as packed if the vector stride differs from natural alignment.
				if (is_vector(pointee_type) && requested_stride != physical_stride)
					set_extended_decoration(result_id, SPIRVCrossDecorationPhysicalTypePacked);

				break;
			}
		}

		// No custom stride — fall through to base class.
		CompilerGLSL::emit_instruction(instruction);
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
			int32_t row_major_mbr_idx = -1; // member index for row-major check, -1 if N/A

			bool is_subscript_deref = false; // result is a C value (subscripted), not a pointer

			if (length >= 5 && is_single_member)
			{
				// Single-member SSBO flattened to __global T*: ptr[element_idx][.member]*
				// ops[3] = struct member index (always 0, skip)
				// ops[4] = element index within the runtime array
				// ops[5+] = optional sub-member indices
				expr = join(to_name(base_id), "[", to_expression(ops[4]), "]");
				is_subscript_deref = true;
				row_major_mbr_idx = 0; // single member, always index 0
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
			else if (length == 5 && !is_single_member && struct_type && !struct_type->array.empty())
			{
				// Array of multi-member SSBOs: ptr[array_idx].member_name
				// ops[3] = array index (dynamic), ops[4] = member index (constant)
				uint32_t mbr_idx = get<SPIRConstant>(ops[4]).scalar();
				auto mbr_name = to_member_name(*struct_type, mbr_idx);
				expr = join(to_name(base_id), "[", to_expression(ops[3]), "].", mbr_name);
				is_subscript_deref = true;
				row_major_mbr_idx = int32_t(mbr_idx);
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
				row_major_mbr_idx = int32_t(mbr_idx);
				handled = true;
			}
			else if (length == 4 && is_single_member)
			{
				// Single-member SSBO flattened to T*: accessing the one member gives element 0.
				expr = join(to_name(base_id), "[0]");
				is_subscript_deref = true;
				row_major_mbr_idx = 0; // single member, always index 0
				handled = true;
			}
			else if (length == 4 && !is_single_member && struct_type && !struct_type->array.empty())
			{
				// Array of multi-member SSBOs: ptr[array_idx] (result is struct)
				expr = join(to_name(base_id), "[", to_expression(ops[3]), "]");
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
				row_major_mbr_idx = int32_t(mbr_idx);
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

				// Propagate row-major transpose flag for matrix members.
				if (struct_type && row_major_mbr_idx >= 0)
				{
					if (member_is_non_native_row_major_matrix(*struct_type, uint32_t(row_major_mbr_idx)))
						e.need_transpose = true;
				}

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

	case OpCompositeConstruct:
	{
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		auto &type = get<SPIRType>(result_type);
		if (type.columns > 1)
		{
			// Matrix composite construct: emit compound literal (spvMat4){ { col0, col1, ... } }
			const auto *elems = &ops[2];
			uint32_t length = instruction.length - 2;

			bool forward = true;
			for (uint32_t i = 0; i < length; i++)
				forward = forward && should_forward(elems[i]);

			auto mat_name = opencl_matrix_type_name(type);
			string expr = "(" + mat_name + "){ { ";
			for (uint32_t i = 0; i < length; i++)
			{
				if (i > 0)
					expr += ", ";
				expr += to_unpacked_expression(elems[i]);
			}
			expr += " } }";
			emit_op(result_type, result_id, expr, forward);
			for (uint32_t i = 0; i < length; i++)
				inherit_expression_dependencies(result_id, elems[i]);
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
			// Struct-wrapped matrix: replicate the column value across all columns.
			auto mat_name = opencl_matrix_type_name(type);
			string expr = "(" + mat_name + "){ { ";
			for (uint32_t i = 0; i < type.columns; i++)
			{
				if (i > 0)
					expr += ", ";
				expr += to_expression(ops[2]);
			}
			expr += " } }";
			emit_op(result_type, result_id, expr, should_forward(ops[2]));
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

	// OpImageFetch (texelFetch in GLSL) maps to read_image* in OpenCL C,
	// same as OpImageRead but may carry a Lod operand (which we ignore
	// since OpenCL images don't support LOD on read).
	case OpImageFetch:
	{
		uint32_t result_type = ops[0];
		uint32_t result_id = ops[1];
		uint32_t image_id = ops[2];
		uint32_t coord_id = ops[3];

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

		auto raw_expr = join(read_func, "(", to_expression(image_id), ", ", coord_expr, ")");
		auto swizzled = remap_swizzle(result_spirtype, 4, raw_expr);

		bool forward = should_forward(image_id) && should_forward(coord_id);
		emit_op(result_type, result_id, swizzled, forward);
		inherit_expression_dependencies(result_id, image_id);
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

		// Unset NonReadable so image access qualifier deduction works correctly.
		auto *image_var = maybe_get_backing_variable(image_id);
		if (image_var)
		{
			auto &flags = get_decoration_bitset(image_var->self);
			if (flags.get(DecorationNonReadable))
			{
				unset_decoration(image_var->self, DecorationNonReadable);
				force_recompile();
			}
		}

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

	case OpPtrEqual:
		emit_binary_ptr_op(ops[0], ops[1], ops[2], ops[3], "==");
		break;

	case OpPtrNotEqual:
		emit_binary_ptr_op(ops[0], ops[1], ops[2], ops[3], "!=");
		break;

	case OpPtrDiff:
		emit_binary_ptr_op(ops[0], ops[1], ops[2], ops[3], "-");
		break;

	case OpSDot:
	case OpUDot:
	case OpSUDot:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t vec1 = ops[2];
		uint32_t vec2 = ops[3];

		auto &input_type1 = expression_type(vec1);
		auto &input_type2 = expression_type(vec2);
		auto &type = get<SPIRType>(result_type);

		string vec1input, vec2input;
		uint32_t input_size = input_type1.vecsize;

		if (instruction.length == 5)
		{
			if (ops[4] == PackedVectorFormatPackedVectorFormat4x8Bit)
			{
				string type1 = opcode == OpSDot || opcode == OpSUDot ? "char4" : "uchar4";
				vec1input = join("as_", type1, "(", to_expression(vec1), ")");
				string type2 = opcode == OpSDot ? "char4" : "uchar4";
				vec2input = join("as_", type2, "(", to_expression(vec2), ")");
				input_size = 4;
			}
			else
				SPIRV_CROSS_THROW("Packed vector formats other than 4x8Bit for integer dot product is not supported.");
		}
		else
		{
			SPIRType::BaseType vec1_expected_type =
			    opcode != OpUDot ? to_signed_basetype(input_type1.width) : to_unsigned_basetype(input_type1.width);
			SPIRType::BaseType vec2_expected_type =
			    opcode != OpSDot ? to_unsigned_basetype(input_type2.width) : to_signed_basetype(input_type2.width);

			vec1input = bitcast_expression(vec1_expected_type, vec1);
			vec2input = bitcast_expression(vec2_expected_type, vec2);
		}

		// Emit inline sum of component-wise products:
		// (result_type)(a.s0) * (result_type)(b.s0) + ... + (result_type)(a.sN) * (result_type)(b.sN)
		auto result_type_str = type_to_glsl(type);
		string exp;
		for (uint32_t i = 0; i < input_size; i++)
		{
			if (i > 0)
				exp += " + ";
			string comp = input_size > 1 ? join(".s", i) : "";
			exp +=
			    join("(", result_type_str, ")(", vec1input, comp, ") * (", result_type_str, ")(", vec2input, comp, ")");
		}

		emit_op(result_type, id, exp, should_forward(vec1) && should_forward(vec2));
		inherit_expression_dependencies(id, vec1);
		inherit_expression_dependencies(id, vec2);
		break;
	}

	case OpSDotAccSat:
	case OpUDotAccSat:
	case OpSUDotAccSat:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t vec1 = ops[2];
		uint32_t vec2 = ops[3];
		uint32_t acc = ops[4];

		auto input_type1 = expression_type(vec1);
		auto input_type2 = expression_type(vec2);
		auto &type = get<SPIRType>(result_type);

		string vec1input, vec2input;
		uint32_t input_size = input_type1.vecsize;

		if (instruction.length == 6)
		{
			if (ops[5] == PackedVectorFormatPackedVectorFormat4x8Bit)
			{
				string type1 = opcode == OpSDotAccSat || opcode == OpSUDotAccSat ? "char4" : "uchar4";
				vec1input = join("as_", type1, "(", to_expression(vec1), ")");
				string type2 = opcode == OpSDotAccSat ? "char4" : "uchar4";
				vec2input = join("as_", type2, "(", to_expression(vec2), ")");
				input_size = 4;
			}
			else
				SPIRV_CROSS_THROW("Packed vector formats other than 4x8Bit for integer dot product is not supported.");
		}
		else
		{
			SPIRType::BaseType vec1_expected_type = opcode != OpUDotAccSat ? to_signed_basetype(input_type1.width) :
			                                                                 to_unsigned_basetype(input_type1.width);
			SPIRType::BaseType vec2_expected_type = opcode != OpSDotAccSat ? to_unsigned_basetype(input_type2.width) :
			                                                                 to_signed_basetype(input_type2.width);

			vec1input = bitcast_expression(vec1_expected_type, vec1);
			vec2input = bitcast_expression(vec2_expected_type, vec2);
		}

		SPIRType::BaseType pre_saturate_type =
		    opcode != OpUDotAccSat ? to_signed_basetype(type.width) : to_unsigned_basetype(type.width);

		// Use the pre-saturate type for internal computation so add_sat arguments match.
		SPIRType sat_type = type;
		sat_type.basetype = pre_saturate_type;
		auto sat_type_str = type_to_glsl(sat_type);
		auto result_type_str = type_to_glsl(type);

		// Build dot product expression: sum of component-wise products
		string dot_exp;
		for (uint32_t i = 0; i < input_size; i++)
		{
			if (i > 0)
				dot_exp += " + ";
			string comp = input_size > 1 ? join(".s", i) : "";
			dot_exp +=
			    join("(", sat_type_str, ")(", vec1input, comp, ") * (", sat_type_str, ")(", vec2input, comp, ")");
		}

		// Wrap with add_sat and cast to result type
		string exp =
		    join("(", result_type_str, ")add_sat(", dot_exp, ", ", bitcast_expression(pre_saturate_type, acc), ")");

		emit_op(result_type, id, exp, should_forward(vec1) && should_forward(vec2));
		inherit_expression_dependencies(id, vec1);
		inherit_expression_dependencies(id, vec2);
		break;
	}

	default:
		CompilerGLSL::emit_instruction(instruction);
		break;
	}
}
