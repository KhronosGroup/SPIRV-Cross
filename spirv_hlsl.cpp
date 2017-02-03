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

#include "spirv_hlsl.hpp"
#include "GLSL.std.450.h"
#include <algorithm>

using namespace spv;
using namespace spirv_cross;
using namespace std;

namespace
{
struct VariableComparator
{
	VariableComparator(const std::vector<Meta> &m)
	    : meta(m)
	{
	}

	bool operator()(SPIRVariable *var1, SPIRVariable *var2)
	{
		return meta[var1->self].decoration.alias.compare(meta[var2->self].decoration.alias) < 0;
	}

	const std::vector<Meta> &meta;
};
}

string CompilerHLSL::type_to_glsl(const SPIRType &type)
{
	// Ignore the pointer type since GLSL doesn't have pointers.

	switch (type.basetype)
	{
	case SPIRType::Struct:
		// Need OpName lookup here to get a "sensible" name for a struct.
		if (backend.explicit_struct_type)
			return join("struct ", to_name(type.self));
		else
			return to_name(type.self);

	case SPIRType::Image:
	case SPIRType::SampledImage:
		return image_type_glsl(type);

	case SPIRType::Sampler:
		// Not really used.
		return "sampler";

	case SPIRType::Void:
		return "void";

	default:
		break;
	}

	if (type.vecsize == 1 && type.columns == 1) // Scalar builtin
	{
		switch (type.basetype)
		{
		case SPIRType::Boolean:
			return "bool";
		case SPIRType::Int:
			return backend.basic_int_type;
		case SPIRType::UInt:
			return backend.basic_uint_type;
		case SPIRType::AtomicCounter:
			return "atomic_uint";
		case SPIRType::Float:
			return "float";
		case SPIRType::Double:
			return "double";
		case SPIRType::Int64:
			return "int64_t";
		case SPIRType::UInt64:
			return "uint64_t";
		default:
			return "???";
		}
	}
	else if (type.vecsize > 1 && type.columns == 1) // Vector builtin
	{
		switch (type.basetype)
		{
		case SPIRType::Boolean:
			return join("bool", type.vecsize);
		case SPIRType::Int:
			return join("int", type.vecsize);
		case SPIRType::UInt:
			return join("uint", type.vecsize);
		case SPIRType::Float:
			return join("float", type.vecsize);
		case SPIRType::Double:
			return join("double", type.vecsize);
		case SPIRType::Int64:
			return join("i64vec", type.vecsize);
		case SPIRType::UInt64:
			return join("u64vec", type.vecsize);
		default:
			return "???";
		}
	}
	else
	{
		switch (type.basetype)
		{
		case SPIRType::Boolean:
			return join("bool", type.columns, "x", type.vecsize);
		case SPIRType::Int:
			return join("int", type.columns, "x", type.vecsize);
		case SPIRType::UInt:
			return join("uint", type.columns, "x", type.vecsize);
		case SPIRType::Float:
			return join("float", type.columns, "x", type.vecsize);
		case SPIRType::Double:
			return join("double", type.columns, "x", type.vecsize);
		// Matrix types not supported for int64/uint64.
		default:
			return "???";
		}
	}
}

void CompilerHLSL::emit_header()
{
	for (auto &header : header_lines)
		statement(header);

	if (header_lines.size() > 0)
	{
		statement("");
	}
}

void CompilerHLSL::emit_interface_block_globally(const SPIRVariable &var)
{
	auto &execution = get_entry_point();

	add_resource_name(var.self);

	if (execution.model == ExecutionModelVertex && var.storage == StorageClassInput && is_builtin_variable(var))
	{
	}
	else if (execution.model == ExecutionModelVertex && var.storage == StorageClassOutput && is_builtin_variable(var))
	{
		statement("static float4 gl_Position;");
	}
	else
	{
		statement("static ", variable_decl(var), ";");
	}
}

