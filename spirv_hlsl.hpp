/*
* Copyright 2016 Robert Konrad
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
	CompilerHLSL(std::vector<uint32_t> spirv_)
	    : CompilerGLSL(move(spirv_))
	{
	}
	std::string compile() override;

private:
	std::string type_to_glsl(const SPIRType &type) override;
	void emit_function_prototype(SPIRFunction &func, uint64_t return_flags) override;
	void emit_hlsl_entry_point();
	void emit_header() override;
	void emit_resources();
	void emit_interface_block_globally(const SPIRVariable &type);
	void emit_interface_block_in_struct(const SPIRVariable &type, uint32_t &binding_number);
	void emit_texture_op(const Instruction &i) override;
};
}

#endif
