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

#ifndef SPIRV_CROSS_OPENCL_HPP
#define SPIRV_CROSS_OPENCL_HPP

#include "spirv_glsl.hpp"
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace SPIRV_CROSS_NAMESPACE
{
using namespace SPIRV_CROSS_SPV_HEADER_NAMESPACE;

// Decompiles SPIR-V (compute only) to OpenCL C
class CompilerOpenCL : public CompilerGLSL
{
public:
	struct Options
	{
		// OpenCL C version: 120 = 1.2, 200 = 2.0
		uint32_t opencl_version = make_opencl_version(1, 2);
		// Enable cl_khr_fp16 (half) extension
		bool enable_fp16 = false;
		// Enable cl_khr_fp64 (double) extension
		bool enable_fp64 = false;
		// Enable cl_khr_int64_extended_atomics extension
		bool enable_64bit_atomics = false;
		// Enable cl_khr_subgroups extension
		bool enable_subgroups = false;
		// Enable all subgroup extensions
		bool enable_subgroups_all = false;
		// Emulate missing subgroup extensions
		bool emulate_subgroups = false;
		// Size of subgroup emulation
		uint32_t fixed_subgroup_size = 0;

		void set_opencl_version(uint32_t major, uint32_t minor = 0, uint32_t patch = 0)
		{
			opencl_version = make_opencl_version(major, minor, patch);
			if (opencl_version >= 200 && opencl_version < 300)
				enable_subgroups = true;
		}

		bool supports_opencl_version(uint32_t major, uint32_t minor = 0, uint32_t patch = 0) const
		{
			return opencl_version >= make_opencl_version(major, minor, patch);
		}

		static uint32_t make_opencl_version(uint32_t major, uint32_t minor = 0, uint32_t patch = 0)
		{
			return (major * 100) + (minor * 10) + patch;
		}
	};

	explicit CompilerOpenCL(std::vector<uint32_t> spirv_);
	CompilerOpenCL(const uint32_t *ir_, size_t word_count);
	explicit CompilerOpenCL(const ParsedIR &ir_);
	explicit CompilerOpenCL(ParsedIR &&ir_);

	const Options &get_opencl_options() const
	{
		return opencl_options;
	}
	void set_opencl_options(const Options &opts)
	{
		opencl_options = opts;
	}

	std::string compile() override;

	// Information about specialization constants that are translated into macros
	// instead of using constant declarations.
	// These must only be called after a successful call to CompilerOpenCL::compile().
	bool specialization_constant_is_macro(uint32_t constant_id) const;

protected:
	void emit_header() override;
	void emit_resources();
	void emit_specialization_constants_and_structs();
	std::string type_to_glsl(const SPIRType &type, uint32_t id, bool member);
	std::string type_to_glsl(const SPIRType &type, uint32_t id = 0) override;
	std::string builtin_to_glsl(BuiltIn builtin, StorageClass storage) override;
	std::string image_type_glsl(const SPIRType &type, uint32_t id = 0, bool member = false) override;
	const char *to_storage_qualifiers_glsl(const SPIRVariable &var) override;
	void emit_entry_point_declarations() override;
	// GCC workaround of lambdas calling protected functions (for older GCC versions)
	std::string variable_decl(const SPIRType &type, const std::string &name, uint32_t id = 0) override;
	void emit_function_prototype(SPIRFunction &func, const Bitset &return_flags) override;
	void emit_instruction(const Instruction &instruction) override;
	bool should_dereference(uint32_t id) override;
	std::string to_member_reference(uint32_t base, const SPIRType &type, uint32_t index,
	                                bool ptr_chain_is_resolved) override;
	std::string to_func_call_arg(const SPIRFunction::Parameter &arg, uint32_t id) override;
	void add_function_overload(const SPIRFunction &func) override;
	void emit_struct(SPIRType &type) override;
	std::string type_to_glsl_constructor(const SPIRType &type) override;
	bool emit_array_copy(const char *expr, uint32_t lhs_id, uint32_t rhs_id, StorageClass lhs_storage,
	                     StorageClass rhs_storage) override;
	std::string constant_expression(const SPIRConstant &c, bool inside_block_like_struct_scope = false,
	                                bool inside_struct_scope = false) override;
	std::string to_initializer_expression(const SPIRVariable &var) override;
	std::string constant_expression_vector(const SPIRConstant &c, uint32_t vector) override;
	std::string bitcast_glsl_op(const SPIRType &result_type, const SPIRType &argument_type) override;
	bool emit_complex_bitcast(uint32_t result_type, uint32_t id, uint32_t op0) override;
	std::string to_atomic_ptr_expression(uint32_t id) override;
	void emit_glsl_op(uint32_t result_type, uint32_t result_id, uint32_t op, const uint32_t *args,
	                  uint32_t count) override;
	virtual bool builtin_translates_to_nonarray(BuiltIn builtin) const override;
	std::string get_variable_address_space(const SPIRVariable &argument);
	std::string get_type_address_space(const SPIRType &type, uint32_t id, bool argument = false);
	const char *to_restrict(uint32_t id, bool space);
	uint32_t get_physical_type_id_stride(TypeID type_id) const override;
	bool member_is_non_native_row_major_matrix(const SPIRType &type, uint32_t index) override;
	std::string convert_row_major_matrix(std::string exp_str, const SPIRType &exp_type, uint32_t physical_type_id,
	                                     bool is_packed, bool relaxed) override;

	void replace_illegal_names() override;
	void emit_function(SPIRFunction &func, const Bitset &return_flags) override;
	void emit_block_hints(const SPIRBlock &block) override;
	void emit_store_statement(uint32_t lhs_expression, uint32_t rhs_expression) override;
	void emit_struct_member(const SPIRType &type, uint32_t member_type_id, uint32_t index,
	                        const std::string &qualifier = "", uint32_t base_offset = 0) override;
	void emit_binary_ptr_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char *op);
	std::string to_ptr_expression(uint32_t id, bool register_expression_read = true);
	bool prepare_access_chain_for_scalar_access(std::string &expr, const SPIRType &type, StorageClass storage,
	                                            bool &is_packed) override;

	Options opencl_options;

	// SSBO variables emitted as flat element pointers (__global T*) in the kernel signature
	std::unordered_set<uint32_t> flattened_buffer_vars;
	// Push-constant variable → { member_index → scalar param name }
	std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::string>> push_const_member_map;

	std::unordered_set<uint32_t> constant_macro_ids;

	// Expression IDs that were produced by subscripting a flattened SSBO pointer (e.g. ptr[idx]).
	// These are C values (not pointers), so subsequent member accesses must use '.' not '->'.
	std::unordered_set<uint32_t> subscripted_deref_exprs;

	// Set when packHalf2x16/unpackHalf2x16 polyfill helpers are needed.
	bool needs_half_pack_polyfill = false;
	bool needs_half_unpack_polyfill = false;
	// Set when bitfieldReverse polyfill is needed.
	bool needs_bitreverse_polyfill = false;
	// Set when a default sampler is needed for combined image+sampler usage.
	bool needs_default_sampler = false;
	// Set when findLSB polyfill is needed.
	bool needs_findlsb_polyfill = false;
	// Set when pack/unpack Snorm/Unorm polyfills are needed.
	bool needs_pack_snorm_4x8 = false;
	bool needs_pack_unorm_4x8 = false;
	bool needs_pack_snorm_2x16 = false;
	bool needs_pack_unorm_2x16 = false;
	bool needs_unpack_snorm_4x8 = false;
	bool needs_unpack_unorm_4x8 = false;
	bool needs_unpack_snorm_2x16 = false;
	bool needs_unpack_unorm_2x16 = false;
	// Set when determinant/inverse polyfills are needed (per size).
	bool needs_determinant_2 = false;
	bool needs_determinant_3 = false;
	bool needs_determinant_4 = false;
	bool needs_inverse_2 = false;
	bool needs_inverse_3 = false;
	bool needs_inverse_4 = false;

	// Matrix type support: tracks which matrix signatures (basetype, vecsize, columns) are needed.
	struct MatrixTypeKey
	{
		SPIRType::BaseType basetype;
		uint32_t vecsize;
		uint32_t columns;
		bool operator<(const MatrixTypeKey &o) const
		{
			if (basetype != o.basetype)
				return basetype < o.basetype;
			if (columns != o.columns)
				return columns < o.columns;
			return vecsize < o.vecsize;
		}
		bool operator==(const MatrixTypeKey &o) const
		{
			return basetype == o.basetype && vecsize == o.vecsize && columns == o.columns;
		}
		bool operator!=(const MatrixTypeKey &o) const
		{
			return !(*this == o);
		}
	};
	std::set<MatrixTypeKey> used_matrix_types;

	// Flags for which matrix helper functions need to be emitted.
	std::set<MatrixTypeKey> need_mul_mat_vec; // MatrixTimesVector
	std::set<MatrixTypeKey> need_mul_vec_mat; // VectorTimesMatrix
	std::set<std::pair<MatrixTypeKey, MatrixTypeKey>> need_mul_mat_mat; // MatrixTimesMatrix
	std::set<MatrixTypeKey> need_mul_mat_scalar; // MatrixTimesScalar
	std::set<MatrixTypeKey> need_transpose; // OpTranspose (key is input matrix type)
	std::set<MatrixTypeKey> need_outer_product; // OpOuterProduct (key is result matrix type)

	std::string opencl_matrix_type_name(const SPIRType &type);
	std::string opencl_matrix_type_name(SPIRType::BaseType basetype, uint32_t vecsize, uint32_t columns);
	std::string opencl_column_type_name(SPIRType::BaseType basetype, uint32_t vecsize);
	// Short names for building helper function names (e.g. "Mat4", "Vec4", "DVec4").
	std::string opencl_matrix_short_name(SPIRType::BaseType basetype, uint32_t vecsize, uint32_t columns);
	std::string opencl_vector_short_name(SPIRType::BaseType basetype, uint32_t vecsize);
	void emit_matrix_typedefs();
	void emit_matrix_helpers();
	void emit_mul_mat_vec_helper(SPIRType::BaseType basetype, uint32_t vecsize, uint32_t columns);
	void emit_mul_vec_mat_helper(SPIRType::BaseType basetype, uint32_t vecsize, uint32_t columns);
	void emit_mul_mat_mat_helper(const MatrixTypeKey &a, const MatrixTypeKey &b);
	void emit_mul_mat_scalar_helper(SPIRType::BaseType basetype, uint32_t vecsize, uint32_t columns);
	void emit_transpose_helper(SPIRType::BaseType basetype, uint32_t vecsize, uint32_t columns);
	void emit_outer_product_helper(SPIRType::BaseType basetype, uint32_t vecsize, uint32_t columns);
	MatrixTypeKey make_matrix_key(const SPIRType &type);
	void prepass_discover_matrix_types();

	// For each non-entry function, the ordered list of flattened buffer var IDs to thread as extra params.
	std::unordered_map<uint32_t, SmallVector<uint32_t>> func_flattened_args;
	// Map from flattened buffer var ID to its OpenCL type declaration prefix ("__global T*" etc.)
	std::unordered_map<uint32_t, std::string> flattened_var_type_decl;

	// For each non-entry function, workgroup/private global vars accessed and needing pointer threading.
	std::unordered_map<uint32_t, SmallVector<uint32_t>> func_workgroup_args;
	// Map from workgroup/private var ID to its pointer type declaration prefix
	std::unordered_map<uint32_t, std::string> workgroup_var_ptr_type;
	// Set of scalar (non-array) workgroup/private vars that need #define dereference inside callees
	std::unordered_set<uint32_t> workgroup_scalar_vars;

	// Input builtin variables threaded to non-entry functions (BuiltIn enum → variable ID)
	std::unordered_map<uint32_t, uint32_t> threaded_input_builtins;
	// Input builtin variables materialized as local vars in the entry point (BuiltIn enum → variable ID)
	std::unordered_map<uint32_t, uint32_t> entry_point_materialized_builtins;
	// Guard flag to avoid circular reference during builtin materialization emission
	bool emitting_builtin_materialization = false;

	void compute_kernel_resources();
	void append_global_func_args(const SPIRFunction &func, uint32_t index, SmallVector<std::string> &arglist) override;
	void emit_workgroup_size_attribute();

	std::string entry_point_args(bool append_comma);
	std::string get_inner_entry_point_name() const;
};

} // namespace SPIRV_CROSS_NAMESPACE

#endif