void CompilerHLSL::emit_interface_block_in_struct(const SPIRVariable &var, uint32_t &binding_number, bool builtins)
{
	auto &execution = get_entry_point();
	auto &type = get<SPIRType>(var.basetype);

	const char *binding = "TEXCOORD";
	bool use_binding_number = true;
	if (execution.model == ExecutionModelFragment && var.storage == StorageClassOutput)
	{
		binding = "COLOR";
		use_binding_number = false;
	}
	else if (execution.model == ExecutionModelVertex && var.storage == StorageClassOutput && is_builtin_variable(var))
	{
		if (options.shader_model <= 30)
		{
			binding = "POSITION";
		}
		else
		{
			binding = "SV_POSITION";
		}
		use_binding_number = false;
	}

	bool is_no_builtin = !is_builtin_variable(var) && !var.remapped_variable;

	if ((is_no_builtin && !builtins) || (!is_no_builtin && builtins))
	{
		auto &m = meta[var.self].decoration;
		if (use_binding_number)
		{
			if (type.vecsize == 4 && type.columns == 4)
			{
				for (int i = 0; i < 4; ++i)
				{
					std::stringstream name;
					name << m.alias << "_" << i;
					SPIRType newtype = type;
					newtype.columns = 1;
					statement(variable_decl(newtype, name.str()), " : ", binding, binding_number++, ";");
				}
				--binding_number;
			}
			else
			{
				statement(variable_decl(type, m.alias), " : ", binding, binding_number, ";");
			}
		}
		else
		{
			if (execution.model == ExecutionModelVertex)
			{
				statement("float4 gl_Position", " : ", binding, ";");
			}
			else
			{
				statement(variable_decl(type, m.alias), " : ", binding, ";");
			}
		}
	}

	if (is_no_builtin)
	{
		++binding_number;
	}
}

void CompilerHLSL::emit_resources()
{
	auto &execution = get_entry_point();

	// Emit PLS blocks if we have such variables.
	if (!pls_inputs.empty() || !pls_outputs.empty())
		emit_pls();

	// Output all basic struct types which are not Block or BufferBlock as these are declared inplace
	// when such variables are instantiated.
	for (auto &id : ids)
	{
		if (id.get_type() == TypeType)
		{
			auto &type = id.get<SPIRType>();
			if (type.basetype == SPIRType::Struct && type.array.empty() && !type.pointer &&
			    (meta[type.self].decoration.decoration_flags &
			     ((1ull << DecorationBlock) | (1ull << DecorationBufferBlock))) == 0)
			{
				emit_struct(type);
			}
		}
	}

	bool emitted = false;

	// Output UBOs and SSBOs
	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			auto &type = get<SPIRType>(var.basetype);

			if (var.storage != StorageClassFunction && type.pointer && type.storage == StorageClassUniform &&
			    !is_hidden_variable(var) && (meta[type.self].decoration.decoration_flags &
			                                 ((1ull << DecorationBlock) | (1ull << DecorationBufferBlock))))
			{
				emit_buffer_block(var);
				emitted = true;
			}
		}
	}

	// Output push constant blocks
	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			auto &type = get<SPIRType>(var.basetype);
			if (var.storage != StorageClassFunction && type.pointer && type.storage == StorageClassPushConstant &&
			    !is_hidden_variable(var))
			{
				emit_push_constant_block(var);
				emitted = true;
			}
		}
	}

	if (execution.model == ExecutionModelVertex && options.shader_model <= 30)
	{
		statement("uniform float4 gl_HalfPixel;");
		emitted = true;
	}

	// Output Uniform Constants (values, samplers, images, etc).
	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			auto &type = get<SPIRType>(var.basetype);

			if (var.storage != StorageClassFunction && !is_builtin_variable(var) && !var.remapped_variable &&
			    type.pointer &&
			    (type.storage == StorageClassUniformConstant || type.storage == StorageClassAtomicCounter))
			{
				emit_uniform(var);
				emitted = true;
			}
		}
	}

	if (emitted)
		statement("");
	emitted = false;

	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			auto &type = get<SPIRType>(var.basetype);

			if (var.storage != StorageClassFunction && !var.remapped_variable && type.pointer &&
			    (var.storage == StorageClassInput || var.storage == StorageClassOutput) &&
			    interface_variable_exists_in_entry_point(var.self))
			{
				emit_interface_block_globally(var);
				emitted = true;
			}
		}
	}

	if (emitted)
		statement("");
	emitted = false;

	if (execution.model == ExecutionModelVertex)
	{
		statement("struct InputVert");
	}
	else
	{
		statement("struct InputFrag");
	}
	begin_scope();
	uint32_t binding_number = 0;
	std::vector<SPIRVariable *> variables;
	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			auto &type = get<SPIRType>(var.basetype);

			if (var.storage != StorageClassFunction && !var.remapped_variable && type.pointer &&
			    var.storage == StorageClassInput && interface_variable_exists_in_entry_point(var.self))
			{
				if (execution.model == ExecutionModelVertex && is_builtin_variable(var))
					continue;
				variables.push_back(&var);
			}
		}
	}
	sort(variables.begin(), variables.end(), VariableComparator(meta));
	for (auto var : variables)
	{
		emit_interface_block_in_struct(*var, binding_number, false);
	}
	for (auto var : variables)
	{
		emit_interface_block_in_struct(*var, binding_number, true);
	}
	end_scope_decl();
	statement("");

	if (execution.model == ExecutionModelVertex)
	{
		statement("struct OutputVert");
	}
	else
	{
		statement("struct OutputFrag");
	}
	begin_scope();
	binding_number = 0;
	variables.clear();
	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			auto &type = get<SPIRType>(var.basetype);

			if (var.storage != StorageClassFunction && !var.remapped_variable && type.pointer &&
			    var.storage == StorageClassOutput && interface_variable_exists_in_entry_point(var.self))
			{
				variables.push_back(&var);
			}
		}
	}
	sort(variables.begin(), variables.end(), VariableComparator(meta));
	for (auto var : variables)
	{
		emit_interface_block_in_struct(*var, binding_number, false);
	}
	for (auto var : variables)
	{
		emit_interface_block_in_struct(*var, binding_number, true);
	}
	end_scope_decl();
	statement("");

	// Global variables.
	for (auto global : global_variables)
	{
		auto &var = get<SPIRVariable>(global);
		if (var.storage != StorageClassOutput)
		{
			add_resource_name(var.self);
			statement("static ", variable_decl(var), ";");
			emitted = true;
		}
	}

	if (emitted)
		statement("");

	if (requires_op_fmod)
	{
		statement("float mod(float x, float y)");
		begin_scope();
		statement("return x - y * floor(x / y);");
		end_scope();
		statement("");
	}
}

