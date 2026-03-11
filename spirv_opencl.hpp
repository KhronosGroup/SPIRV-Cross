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
		// Enable cl_khr_fp64 (double) extension
		bool enable_fp64 = false;
		// Enable cl_khr_int64_extended_atomics extension
		bool enable_64bit_atomics = false;

		void set_opencl_version(uint32_t major, uint32_t minor = 0, uint32_t patch = 0)
		{
			opencl_version = make_opencl_version(major, minor, patch);
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
	virtual bool builtin_translates_to_nonarray(BuiltIn builtin) const override;
	std::string get_variable_address_space(const SPIRVariable &argument);
	std::string get_type_address_space(const SPIRType &type, uint32_t id, bool argument = false);
	const char *to_restrict(uint32_t id, bool space);

	void replace_illegal_names() override;

	Options opencl_options;

	// SSBO variables emitted as flat element pointers (__global T*) in the kernel signature
	std::unordered_set<uint32_t> flattened_buffer_vars;
	// Push-constant variable → { member_index → scalar param name }
	std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::string>> push_const_member_map;

	std::unordered_set<uint32_t> constant_macro_ids;

	void emit_workgroup_size_attribute();

	std::string entry_point_args(bool append_comma);
	std::string get_inner_entry_point_name() const;
};

} // namespace SPIRV_CROSS_NAMESPACE

#endif
