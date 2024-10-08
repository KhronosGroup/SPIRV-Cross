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
	this->emit_annotations();
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

			this->emit_opcode(Op::OpExtInstImport, 1+ext_name.size());
			this->emit_id(id);
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

		this->emit_opcode(Op::OpEntryPoint, 2 + function_info.interface_variables.size() + spirv_name.size());
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

bool CompilerSPIRV::has_decorations(Meta::Decoration decoration)
{
	return decoration.decoration_flags.get_lower() != 0;
}

vector<Decoration> CompilerSPIRV::get_decoration_list(Meta::Decoration decoration)
{
	vector<Decoration> out_;

	auto flags = decoration.decoration_flags;
	if (flags.get(DecorationBuiltIn) && decoration.builtin)
		out_.push_back(DecorationBuiltIn);

	if (flags.get(DecorationLocation))
		out_.push_back(DecorationLocation);

	if (flags.get(DecorationComponent))
		out_.push_back(DecorationComponent);

	if (flags.get(DecorationBinding))
		out_.push_back(DecorationBinding);

	if (flags.get(DecorationOffset))
		out_.push_back(DecorationOffset);

	if (flags.get(DecorationXfbBuffer))
		out_.push_back(DecorationXfbBuffer);

	if (flags.get(DecorationXfbStride))
		out_.push_back(DecorationXfbStride);

	if (flags.get(DecorationStream))
		out_.push_back(DecorationStream);

	if (flags.get(DecorationSpecId))
		out_.push_back(DecorationSpecId);

	if (flags.get(DecorationMatrixStride))
		out_.push_back(DecorationMatrixStride);

	if (flags.get(DecorationIndex))
		out_.push_back(DecorationIndex);

	return out_;
}

void CompilerSPIRV::emit_decorations_operand(Decoration decoration, Meta::Decoration meta)
{
	this->emit_id(decoration);
	switch (decoration)
	{

	case DecorationBuiltIn:
		this->emit_id(meta.builtin_type);
		break;

	case DecorationLocation:
		this->emit_id(meta.location);
		break;

	case DecorationComponent:
		this->emit_id(meta.component);
		break;

	case DecorationBinding:
		this->emit_id(meta.binding);
		break;

	case DecorationOffset:
		this->emit_id(meta.offset);
		break;

	case DecorationXfbBuffer:
		this->emit_id(meta.xfb_buffer);
		break;

	case DecorationXfbStride:
		this->emit_id(meta.xfb_stride);
		break;

	case DecorationStream:
		this->emit_id(meta.stream);
		break;

	case DecorationSpecId:
		this->emit_id(meta.spec_id);
		break;

	case DecorationMatrixStride:
		this->emit_id(meta.matrix_stride);
		break;

	case DecorationIndex:
		this->emit_id(meta.index);
		break;

	default:
		break;
	}
}

void CompilerSPIRV::emit_decorations(ID id, Meta::Decoration decoration)
{
	for (auto &decor : this->get_decoration_list(decoration))
	{
		this->emit_opcode(Op::OpDecorate, 3);
		this->emit_id(id);
		this->emit_decorations_operand(decor, decoration);
	}
}

void CompilerSPIRV::emit_decorations_member(uint32_t offset, ID id, Meta::Decoration decoration)
{
	for (auto &decor : this->get_decoration_list(decoration))
	{
		this->emit_opcode(Op::OpMemberDecorate, 4);
		this->emit_id(id);
		this->emit_id(offset);
		this->emit_decorations_operand(decor, decoration);
	}
}

void CompilerSPIRV::emit_annotations()
{
	for (auto &meta : ir.meta)
	{
		auto meta_id = meta.first;
		auto meta_obj = meta.second;

		if (this->has_decorations(meta_obj.decoration))
			this->emit_decorations(meta_id, meta_obj.decoration);
		if (meta_obj.members.size() > 0)
		{
			uint32_t i = 0;
			for (auto &member : meta_obj.members)
			{
				if (this->has_decorations(member))
					this->emit_decorations_member(i, meta_id, member);

				i++;
			}
		}
	}
}