void CompilerHLSL::emit_buffer_block(const SPIRVariable &var)
{
	auto &type = get<SPIRType>(var.basetype);

	add_resource_name(type.self);

	statement("struct _", to_name(type.self));
	begin_scope();

	type.member_name_cache.clear();

	uint32_t i = 0;
	for (auto &member : type.member_types)
	{
		add_member_name(type, i);

		auto &membertype = get<SPIRType>(member);
		statement(member_decl(type, membertype, i), ";");
		i++;
	}
	end_scope_decl();
	statement("");

	statement("cbuffer ", to_name(type.self));
	begin_scope();
	statement("_", to_name(type.self), " ", to_name(var.self), ";");
	end_scope_decl();
}

void CompilerHLSL::emit_push_constant_block(const SPIRVariable &)
{
	statement("constant block");
}

void CompilerHLSL::emit_function_prototype(SPIRFunction &func, uint64_t return_flags)
{
	auto &execution = get_entry_point();
	// Avoid shadow declarations.
	local_variable_names = resource_names;

	string decl;

	auto &type = get<SPIRType>(func.return_type);
	decl += flags_to_precision_qualifiers_glsl(type, return_flags);
	decl += type_to_glsl(type);
	decl += " ";

	if (func.self == entry_point)
	{
		if (execution.model == ExecutionModelVertex)
		{
			decl += "vert_main";
		}
		else
		{
			decl += "frag_main";
		}
		processing_entry_point = true;
	}
	else
		decl += to_name(func.self);

	decl += "(";
	for (auto &arg : func.arguments)
	{
		// Might change the variable name if it already exists in this function.
		// SPIRV OpName doesn't have any semantic effect, so it's valid for an implementation
		// to use same name for variables.
		// Since we want to make the GLSL debuggable and somewhat sane, use fallback names for variables which are duplicates.
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

void CompilerHLSL::emit_hlsl_entry_point()
{
	auto &execution = get_entry_point();
	const char *post = "Frag";
	if (execution.model == ExecutionModelVertex)
	{
		post = "Vert";
	}

	statement("Output", post, " main(Input", post, " input)");
	begin_scope();

	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			auto &type = get<SPIRType>(var.basetype);

			if (var.storage != StorageClassFunction && !var.remapped_variable && type.pointer &&
			    var.storage == StorageClassInput && interface_variable_exists_in_entry_point(var.self))
			{
				if (execution.model == ExecutionModelVertex && is_builtin_variable(var))
					continue;

				auto &m = meta[var.self].decoration;
				auto &mtype = get<SPIRType>(var.basetype);
				if (mtype.vecsize == 4 && mtype.columns == 4)
				{
					statement(m.alias, "[0] = input.", m.alias, "_0;");
					statement(m.alias, "[1] = input.", m.alias, "_1;");
					statement(m.alias, "[2] = input.", m.alias, "_2;");
					statement(m.alias, "[3] = input.", m.alias, "_3;");
				}
				else
				{
					statement(m.alias, " = input.", m.alias, ";");
				}
			}
		}
	}

	if (execution.model == ExecutionModelVertex)
	{
		statement("vert_main();");
	}
	else
	{
		statement("frag_main();");
	}

	statement("Output", post, " output;");

	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			auto &type = get<SPIRType>(var.basetype);

			if (var.storage != StorageClassFunction && !var.remapped_variable && type.pointer &&
			    var.storage == StorageClassOutput && interface_variable_exists_in_entry_point(var.self))
			{
				auto &m = meta[var.self].decoration;
				bool is_no_builtin = !is_builtin_variable(var) && !var.remapped_variable;
				if (is_no_builtin)
					statement("output.", m.alias, " = ", m.alias, ";");
				else if (execution.model == ExecutionModelVertex)
				{
					statement("output.gl_Position = gl_Position;");
				}
			}
		}
	}

	if (execution.model == ExecutionModelVertex)
	{
		if (options.shader_model <= 30)
		{
			statement("output.gl_Position.x = output.gl_Position.x - gl_HalfPixel.x * output.gl_Position.w;");
			statement("output.gl_Position.y = output.gl_Position.y + gl_HalfPixel.y * output.gl_Position.w;");
		}
		if (options.flip_vert_y)
		{
			statement("output.gl_Position.y = -output.gl_Position.y;");
		}
		if (options.fixup_clipspace)
		{
			statement("output.gl_Position.z = (output.gl_Position.z + output.gl_Position.w) * 0.5;");
		}
	}

	statement("return output;");

	end_scope();
}

