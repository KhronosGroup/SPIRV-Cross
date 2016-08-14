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

void CompilerHLSL::emit_header()
{
	auto &execution = get_entry_point();

	for (auto &header : header_lines)
		statement(header);
		
	statement("");
}

void CompilerHLSL::emit_interface_block(const SPIRVariable &var, uint32_t &binding_number)
{
	auto &execution = get_entry_point();
	auto &type = get<SPIRType>(var.basetype);

	const char* binding = "TEXCOORD";
	if (is_builtin_variable(var) || var.remapped_variable)
	{
		binding = "COLOR";
	}
	
	add_resource_name(var.self);
	statement(layout_for_variable(var), variable_decl(var), " : ", binding, binding_number, ";");

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
				emit_interface_block(var, binding_number);
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
				emit_interface_block(var, binding_number);
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

		pass_count++;
	} while (force_recompile);

	return buffer->str();
}