void CompilerSPIRV::emit_types()
{
	ir.for_each_typed_id<SPIRType>(
	    [this](ID id, SPIRType type)
	    {
		    uint32_t signed_ = 0;
		    switch (type.op)
		    {
			default:
			    break;

			case Op::OpTypeSampler:
		    case Op::OpTypeVoid:
			    this->emit_opcode(type.op, 1);
			    this->emit_id(id);
			    break;

		    case Op::OpTypeInt:
			    signed_ = (type.basetype == SPIRType::SByte || type.basetype == SPIRType::Short ||
			                        type.basetype == SPIRType::Int || type.basetype == SPIRType::Int64);
			    this->emit_opcode(type.op, 3);
			    this->emit_id(id);
			    this->emit_id(type.width);
			    this->emit_id(signed_); // Signed
			    break;

		    case Op::OpTypeFloat:
			    this->emit_opcode(Op::OpTypeFloat, 2);
			    this->emit_id(id);
			    this->emit_id(type.width);
			    break;

		    case Op::OpTypeVector:
			    this->emit_opcode(type.op, 3);
			    this->emit_id(id);
			    this->emit_id(type.parent_type);
			    this->emit_id(type.vecsize);
			    break;

		    case Op::OpTypeMatrix:
			    this->emit_opcode(type.op, 3);
			    this->emit_id(id);
			    this->emit_id(type.parent_type);
			    this->emit_id(type.columns);
			    break;

			case Op::OpTypeImage:
			    this->emit_opcode(type.op, 9);
			    this->emit_id(id);
			    this->emit_id(type.image.sampled);
			    this->emit_id(type.image.dim);
			    this->emit_id(type.image.depth);
			    this->emit_id(type.image.arrayed);
			    this->emit_id(type.image.ms);
			    this->emit_id(type.image.sampled);
			    this->emit_id(type.image.format);
			    this->emit_id(type.image.access);
				break;

			case Op::OpTypeSampledImage:
			    this->emit_opcode(type.op, 2);
			    this->emit_id(id);
			    this->emit_id(type.parent_type);
			    break;

			case Op::OpTypeArray:
			    if (type.array_size_literal[0])
			    {
					this->emit_opcode(type.op, 3);
					this->emit_id(id);
					this->emit_id(type.parent_type);
				    this->emit_id(type.array[0]);
			    }
			    else
			    {
				    this->emit_opcode(type.op, 2);
				    this->emit_id(id);
				    this->emit_id(type.parent_type);
			    }
			    break;

		    case Op::OpTypeStruct:
			    this->emit_opcode(type.op, 1+type.member_types.size());
			    this->emit_id(id);
			    for (auto member_id : type.member_types)
			    {
				    this->emit_id(member_id);
				}
			    break;


			case Op::OpTypePointer:
			    this->emit_opcode(type.op, 3);
			    this->emit_id(id);
			    this->emit_id(type.storage);
			    this->emit_id(type.parent_type);
			    break;
			}
	    }
	);

	ir.for_each_typed_id<SPIRConstant>(
		[this](ID id, SPIRConstant const_) 
		{ 
			Op opcode;
		    auto type = this->get_type(const_.constant_type);

			// SPIRConstant does itself not fully know what type it is.
			// So we'll go looking.
			// Additionally, we don't care what kind of float
			// or integer scalar types are, so we use the op as the switch key.
		    switch (type.op)
		    {
		    case Op::OpTypeBool:
				
			    opcode = const_.scalar_u8() == 1 ? 
					Op::OpConstantTrue :
					Op::OpConstantFalse;

			    this->emit_opcode(opcode, 2);
			    this->emit_id(const_.constant_type);
			    this->emit_id(id);
			    break;

			
		    case Op::OpTypeInt:
		    case Op::OpTypeFloat:
			    this->emit_opcode(type.op, 3);
			    this->emit_id(const_.constant_type);
			    this->emit_id(id);
			    this->emit_id(const_.scalar());
			    break;


			}
		}
	);

	ir.for_each_typed_id<SPIRFunction>(
		[this] (ID id, SPIRFunction func) { 
			this->emit_opcode(Op::OpTypeFunction, 2 + func.arguments.size());
		    this->emit_id(func.function_type);
		    this->emit_id(func.return_type);
		    for (auto &param : func.arguments)
		    {
			    this->emit_id(param.type);
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
	buffer.push_back(opcode + ((arglength+1) << 16));
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
		out_char |= (c << (acc++ * 8));
		if (acc == 4)
		{
			tmp.push_back(out_char);
			acc = 0;
			out_char = 0;
		}
	}

	tmp.push_back(out_char);
	return tmp;
}