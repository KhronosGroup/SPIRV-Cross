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

#include "spirv_hlsl.hpp"

using namespace spv;
using namespace spirv_cross;
using namespace std;

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
			return join("bfloat", type.vecsize);
		case SPIRType::Int:
			return join("ifloat", type.vecsize);
		case SPIRType::UInt:
			return join("ufloat", type.vecsize);
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
	else if (type.vecsize == type.columns) // Simple Matrix builtin
	{
		switch (type.basetype)
		{
		case SPIRType::Boolean:
			return join("bmat", type.vecsize);
		case SPIRType::Int:
			return join("imat", type.vecsize);
		case SPIRType::UInt:
			return join("umat", type.vecsize);
		case SPIRType::Float:
			return join("mat", type.vecsize);
		case SPIRType::Double:
			return join("dmat", type.vecsize);
			// Matrix types not supported for int64/uint64.
		default:
			return "???";
		}
	}
	else
	{
		switch (type.basetype)
		{
		case SPIRType::Boolean:
			return join("bmat", type.columns, "x", type.vecsize);
		case SPIRType::Int:
			return join("imat", type.columns, "x", type.vecsize);
		case SPIRType::UInt:
			return join("umat", type.columns, "x", type.vecsize);
		case SPIRType::Float:
			return join("mat", type.columns, "x", type.vecsize);
		case SPIRType::Double:
			return join("dmat", type.columns, "x", type.vecsize);
			// Matrix types not supported for int64/uint64.
		default:
			return "???";
		}
	}
}

void CompilerHLSL::emit_header()
{
	auto &execution = get_entry_point();

	for (auto &header : header_lines)
		statement(header);
		
	statement("");
}

void CompilerHLSL::emit_interface_block_globally(const SPIRVariable &var)
{
	auto &execution = get_entry_point();
	auto &type = get<SPIRType>(var.basetype);

	add_resource_name(var.self);
	statement("static ", variable_decl(var), ";");
}

void CompilerHLSL::emit_interface_block_in_struct(const SPIRVariable &var, uint32_t &binding_number)
{
	auto &execution = get_entry_point();
	auto &type = get<SPIRType>(var.basetype);

	const char* binding = "TEXCOORD";
	if (is_builtin_variable(var) || var.remapped_variable)
	{
		binding = "COLOR";
	}
	
	auto &m = meta[var.self].decoration;
	statement(variable_decl(type, m.alias), " : ", binding, binding_number, ";");

	if (!is_builtin_variable(var) && !var.remapped_variable)
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

			if (var.storage != StorageClassFunction && !var.remapped_variable &&
				type.pointer && (var.storage == StorageClassInput || var.storage == StorageClassOutput) &&
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
	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			auto &type = get<SPIRType>(var.basetype);

			if (var.storage != StorageClassFunction && !var.remapped_variable &&
				type.pointer && var.storage == StorageClassInput &&
				interface_variable_exists_in_entry_point(var.self))
			{
				emit_interface_block_in_struct(var, binding_number);
				emitted = true;
			}
		}
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
	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			auto &type = get<SPIRType>(var.basetype);

			if (var.storage != StorageClassFunction && !var.remapped_variable &&
				type.pointer && var.storage == StorageClassOutput &&
				interface_variable_exists_in_entry_point(var.self))
			{
				emit_interface_block_in_struct(var, binding_number);
				emitted = true;
			}
		}
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
	const char* post = "Frag";
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

			if (var.storage != StorageClassFunction && !var.remapped_variable &&
				type.pointer && var.storage == StorageClassInput &&
				interface_variable_exists_in_entry_point(var.self))
			{
				auto &m = meta[var.self].decoration;
				statement(m.alias, " = input.", m.alias, ";");
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

			if (var.storage != StorageClassFunction && !var.remapped_variable &&
				type.pointer && var.storage == StorageClassOutput &&
				interface_variable_exists_in_entry_point(var.self))
			{
				auto &m = meta[var.self].decoration;
				statement("output.", m.alias, " = ", m.alias, ";");
			}
		}
	}

	statement("return output;");

	end_scope();

	/*OutputFrag main(InputFrag input)
	{
		f_color = input.color;
		f_texCoord = input.texCoord;
		frag_main();
		OutputFrag output;
		output.gl_FragColor = f_gl_FragColor;
		return output;
	}*/

}

string CompilerHLSL::compile()
{
	// Do not deal with ES-isms like precision, older extensions and such.
	options.es = false;
	options.version = 450;
	backend.float_literal_suffix = true;
	backend.double_literal_suffix = false;
	backend.long_long_literal_suffix = true;
	backend.uint32_t_literal_suffix = true;
	backend.basic_int_type = "int32_t";
	backend.basic_uint_type = "uint32_t";
	backend.swizzle_is_function = true;
	backend.shared_is_implied = true;
	backend.flexible_member_array_supported = false;
	backend.explicit_struct_type = true;
	backend.use_initializer_list = true;
	
	uint32_t pass_count = 0;
	do
	{
		if (pass_count >= 3)
			throw CompilerError("Over 3 compilation loops detected. Must be a bug!");

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