void CompilerHLSL::emit_texture_op(const Instruction &i)
{
	auto ops = stream(i);
	auto op = static_cast<Op>(i.op);
	uint32_t length = i.length;

	if (i.offset + length > spirv.size())
		SPIRV_CROSS_THROW("Compiler::parse() opcode out of range.");

	uint32_t result_type = ops[0];
	uint32_t id = ops[1];
	uint32_t img = ops[2];
	uint32_t coord = ops[3];
	uint32_t dref = 0;
	uint32_t comp = 0;
	bool gather = false;
	bool proj = false;
	const uint32_t *opt = nullptr;

	switch (op)
	{
	case OpImageSampleDrefImplicitLod:
	case OpImageSampleDrefExplicitLod:
		dref = ops[4];
		opt = &ops[5];
		length -= 5;
		break;

	case OpImageSampleProjDrefImplicitLod:
	case OpImageSampleProjDrefExplicitLod:
		dref = ops[4];
		proj = true;
		opt = &ops[5];
		length -= 5;
		break;

	case OpImageDrefGather:
		dref = ops[4];
		opt = &ops[5];
		gather = true;
		length -= 5;
		break;

	case OpImageGather:
		comp = ops[4];
		opt = &ops[5];
		gather = true;
		length -= 5;
		break;

	case OpImageSampleProjImplicitLod:
	case OpImageSampleProjExplicitLod:
		opt = &ops[4];
		length -= 4;
		proj = true;
		break;

	default:
		opt = &ops[4];
		length -= 4;
		break;
	}

	auto &imgtype = expression_type(img);
	uint32_t coord_components = 0;
	switch (imgtype.image.dim)
	{
	case spv::Dim1D:
		coord_components = 1;
		break;
	case spv::Dim2D:
		coord_components = 2;
		break;
	case spv::Dim3D:
		coord_components = 3;
		break;
	case spv::DimCube:
		coord_components = 3;
		break;
	case spv::DimBuffer:
		coord_components = 1;
		break;
	default:
		coord_components = 2;
		break;
	}

	if (proj)
		coord_components++;
	if (imgtype.image.arrayed)
		coord_components++;

	uint32_t bias = 0;
	uint32_t lod = 0;
	uint32_t grad_x = 0;
	uint32_t grad_y = 0;
	uint32_t coffset = 0;
	uint32_t offset = 0;
	uint32_t coffsets = 0;
	uint32_t sample = 0;
	uint32_t flags = 0;

	if (length)
	{
		flags = opt[0];
		opt++;
		length--;
	}

	auto test = [&](uint32_t &v, uint32_t flag) {
		if (length && (flags & flag))
		{
			v = *opt++;
			length--;
		}
	};

	test(bias, ImageOperandsBiasMask);
	test(lod, ImageOperandsLodMask);
	test(grad_x, ImageOperandsGradMask);
	test(grad_y, ImageOperandsGradMask);
	test(coffset, ImageOperandsConstOffsetMask);
	test(offset, ImageOperandsOffsetMask);
	test(coffsets, ImageOperandsConstOffsetsMask);
	test(sample, ImageOperandsSampleMask);

	string expr;
	string texop;

	if (op == OpImageFetch)
		texop += "texelFetch";
	else
	{
		texop += "tex2D";

		if (gather)
			texop += "Gather";
		if (coffsets)
			texop += "Offsets";
		if (proj)
			texop += "Proj";
		if (grad_x || grad_y)
			texop += "Grad";
		if (lod)
			texop += "Lod";
	}

	if (coffset || offset)
		texop += "Offset";

	if (is_legacy())
		texop = legacy_tex_op(texop, imgtype);

	expr += texop;
	expr += "(";
	expr += to_expression(img);

	bool swizz_func = backend.swizzle_is_function;
	auto swizzle = [swizz_func](uint32_t comps, uint32_t in_comps) -> const char * {
		if (comps == in_comps)
			return "";

		switch (comps)
		{
		case 1:
			return ".x";
		case 2:
			return swizz_func ? ".xy()" : ".xy";
		case 3:
			return swizz_func ? ".xyz()" : ".xyz";
		default:
			return "";
		}
	};

	bool forward = should_forward(coord);

	// The IR can give us more components than we need, so chop them off as needed.
	auto coord_expr = to_expression(coord) + swizzle(coord_components, expression_type(coord).vecsize);

	// TODO: implement rest ... A bit intensive.

	if (dref)
	{
		forward = forward && should_forward(dref);

		// SPIR-V splits dref and coordinate.
		if (coord_components == 4) // GLSL also splits the arguments in two.
		{
			expr += ", ";
			expr += to_expression(coord);
			expr += ", ";
			expr += to_expression(dref);
		}
		else
		{
			// Create a composite which merges coord/dref into a single vector.
			auto type = expression_type(coord);
			type.vecsize = coord_components + 1;
			expr += ", ";
			expr += type_to_glsl_constructor(type);
			expr += "(";
			expr += coord_expr;
			expr += ", ";
			expr += to_expression(dref);
			expr += ")";
		}
	}
	else
	{
		expr += ", ";
		expr += coord_expr;
	}

	if (grad_x || grad_y)
	{
		forward = forward && should_forward(grad_x);
		forward = forward && should_forward(grad_y);
		expr += ", ";
		expr += to_expression(grad_x);
		expr += ", ";
		expr += to_expression(grad_y);
	}

	if (lod)
	{
		forward = forward && should_forward(lod);
		expr += ", ";
		expr += to_expression(lod);
	}

	if (coffset)
	{
		forward = forward && should_forward(coffset);
		expr += ", ";
		expr += to_expression(coffset);
	}
	else if (offset)
	{
		forward = forward && should_forward(offset);
		expr += ", ";
		expr += to_expression(offset);
	}

	if (bias)
	{
		forward = forward && should_forward(bias);
		expr += ", ";
		expr += to_expression(bias);
	}

	if (comp)
	{
		forward = forward && should_forward(comp);
		expr += ", ";
		expr += to_expression(comp);
	}

	if (sample)
	{
		expr += ", ";
		expr += to_expression(sample);
	}

	expr += ")";

	emit_op(result_type, id, expr, forward, false);
}

