/*
 * Copyright 2024 Kitsunebi Games
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
    : CompilerGLSL(std::move(spirv_))
{
}

CompilerSPIRV::CompilerSPIRV(const uint32_t *ir_, size_t word_count)
    : CompilerGLSL(ir_, word_count)
{
}

CompilerSPIRV::CompilerSPIRV(const ParsedIR &ir_)
    : CompilerGLSL(ir_)
{
}

CompilerSPIRV::CompilerSPIRV(ParsedIR &&ir_)
    : CompilerGLSL(std::move(ir_))
{
}

string CompilerSPIRV::compile()
{
	buffer.clear();
	ir.fixup_reserved_names();
	
	this->emit_header();
	this->emit_capabilities();
	this->emit_ext_inst_imports();
	this->emit_exec_info();
	this->emit_types();

	return string(reinterpret_cast<const char *>(buffer.data()), buffer.size()*sizeof(uint32_t));
}

void CompilerSPIRV::emit_header()
{
	// Push header info to buffer
	this->buffer.push_back(spv::MagicNumber);
	this->buffer.push_back(ir.get_spirv_version());
	this->buffer.push_back(0);
	this->buffer.push_back(ir.ids.size());
	this->buffer.push_back(0);
}

void CompilerSPIRV::emit_capabilities()
{
	for (spv::Capability cap : ir.declared_capabilities)
	{
		this->emit_opcode(Op::OpCapability, 1);
		this->emit_id(cap);
	}
}

void CompilerSPIRV::emit_extensions()
{
	
	for (string& ext: ir.declared_extensions)
	{
		auto str = this->get_spirv_string(ext);
		this->emit_opcode(Op::OpExtension, str.size());
		this->emit_string(str);
	}
}

void CompilerSPIRV::emit_ext_inst_imports()
{
	ir.for_each_typed_id<SPIRExtension>([this](ID id, SPIRExtension ext) {
		if (ext.ext != SPIRExtension::Unsupported && ext.ext != SPIRExtension::NonSemanticGeneric)
		{

			vector<uint32_t> ext_name;

			switch (ext.ext)
			{
			default:
				break;

			case SPIRExtension::GLSL:
				ext_name = this->get_spirv_string("GLSL.std.450");
				break;

			case SPIRExtension::SPV_debug_info:
				ext_name = this->get_spirv_string("DebugInfo");
				break;

			case SPIRExtension::SPV_AMD_shader_ballot:
				ext_name = this->get_spirv_string("SPV_AMD_shader_ballot");
				break;

			case SPIRExtension::SPV_AMD_shader_explicit_vertex_parameter:
				ext_name = this->get_spirv_string("SPV_AMD_shader_explicit_vertex_parameter");
				break;

			case SPIRExtension::SPV_AMD_shader_trinary_minmax:
				ext_name = this->get_spirv_string("SPV_AMD_shader_trinary_minmax");
				break;

			case SPIRExtension::SPV_AMD_gcn_shader:
				ext_name = this->get_spirv_string("SPV_AMD_gcn_shader");
				break;

			case SPIRExtension::NonSemanticDebugPrintf:
				ext_name = this->get_spirv_string("NonSemantic.DebugPrintf");
				break;

			case SPIRExtension::NonSemanticShaderDebugInfo:
				ext_name = this->get_spirv_string("NonSemantic.Shader.DebugInfo.100");
				break;
			}

			this->emit_opcode(Op::OpExtInstImport, ext_name.size());
			this->emit_string(ext_name);
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
	for (auto &entrypoint : ir.entry_points)
	{
		auto function_id = entrypoint.first;
		auto function_info = entrypoint.second;
		auto spirv_name = this->get_spirv_string(function_info.name);

		this->emit_opcode(Op::OpEntryPoint, 3 + function_info.interface_variables.size() + spirv_name.size());
		this->emit_id(function_info.model);
		this->emit_id(function_id);
		this->emit_string(spirv_name);
		for (auto &iface_id : function_info.interface_variables)
		{
			this->emit_id(iface_id);
		}
	}

	// Emit execution modes
	for (auto &entrypoint : ir.entry_points)
	{
		auto function_id = entrypoint.first;
		auto function_info = entrypoint.second;

		switch (function_info.flags.get_lower())
		{
		case ExecutionModeInvocations:
			this->emit_opcode(Op::OpExecutionMode, 3);
			this->emit_id(function_id);
			this->emit_id(ExecutionModeInvocations);
			this->emit_id(function_info.invocations);
			break;

		case ExecutionModeLocalSize:
			this->emit_opcode(Op::OpExecutionMode, 5);
			this->emit_id(function_id);
			this->emit_id(ExecutionModeLocalSize);
			this->emit_id(function_info.workgroup_size.x);
			this->emit_id(function_info.workgroup_size.y);
			this->emit_id(function_info.workgroup_size.z);
			break;

		case ExecutionModeOutputVertices:
			this->emit_opcode(Op::OpExecutionMode, 3);
			this->emit_id(function_id);
			this->emit_id(ExecutionModeOutputVertices);
			this->emit_id(function_info.output_vertices);
			break;

		case ExecutionModeOutputPrimitivesEXT:
			this->emit_opcode(Op::OpExecutionMode, 3);
			this->emit_id(function_id);
			this->emit_id(ExecutionModeOutputPrimitivesEXT);
			this->emit_id(function_info.output_primitives);
			break;

		default:
			break;
		}
	}

	// Emit execution mode IDs
	for (auto &entrypoint : ir.entry_points)
	{
		auto function_id = entrypoint.first;
		auto function_info = entrypoint.second;

		switch (function_info.flags.get_lower())
		{
		case ExecutionModeLocalSizeId:
			this->emit_opcode(Op::OpExecutionMode, 5);
			this->emit_id(function_id);
			this->emit_id(ExecutionModeLocalSizeId);
			this->emit_id(function_info.workgroup_size.id_x);
			this->emit_id(function_info.workgroup_size.id_y);
			this->emit_id(function_info.workgroup_size.id_z);
			break;

		default:
			break;
		}
	}
}

void CompilerSPIRV::emit_types()
{
	ir.for_each_typed_id<SPIRType>(
	    [this](ID id, SPIRType type)
	    {
		    switch (type.type)
		    {
			default:
				break;

			case SPIRType::Void:
			    this->emit_opcode(type.op, 1);
				this->emit_id(id);
				break;

			// We fall-through
			case SPIRType::Int64:
			case SPIRType::Int:
			case SPIRType::Short:
			case SPIRType::SByte:
				this->emit_opcode(type.op, 3);
				this->emit_id(id);
				this->emit_id(type.width);
				this->emit_id(1); // Signed
				break;

			case SPIRType::UByte:
			case SPIRType::UShort:
			case SPIRType::UInt:
		    case SPIRType::UInt64:
			    this->emit_opcode(type.op, 3);
				this->emit_id(id);
				this->emit_id(type.width);
				this->emit_id(0); // Unsigned
				break;

			case SPIRType::Half:
			case SPIRType::Float:
			case SPIRType::Double:
			    this->emit_opcode(type.op, 2);
				this->emit_id(id);
				this->emit_id(type.width);
			    break;

			// Apparently vectors, matrices, etc. are "unknown".
			case SPIRType::Unknown:
			    if (type.columns > 1)
			    {
				    this->emit_opcode(type.op, 3);
				    this->emit_id(id);
				    this->emit_id(this->get_type(type.parent_type).self);
				    this->emit_id(type.columns);
				}
			    else if (type.vecsize > 1)
			    {
				    this->emit_opcode(type.op, 3);
				    this->emit_id(id);
				    this->emit_id(this->get_type(type.parent_type).self);
				    this->emit_id(type.vecsize);
				}
				break;

		    case SPIRType::Struct:
			    this->emit_opcode(type.op, 1+type.member_types.size());
			    this->emit_id(id);
			    for (auto id : type.member_types)
			    {
				    this->emit_id(this->get_type(id).self);
				}
			    break;
			}
	    }
	);
}


void CompilerSPIRV::emit_function_defs()
{

}

void CompilerSPIRV::emit_functions()
{

}

void CompilerSPIRV::emit_opcode(Op opcode, uint32_t arglength)
{
	buffer.push_back(opcode | ((arglength+1) << 16));
}

void CompilerSPIRV::emit_id(uint32_t id)
{
	buffer.push_back(id);
}

void CompilerSPIRV::emit_string(vector<uint32_t> str)
{
	buffer.insert(buffer.end(), str.begin(), str.end());
}

vector<uint32_t> CompilerSPIRV::get_spirv_string(string str)
{
	vector<uint32_t> tmp;
	uint32_t out_char = 0;
	size_t acc = 0;

	for (char c : str)
	{
		if (acc == 4) {
			acc = 0;
			tmp.push_back(out_char);
			out_char = 0;
		}

		out_char |= (c << (acc++ * 8));
	}
	
	if (out_char != 0)
	{
		tmp.push_back(out_char);
	}
	return tmp;
}