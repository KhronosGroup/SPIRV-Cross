/*
 * Copyright 2016-2021 The Brenwill Workshop Ltd.
 * SPDX-License-Identifier: Apache-2.0 OR MIT
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

/*
 * At your option, you may choose to accept this material under either:
 *  1. The Apache License, Version 2.0, found at <http://www.apache.org/licenses/LICENSE-2.0>, or
 *  2. The MIT License, found at <http://opensource.org/licenses/MIT>.
 */
#include "spirv_spirv.hpp"

using namespace spv;
using namespace SPIRV_CROSS_NAMESPACE;
using namespace std;

CompilerSPIRV::CompilerSPIRV(std::vector<uint32_t> spirv_)
    : Compiler(std::move(spirv_))
{
}

CompilerSPIRV::CompilerSPIRV(const uint32_t *ir_, size_t word_count)
    : Compiler(ir_, word_count)
{
}

CompilerSPIRV::CompilerSPIRV(const ParsedIR &ir_)
    : Compiler(ir_)
{
}

CompilerSPIRV::CompilerSPIRV(ParsedIR &&ir_)
    : Compiler(std::move(ir_))
{
}

string CompilerSPIRV::compile()
{
	ir.fixup_reserved_names();
	
	this->emit_header();
	this->emit_capabilities();
	this->emit_ext_inst_imports();
	this->emit_exec_info();
}

void CompilerSPIRV::emit_header()
{
	// Push header info to buffer
	buffer.push_back(spv::MagicNumber);
	buffer.push_back(ir.get_spirv_version());
	buffer.push_back(0);
	buffer.push_back(ir.ids.size());
	buffer.push_back(0);
}

void CompilerSPIRV::emit_capabilities()
{
	for (spv::Capability cap : ir.declared_capabilities)
	{
		this->emit_opcode(Op::OpCapability);
		this->emit_id(cap);
	}
}

void CompilerSPIRV::emit_extensions()
{
	
	for (string& ext: ir.declared_extensions)
	{
		this->emit_opcode(Op::OpExtension);
		this->emit_string(ext);
	}
}

void CompilerSPIRV::emit_ext_inst_imports()
{
	ir.for_each_typed_id<SPIRExtension>([this](ID id, SPIRExtension ext) {
		if (ext.ext != SPIRExtension::Unsupported && ext.ext != SPIRExtension::NonSemanticGeneric)
		{
			this->emit_opcode(Op::OpExtInstImport);

			switch (ext.ext)
			{
			default:
				break;

			case SPIRExtension::GLSL:
				this->emit_string("GLSL.std.450");
				break;

			case SPIRExtension::SPV_debug_info:
				this->emit_string("DebugInfo");
				break;

			case SPIRExtension::SPV_AMD_shader_ballot:
				this->emit_string("SPV_AMD_shader_ballot");
				break;

			case SPIRExtension::SPV_AMD_shader_explicit_vertex_parameter:
				this->emit_string("SPV_AMD_shader_explicit_vertex_parameter");
				break;

			case SPIRExtension::SPV_AMD_shader_trinary_minmax:
				this->emit_string("SPV_AMD_shader_trinary_minmax");
				break;

			case SPIRExtension::SPV_AMD_gcn_shader:
				this->emit_string("SPV_AMD_gcn_shader");
				break;

			case SPIRExtension::NonSemanticDebugPrintf:
				this->emit_string("NonSemantic.DebugPrintf");
				break;

			case SPIRExtension::NonSemanticShaderDebugInfo:
				this->emit_string("NonSemantic.Shader.DebugInfo.100");
				break;

			}
		    
		}
	});
}

void CompilerSPIRV::emit_exec_info()
{
	// OpMemoryModel
	this->emit_opcode(Op::OpMemoryModel, 2);
	this->emit_id(ir.addressing_model);
	this->emit_id(ir.memory_model);

	// Emit entrypoints
	for (auto& entrypoint : ir.entry_points)
	{
		auto functionId = entrypoint.first;
		auto functionInfo = entrypoint.second;

		this->emit_opcode(Op::OpEntryPoint, 3 + functionInfo.interface_variables.size());
		this->emit_id(functionInfo.model);
		this->emit_id(functionId);
		this->emit_string(functionInfo.name);
		for (auto &ifaceId : functionInfo.interface_variables)
		{
			this->emit_id(ifaceId);
		}
	}
}

void CompilerSPIRV::emit_opcode(Op opcode, uint32_t arglength = 0)
{
	buffer.push_back(opcode | ((arglength+1) << 16));
}

void CompilerSPIRV::emit_id(uint32_t id)
{
	buffer.push_back(id);
}

void CompilerSPIRV::emit_string(string str)
{
	// TODO implement
}