void CompilerHLSL::emit_uniform(const SPIRVariable &var)
{
	add_resource_name(var.self);
	statement("uniform ", variable_decl(var), ";");
}

void CompilerHLSL::emit_glsl_op(uint32_t result_type, uint32_t id, uint32_t eop, const uint32_t *args, uint32_t count)
{
	GLSLstd450 op = static_cast<GLSLstd450>(eop);

	switch (op)
	{
	case GLSLstd450InverseSqrt:
	{
		emit_unary_func_op(result_type, id, args[0], "rsqrt");
		break;
	}
	case GLSLstd450Fract:
	{
		emit_unary_func_op(result_type, id, args[0], "frac");
		break;
	}
	case GLSLstd450FMix:
	case GLSLstd450IMix:
	{
		emit_trinary_func_op(result_type, id, args[0], args[1], args[2], "lerp");
		break;
	}
	case GLSLstd450Atan2:
	{
		emit_binary_func_op(result_type, id, args[1], args[0], "atan2");
		break;
	}
	default:
		CompilerGLSL::emit_glsl_op(result_type, id, eop, args, count);
		break;
	}
}

void CompilerHLSL::emit_instruction(const Instruction &instruction)
{
	auto ops = stream(instruction);
	auto opcode = static_cast<Op>(instruction.op);

#define BOP(op) emit_binary_op(ops[0], ops[1], ops[2], ops[3], #op)
#define BOP_CAST(op, type, skip_cast) emit_binary_op_cast(ops[0], ops[1], ops[2], ops[3], #op, type, skip_cast)
#define UOP(op) emit_unary_op(ops[0], ops[1], ops[2], #op)
#define QFOP(op) emit_quaternary_func_op(ops[0], ops[1], ops[2], ops[3], ops[4], ops[5], #op)
#define TFOP(op) emit_trinary_func_op(ops[0], ops[1], ops[2], ops[3], ops[4], #op)
#define BFOP(op) emit_binary_func_op(ops[0], ops[1], ops[2], ops[3], #op)
#define BFOP_CAST(op, type, skip_cast) emit_binary_func_op_cast(ops[0], ops[1], ops[2], ops[3], #op, type, skip_cast)
#define BFOP(op) emit_binary_func_op(ops[0], ops[1], ops[2], ops[3], #op)
#define UFOP(op) emit_unary_func_op(ops[0], ops[1], ops[2], #op)

	switch (opcode)
	{
	case OpMatrixTimesVector:
	{
		emit_binary_func_op(ops[0], ops[1], ops[3], ops[2], "mul");
		break;
	}
	case OpVectorTimesMatrix:
	{
		emit_binary_func_op(ops[0], ops[1], ops[3], ops[2], "mul");
		break;
	}
	case OpMatrixTimesMatrix:
	{
		emit_binary_func_op(ops[0], ops[1], ops[3], ops[2], "mul");
		break;
	}
	case OpFMod:
	{
		requires_op_fmod = true;
		CompilerGLSL::emit_instruction(instruction);
		break;
	}
	default:
		CompilerGLSL::emit_instruction(instruction);
		break;
	}
}

string CompilerHLSL::compile()
{
	// Do not deal with ES-isms like precision, older extensions and such.
	CompilerGLSL::options.es = false;
	CompilerGLSL::options.version = 450;
	backend.float_literal_suffix = true;
	backend.double_literal_suffix = false;
	backend.long_long_literal_suffix = true;
	backend.uint32_t_literal_suffix = true;
	backend.basic_int_type = "int";
	backend.basic_uint_type = "uint";
	backend.swizzle_is_function = false;
	backend.shared_is_implied = true;
	backend.flexible_member_array_supported = false;
	backend.explicit_struct_type = true;
	backend.use_initializer_list = true;
	backend.use_constructor_splatting = false;

	uint32_t pass_count = 0;
	do
	{
		if (pass_count >= 3)
			SPIRV_CROSS_THROW("Over 3 compilation loops detected. Must be a bug!");

		reset();

		// Move constructor for this type is broken on GCC 4.9 ...
		buffer = unique_ptr<ostringstream>(new ostringstream());

		emit_header();
		emit_resources();

		emit_function(get<SPIRFunction>(entry_point), 0);
		emit_hlsl_entry_point();

		pass_count++;
	} while (force_recompile);

	return buffer->str();
}
