/*
 * Copyright 2016-2017 Robert Konrad
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

#ifndef SPIRV_HLSL_HPP
#define SPIRV_HLSL_HPP

#include "spirv_glsl.hpp"
#include <utility>
#include <vector>

namespace spirv_cross
{
class CompilerHLSL : public CompilerGLSL
{
public:
	struct Options
	{
		uint32_t shader_model = 30; // TODO: map ps_4_0_level_9_0,... somehow
		bool fixup_clipspace = false;
		bool flip_vert_y = false;
	};

	CompilerHLSL(std::vector<uint32_t> spirv_)
	    : CompilerGLSL(move(spirv_))
	{
	}

	const Options &get_options() const
	{
		return options;
	}

	void set_options(Options &opts)
	{
		options = opts;
	}

	std::string compile() override;

private:
	std::string type_to_glsl(const SPIRType &type) override;
	void emit_function_prototype(SPIRFunction &func, uint64_t return_flags) override;
	void emit_hlsl_entry_point();
	void emit_header() override;
	void emit_resources();
	void emit_interface_block_globally(const SPIRVariable &type);
	void emit_interface_block_in_struct(const SPIRVariable &type, uint32_t &binding_number, bool builtins);
	void emit_texture_op(const Instruction &i) override;
	void emit_instruction(const Instruction &instruction) override;
	void emit_glsl_op(uint32_t result_type, uint32_t result_id, uint32_t op, const uint32_t *args,
	                  uint32_t count) override;
	void emit_buffer_block(const SPIRVariable &type) override;
	void emit_push_constant_block(const SPIRVariable &var) override;
	void emit_uniform(const SPIRVariable &var) override;

	Options options;
	bool requires_op_fmod = false;
};
}

#endif
