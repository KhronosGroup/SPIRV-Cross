/*
 * Copyright 2016-2017 The Brenwill Workshop Ltd.
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

#include "spirv_msl.hpp"
#include "GLSL.std.450.h"

#include <algorithm>
#include <cassert>
#include <numeric>

using namespace spv;
using namespace spirv_cross;
using namespace std;

static const uint32_t k_unknown_location = ~0;

CompilerMSL::CompilerMSL(vector<uint32_t> spirv_, vector<MSLVertexAttr> *p_vtx_attrs,
                         vector<MSLResourceBinding> *p_res_bindings)
    : CompilerGLSL(move(spirv_))
{
	populate_func_name_overrides();
	populate_var_name_overrides();

	if (p_vtx_attrs)
		for (auto &va : *p_vtx_attrs)
			vtx_attrs_by_location[va.location] = &va;

	if (p_res_bindings)
		for (auto &rb : *p_res_bindings)
			resource_bindings.push_back(&rb);
}

// Populate the collection of function names that need to be overridden
void CompilerMSL::populate_func_name_overrides()
{
	func_name_overrides["main"] = "main0";
	func_name_overrides["saturate"] = "saturate0";
}

void CompilerMSL::populate_var_name_overrides()
{
	var_name_overrides["kernel"] = "kernel0";
	var_name_overrides["bias"] = "bias0";
}

string CompilerMSL::compile()
{
	// Force a classic "C" locale, reverts when function returns
	ClassicLocale classic_locale;

	// Set main function name if it was explicitly set
	if (!options.entry_point_name.empty())
		set_name(entry_point, options.entry_point_name);

	non_stage_in_input_var_ids.clear();
	struct_member_padding.clear();

	update_active_builtins();

	// Preprocess OpCodes to extract the need to output additional header content
	set_enabled_interface_variables(get_active_interface_variables());
	preprocess_op_codes();

	// Create structs to hold input, output and uniform variables
	qual_pos_var_name = "";
	stage_in_var_id = add_interface_block(StorageClassInput);
	stage_out_var_id = add_interface_block(StorageClassOutput);
	stage_uniforms_var_id = add_interface_block(StorageClassUniformConstant);

	// Convert the use of global variables to recursively-passed function parameters
	localize_global_variables();
	extract_global_variables_from_functions();

	// Do not deal with GLES-isms like precision, older extensions and such.
	CompilerGLSL::options.es = false;
	CompilerGLSL::options.version = 120;
	CompilerGLSL::options.vertex.fixup_clipspace = false;
	backend.float_literal_suffix = false;
	backend.uint32_t_literal_suffix = true;
	backend.basic_int_type = "int";
	backend.basic_uint_type = "uint";
	backend.discard_literal = "discard_fragment()";
	backend.swizzle_is_function = false;
	backend.shared_is_implied = false;
	backend.native_row_major_matrix = false;

	uint32_t pass_count = 0;
	do
	{
		if (pass_count >= 3)
			SPIRV_CROSS_THROW("Over 3 compilation loops detected. Must be a bug!");

		reset();

		next_metal_resource_index = MSLResourceBinding(); // Start bindings at zero

		// Move constructor for this type is broken on GCC 4.9 ...
		buffer = unique_ptr<ostringstream>(new ostringstream());

		emit_header();
		emit_resources();
		emit_custom_functions();
		emit_function(get<SPIRFunction>(entry_point), 0);

		pass_count++;
	} while (force_recompile);

	return buffer->str();
}

string CompilerMSL::compile(vector<MSLVertexAttr> *p_vtx_attrs, vector<MSLResourceBinding> *p_res_bindings)
{
	if (p_vtx_attrs)
	{
		vtx_attrs_by_location.clear();
		for (auto &va : *p_vtx_attrs)
			vtx_attrs_by_location[va.location] = &va;
	}

	if (p_res_bindings)
	{
		resource_bindings.clear();
		for (auto &rb : *p_res_bindings)
			resource_bindings.push_back(&rb);
	}

	return compile();
}

string CompilerMSL::compile(MSLConfiguration &msl_cfg, vector<MSLVertexAttr> *p_vtx_attrs,
                            vector<MSLResourceBinding> *p_res_bindings)
{
	options = msl_cfg;
	return compile(p_vtx_attrs, p_res_bindings);
}

// Register the need to output any custom functions.
void CompilerMSL::preprocess_op_codes()
{
	custom_function_ops.clear();

	OpCodePreprocessor preproc(*this);
	traverse_all_reachable_opcodes(get<SPIRFunction>(entry_point), preproc);

	if (preproc.suppress_missing_prototypes)
		add_header_line("#pragma clang diagnostic ignored \"-Wmissing-prototypes\"");
}

// Move the Private global variables to the entry function.
// Non-constant variables cannot have global scope in Metal.
void CompilerMSL::localize_global_variables()
{
	auto &entry_func = get<SPIRFunction>(entry_point);
	auto iter = global_variables.begin();
	while (iter != global_variables.end())
	{
		uint32_t gv_id = *iter;
		auto &gbl_var = get<SPIRVariable>(gv_id);
		if (gbl_var.storage == StorageClassPrivate)
		{
			entry_func.add_local_variable(gv_id);
			iter = global_variables.erase(iter);
		}
		else
			iter++;
	}
}

// For any global variable accessed directly by a function,
// extract that variable and add it as an argument to that function.
void CompilerMSL::extract_global_variables_from_functions()
{

	// Uniforms
	unordered_set<uint32_t> global_var_ids;
	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			if (var.storage == StorageClassInput || var.storage == StorageClassUniform ||
			    var.storage == StorageClassUniformConstant || var.storage == StorageClassPushConstant)
			{
				global_var_ids.insert(var.self);
			}
		}
	}

	// Local vars that are declared in the main function and accessed directy by a function
	auto &entry_func = get<SPIRFunction>(entry_point);
	for (auto &var : entry_func.local_variables)
		global_var_ids.insert(var);

	std::set<uint32_t> added_arg_ids;
	unordered_set<uint32_t> processed_func_ids;
	extract_global_variables_from_function(entry_point, added_arg_ids, global_var_ids, processed_func_ids);
}

// MSL does not support the use of global variables for shader input content.
// For any global variable accessed directly by the specified function, extract that variable,
// add it as an argument to that function, and the arg to the added_arg_ids collection.
void CompilerMSL::extract_global_variables_from_function(uint32_t func_id, std::set<uint32_t> &added_arg_ids,
                                                         unordered_set<uint32_t> &global_var_ids,
                                                         unordered_set<uint32_t> &processed_func_ids)
{
	// Avoid processing a function more than once
	if (processed_func_ids.find(func_id) != processed_func_ids.end())
	{
		// Return function global variables
		added_arg_ids = function_global_vars[func_id];
		return;
	}

	processed_func_ids.insert(func_id);

	auto &func = get<SPIRFunction>(func_id);

	// Recursively establish global args added to functions on which we depend.
	for (auto block : func.blocks)
	{
		auto &b = get<SPIRBlock>(block);
		for (auto &i : b.ops)
		{
			auto ops = stream(i);
			auto op = static_cast<Op>(i.op);

			switch (op)
			{
			case OpLoad:
			case OpAccessChain:
			{
				uint32_t base_id = ops[2];
				if (global_var_ids.find(base_id) != global_var_ids.end())
					added_arg_ids.insert(base_id);

				break;
			}
			case OpFunctionCall:
			{
				uint32_t inner_func_id = ops[2];
				std::set<uint32_t> inner_func_args;
				extract_global_variables_from_function(inner_func_id, inner_func_args, global_var_ids,
				                                       processed_func_ids);
				added_arg_ids.insert(inner_func_args.begin(), inner_func_args.end());
				break;
			}

			default:
				break;
			}
		}
	}

	function_global_vars[func_id] = added_arg_ids;

	// Add the global variables as arguments to the function
	if (func_id != entry_point)
	{
		uint32_t next_id = increase_bound_by(uint32_t(added_arg_ids.size()));
		for (uint32_t arg_id : added_arg_ids)
		{
			uint32_t type_id = get<SPIRVariable>(arg_id).basetype;
			func.add_parameter(type_id, next_id, true);
			set<SPIRVariable>(next_id, type_id, StorageClassFunction);

			// Ensure both the existing and new variables have the same name, and the name is valid
			string vld_name = ensure_valid_name(to_name(arg_id), "v");
			set_name(arg_id, vld_name);
			set_name(next_id, vld_name);

			meta[next_id].decoration.qualified_alias = meta[arg_id].decoration.qualified_alias;
			next_id++;
		}
	}
}

// If a vertex attribute exists at the location, it is marked as being used by this shader
void CompilerMSL::mark_location_as_used_by_shader(uint32_t location, StorageClass storage)
{
	MSLVertexAttr *p_va;
	auto &execution = get_entry_point();
	if ((execution.model == ExecutionModelVertex) && (storage == StorageClassInput) &&
	    (p_va = vtx_attrs_by_location[location]))
		p_va->used_by_shader = true;
}

// Add an interface structure for the type of storage, which is either StorageClassInput or StorageClassOutput.
// Returns the ID of the newly added variable, or zero if no variable was added.
uint32_t CompilerMSL::add_interface_block(StorageClass storage)
{
	// Accumulate the variables that should appear in the interface struct
	vector<SPIRVariable *> vars;
	bool incl_builtins = (storage == StorageClassOutput);
	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			auto &type = get<SPIRType>(var.basetype);
			if (var.storage == storage && interface_variable_exists_in_entry_point(var.self) &&
			    !is_hidden_variable(var, incl_builtins) && type.pointer)
			{
				vars.push_back(&var);
			}
		}
	}

	// If no variables qualify, leave
	if (vars.empty())
		return 0;

	// Add a new typed variable for this interface structure.
	// The initializer expression is allocated here, but populated when the function
	// declaraion is emitted, because it is cleared after each compilation pass.
	uint32_t next_id = increase_bound_by(3);
	uint32_t ib_type_id = next_id++;
	auto &ib_type = set<SPIRType>(ib_type_id);
	ib_type.basetype = SPIRType::Struct;
	ib_type.storage = storage;
	set_decoration(ib_type_id, DecorationBlock);

	uint32_t ib_var_id = next_id++;
	auto &var = set<SPIRVariable>(ib_var_id, ib_type_id, storage, 0);
	var.initializer = next_id++;

	string ib_var_ref;
	switch (storage)
	{
	case StorageClassInput:
		ib_var_ref = stage_in_var_name;
		break;

	case StorageClassOutput:
	{
		ib_var_ref = stage_out_var_name;

		// Add the output interface struct as a local variable to the entry function,
		// and force the entry function to return the output interface struct from
		// any blocks that perform a function return.
		auto &entry_func = get<SPIRFunction>(entry_point);
		entry_func.add_local_variable(ib_var_id);
		for (auto &blk_id : entry_func.blocks)
		{
			auto &blk = get<SPIRBlock>(blk_id);
			if (blk.terminator == SPIRBlock::Return)
				blk.return_value = ib_var_id;
		}
		break;
	}

	case StorageClassUniformConstant:
	{
		ib_var_ref = stage_uniform_var_name;
		break;
	}

	default:
		break;
	}

	set_name(ib_type_id, get_entry_point_name() + "_" + ib_var_ref);
	set_name(ib_var_id, ib_var_ref);

	for (auto p_var : vars)
	{
		uint32_t type_id = p_var->basetype;
		auto &type = get<SPIRType>(type_id);
		if (type.basetype == SPIRType::Struct)
		{
			// Flatten the struct members into the interface struct
			uint32_t mbr_idx = 0;
			for (auto &mbr_type_id : type.member_types)
			{
				auto &mbr_type = get<SPIRType>(mbr_type_id);
				if (is_matrix(mbr_type))
				{
					exclude_member_from_stage_in(type, mbr_idx);
				}
				else
				{
					// Add a reference to the member to the interface struct.
					uint32_t ib_mbr_idx = uint32_t(ib_type.member_types.size());
					ib_type.member_types.push_back(mbr_type_id); // membertype.self is different for array types

					// Give the member a name
					string mbr_name = ensure_valid_name(to_qualified_member_name(type, mbr_idx), "m");
					set_member_name(ib_type_id, ib_mbr_idx, mbr_name);

					// Update the original variable reference to include the structure reference
					string qual_var_name = ib_var_ref + "." + mbr_name;
					set_member_qualified_name(type_id, mbr_idx, qual_var_name);

					// Copy the variable location from the original variable to the member
					if (has_member_decoration(type_id, mbr_idx, DecorationLocation))
					{
						uint32_t locn = get_member_decoration(type_id, mbr_idx, DecorationLocation);
						set_member_decoration(ib_type_id, ib_mbr_idx, DecorationLocation, locn);
						mark_location_as_used_by_shader(locn, storage);
					}

					// Mark the member as builtin if needed
					BuiltIn builtin;
					if (is_member_builtin(type, mbr_idx, &builtin))
					{
						set_member_decoration(ib_type_id, ib_mbr_idx, DecorationBuiltIn, builtin);
						if (builtin == BuiltInPosition)
							qual_pos_var_name = qual_var_name;
					}
				}
				mbr_idx++;
			}
		}
		else if (type.basetype == SPIRType::Boolean || type.basetype == SPIRType::Char ||
		         type.basetype == SPIRType::Int || type.basetype == SPIRType::UInt ||
		         type.basetype == SPIRType::Int64 || type.basetype == SPIRType::UInt64 ||
		         type.basetype == SPIRType::Float || type.basetype == SPIRType::Double ||
		         type.basetype == SPIRType::Boolean)
		{
			if (is_matrix(type))
			{
				exclude_from_stage_in(*p_var);
			}
			else
			{
				// Add a reference to the variable type to the interface struct.
				uint32_t ib_mbr_idx = uint32_t(ib_type.member_types.size());
				ib_type.member_types.push_back(type_id);

				// Give the member a name
				string mbr_name = ensure_valid_name(to_expression(p_var->self), "m");
				set_member_name(ib_type_id, ib_mbr_idx, mbr_name);

				// Update the original variable reference to include the structure reference
				string qual_var_name = ib_var_ref + "." + mbr_name;
				meta[p_var->self].decoration.qualified_alias = qual_var_name;

				// Copy the variable location from the original variable to the member
				if (get_decoration_mask(p_var->self) & (1ull << DecorationLocation))
				{
					uint32_t locn = get_decoration(p_var->self, DecorationLocation);
					set_member_decoration(ib_type_id, ib_mbr_idx, DecorationLocation, locn);
					mark_location_as_used_by_shader(locn, storage);
				}

				// Mark the member as builtin if needed
				if (is_builtin_variable(*p_var))
				{
					uint32_t builtin = get_decoration(p_var->self, DecorationBuiltIn);
					set_member_decoration(ib_type_id, ib_mbr_idx, DecorationBuiltIn, builtin);
					if (builtin == BuiltInPosition)
						qual_pos_var_name = qual_var_name;
				}
			}
		}
	}

	// Sort the members of the interface structure by their attribute numbers.
	// Oddly, Metal handles inputs better if they are sorted in reverse order,
	// particularly if the offsets are all equal.
	MemberSorter::SortAspect sort_aspect =
	    (storage == StorageClassInput) ? MemberSorter::LocationReverse : MemberSorter::Location;
	MemberSorter member_sorter(ib_type, meta[ib_type_id], sort_aspect);
	member_sorter.sort();

	// Sort input or output variables alphabetical
	auto &execution = get_entry_point();
	if ((execution.model == ExecutionModelFragment && storage == StorageClassInput) ||
	    (execution.model == ExecutionModelVertex && storage == StorageClassOutput))
	{
		MemberSorter member_sorter_io(ib_type, meta[ib_type.self], MemberSorter::Alphabetical);
		member_sorter_io.sort();
	}

	return ib_var_id;
}

// Excludes the specified input variable from the stage_in block structure.
// Instead, the variable is added to a block variable corresponding to a secondary MSL buffer.
// The main use case for this is when a stage_in variable contains a matrix, which is a rare occurrence.
void CompilerMSL::exclude_from_stage_in(SPIRVariable &var)
{
	uint32_t var_id = var.self;

	if (!(get_decoration_mask(var_id) & (1ull << DecorationLocation)))
		return;

	uint32_t mbr_type_id = var.basetype;
	string mbr_name = ensure_valid_name(to_expression(var_id), "m");
	uint32_t mbr_locn = get_decoration(var_id, DecorationLocation);
	meta[var_id].decoration.qualified_alias = add_input_buffer_block_member(mbr_type_id, mbr_name, mbr_locn);
}

// Excludes the specified type member from the stage_in block structure.
// Instead, the member is added to a block variable corresponding to a secondary MSL buffer.
// The main use case for this is when a stage_in variable contains a matrix, which is a rare occurrence.
void CompilerMSL::exclude_member_from_stage_in(const SPIRType &type, uint32_t index)
{
	uint32_t type_id = type.self;

	if (!has_member_decoration(type_id, index, DecorationLocation))
		return;

	uint32_t mbr_type_id = type.member_types[index];
	string mbr_name = ensure_valid_name(to_qualified_member_name(type, index), "m");
	uint32_t mbr_locn = get_member_decoration(type_id, index, DecorationLocation);
	string qual_name = add_input_buffer_block_member(mbr_type_id, mbr_name, mbr_locn);
	set_member_qualified_name(type_id, index, qual_name);
}

// Adds a member to the input buffer block that corresponds to the MTLBuffer used by an attribute location
string CompilerMSL::add_input_buffer_block_member(uint32_t mbr_type_id, string mbr_name, uint32_t mbr_locn)
{
	mark_location_as_used_by_shader(mbr_locn, StorageClassInput);

	MSLVertexAttr *p_va = vtx_attrs_by_location[mbr_locn];
	if (!p_va)
		return "";

	if (p_va->per_instance)
		needs_instance_idx_arg = true;
	else
		needs_vertex_idx_arg = true;

	// The variable that is the block struct.
	// Record the stride of this struct in its offset decoration.
	uint32_t ib_var_id = get_input_buffer_block_var_id(p_va->msl_buffer);
	auto &ib_var = get<SPIRVariable>(ib_var_id);
	uint32_t ib_type_id = ib_var.basetype;
	auto &ib_type = get<SPIRType>(ib_type_id);
	set_decoration(ib_type_id, DecorationOffset, p_va->msl_stride);

	// Add a reference to the variable type to the interface struct.
	uint32_t ib_mbr_idx = uint32_t(ib_type.member_types.size());
	ib_type.member_types.push_back(mbr_type_id);

	// Give the member a name
	set_member_name(ib_type_id, ib_mbr_idx, mbr_name);

	// Set MSL buffer and offset decorations, and indicate no valid attribute location
	set_member_decoration(ib_type_id, ib_mbr_idx, DecorationBinding, p_va->msl_buffer);
	set_member_decoration(ib_type_id, ib_mbr_idx, DecorationOffset, p_va->msl_offset);
	set_member_decoration(ib_type_id, ib_mbr_idx, DecorationLocation, k_unknown_location);

	// Update the original variable reference to include the structure and index reference
	string idx_var_name = builtin_to_glsl(p_va->per_instance ? BuiltInInstanceIndex : BuiltInVertexIndex);
	return get_name(ib_var_id) + "[" + idx_var_name + "]." + mbr_name;
}

// Returns the ID of the input block that will use the specified MSL buffer index,
// lazily creating an input block variable and type if needed.
//
// The use of this block applies only to input variables that have been excluded from the stage_in
// block, which typically only occurs if an attempt to pass a matrix in the stage_in block.
uint32_t CompilerMSL::get_input_buffer_block_var_id(uint32_t msl_buffer)
{
	uint32_t ib_var_id = non_stage_in_input_var_ids[msl_buffer];
	if (!ib_var_id)
	{
		// No interface block exists yet. Create a new typed variable for this interface block.
		// The initializer expression is allocated here, but populated when the function
		// declaraion is emitted, because it is cleared after each compilation pass.
		uint32_t next_id = increase_bound_by(3);
		uint32_t ib_type_id = next_id++;
		auto &ib_type = set<SPIRType>(ib_type_id);
		ib_type.basetype = SPIRType::Struct;
		ib_type.storage = StorageClassInput;
		set_decoration(ib_type_id, DecorationBlock);

		ib_var_id = next_id++;
		auto &var = set<SPIRVariable>(ib_var_id, ib_type_id, StorageClassInput, 0);
		var.initializer = next_id++;

		string ib_var_name = stage_in_var_name + convert_to_string(msl_buffer);
		set_name(ib_var_id, ib_var_name);
		set_name(ib_type_id, get_entry_point_name() + "_" + ib_var_name);

		// Add the variable to the map of buffer blocks, accessed by the Metal buffer index.
		non_stage_in_input_var_ids[msl_buffer] = ib_var_id;
	}
	return ib_var_id;
}

// Sort the members of the struct type by offset, and pack and then pad members where needed
// to align MSL members with SPIR-V offsets. The struct members are iterated twice. Packing
// occurs first, followed by padding, because packing a member reduces both its size and its
// natural alignment, possibly requiring a padding member to be added ahead of it.
void CompilerMSL::align_struct(SPIRType &ib_type)
{
	uint32_t &ib_type_id = ib_type.self;

	// Sort the members of the interface structure by their offset.
	// They should already be sorted per SPIR-V spec anyway.
	MemberSorter member_sorter(ib_type, meta[ib_type_id], MemberSorter::Offset);
	member_sorter.sort();

	uint32_t curr_offset = 0;
	uint32_t mbr_cnt = uint32_t(ib_type.member_types.size());

	// Test the alignment of each member, and if a member should be closer to the previous
	// member than the default spacing expects, it is likely that the previous member is in
	// a packed format. If so, and the previous member is packable, pack it.
	// For example...this applies to any 3-element vector that is followed by a scalar.
	for (uint32_t mbr_idx = 0; mbr_idx < mbr_cnt; mbr_idx++)
	{
		// Align current offset to the current member's default alignment.
		size_t align_mask = get_declared_struct_member_alignment(ib_type, mbr_idx) - 1;
		curr_offset = uint32_t((curr_offset + align_mask) & ~align_mask);

		// Fetch the member offset as declared in the SPIRV.
		uint32_t mbr_offset = get_member_decoration(ib_type_id, mbr_idx, DecorationOffset);
		if (curr_offset > mbr_offset)
		{
			uint32_t prev_mbr_idx = mbr_idx - 1;
			if (is_member_packable(ib_type, prev_mbr_idx))
				set_member_decoration(ib_type_id, prev_mbr_idx, DecorationCPacked);
		}

		// Increment the current offset to be positioned immediately after the current member.
		curr_offset = mbr_offset + uint32_t(get_declared_struct_member_size(ib_type, mbr_idx));
	}

	// Test the alignment of each member, and if a member is positioned farther than its
	// alignment and the end of the previous member, add a dummy padding member that will
	// be added before the current member when the delaration of this struct is emitted.
	for (uint32_t mbr_idx = 0; mbr_idx < mbr_cnt; mbr_idx++)
	{
		// Align current offset to the current member's default alignment.
		size_t align_mask = get_declared_struct_member_alignment(ib_type, mbr_idx) - 1;
		curr_offset = uint32_t((curr_offset + align_mask) & ~align_mask);

		// Fetch the member offset as declared in the SPIRV.
		uint32_t mbr_offset = get_member_decoration(ib_type_id, mbr_idx, DecorationOffset);
		if (mbr_offset > curr_offset)
		{
			// Since MSL and SPIR-V have slightly different struct member alignment and
			// size rules, we'll pad to standard C-packing rules. If the member is farther
			// away than C-packing, expects, add an inert padding member before the the member.
			MSLStructMemberKey key = get_struct_member_key(ib_type_id, mbr_idx);
			struct_member_padding[key] = mbr_offset - curr_offset;
		}

		// Increment the current offset to be positioned immediately after the current member.
		curr_offset = mbr_offset + uint32_t(get_declared_struct_member_size(ib_type, mbr_idx));
	}
}

// Returns whether the specified struct member supports a packable type
// variation that is smaller than the unpacked variation of that type.
bool CompilerMSL::is_member_packable(SPIRType &ib_type, uint32_t index)
{
	uint32_t mbr_type_id = ib_type.member_types[index];
	auto &mbr_type = get<SPIRType>(mbr_type_id);

	// 3-element vectors (char3, uchar3, short3, ushort3, int3, uint3, half3, float3)
	if (mbr_type.vecsize == 3 && mbr_type.columns == 1)
		return true;

	return false;
}

// Simple combination of type ID and member index for use as hash key
MSLStructMemberKey CompilerMSL::get_struct_member_key(uint32_t type_id, uint32_t index)
{
	MSLStructMemberKey k = type_id;
	k <<= 32;
	k += index;
	return k;
}

// Converts the format of the current expression from packed to unpacked,
// by wrapping the expression in a constructor of the appropriate type.
string CompilerMSL::unpack_expression_type(string expr_str, const SPIRType &type)
{
	return join(type_to_glsl(type), "(", expr_str, ")");
}

// Emits the file header info
void CompilerMSL::emit_header()
{
	if (!header_lines.empty())
	{
		for (auto &header : header_lines)
			statement(header);

		statement("");
	}

	statement("#include <metal_stdlib>");
	statement("#include <simd/simd.h>");
	statement("");
	statement("using namespace metal;");
	statement("");
}

// Emits any needed custom function bodies.
void CompilerMSL::emit_custom_functions()
{
	for (auto &op : custom_function_ops)
	{
		switch (op)
		{
		case OpFMod:
			statement("// Support GLSL mod(), which is slightly different than Metal fmod()");
			statement("template<typename Tx, typename Ty>");
			statement("Tx mod(Tx x, Ty y)");
			begin_scope();
			statement("return x - y * floor(x / y);");
			end_scope();
			statement("");
			break;

		default:
			break;
		}
	}
}

void CompilerMSL::emit_resources()
{

	// Output all basic struct types which are not Block or BufferBlock as these are declared inplace
	// when such variables are instantiated.
	for (auto &id : ids)
	{
		if (id.get_type() == TypeType)
		{
			auto &type = id.get<SPIRType>();
			if (type.basetype == SPIRType::Struct && type.array.empty() && !type.pointer &&
			    !has_decoration(type.self, DecorationBlock) && !has_decoration(type.self, DecorationBufferBlock))
			{
				emit_struct(type);
			}
		}
	}

	// Output Uniform buffers and constants
	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			auto &type = get<SPIRType>(var.basetype);

			if (var.storage != StorageClassFunction && type.pointer &&
			    (type.storage == StorageClassUniform || type.storage == StorageClassUniformConstant ||
			     type.storage == StorageClassPushConstant) &&
			    (has_decoration(type.self, DecorationBlock) || has_decoration(type.self, DecorationBufferBlock)) &&
			    !is_hidden_variable(var))
			{
				if (options.pad_and_pack_uniform_structs)
					align_struct(type);

				emit_struct(type);
			}
		}
	}

	// Output interface blocks.
	emit_interface_block(stage_in_var_id);
	for (auto &nsi_var : non_stage_in_input_var_ids)
		emit_interface_block(nsi_var.second);

	emit_interface_block(stage_out_var_id);
	emit_interface_block(stage_uniforms_var_id);
}

// Override for MSL-specific syntax instructions
void CompilerMSL::emit_instruction(const Instruction &instruction)
{

#define BOP(op) emit_binary_op(ops[0], ops[1], ops[2], ops[3], #op)
#define BOP_CAST(op, type) \
	emit_binary_op_cast(ops[0], ops[1], ops[2], ops[3], #op, type, opcode_is_sign_invariant(opcode))
#define UOP(op) emit_unary_op(ops[0], ops[1], ops[2], #op)
#define QFOP(op) emit_quaternary_func_op(ops[0], ops[1], ops[2], ops[3], ops[4], ops[5], #op)
#define TFOP(op) emit_trinary_func_op(ops[0], ops[1], ops[2], ops[3], ops[4], #op)
#define BFOP(op) emit_binary_func_op(ops[0], ops[1], ops[2], ops[3], #op)
#define BFOP_CAST(op, type) \
	emit_binary_func_op_cast(ops[0], ops[1], ops[2], ops[3], #op, type, opcode_is_sign_invariant(opcode))
#define BFOP(op) emit_binary_func_op(ops[0], ops[1], ops[2], ops[3], #op)
#define UFOP(op) emit_unary_func_op(ops[0], ops[1], ops[2], #op)

	auto ops = stream(instruction);
	auto opcode = static_cast<Op>(instruction.op);

	switch (opcode)
	{

	// Comparisons
	case OpIEqual:
	case OpLogicalEqual:
	case OpFOrdEqual:
		BOP(==);
		break;

	case OpINotEqual:
	case OpLogicalNotEqual:
	case OpFOrdNotEqual:
		BOP(!=);
		break;

	case OpUGreaterThan:
	case OpSGreaterThan:
	case OpFOrdGreaterThan:
		BOP(>);
		break;

	case OpUGreaterThanEqual:
	case OpSGreaterThanEqual:
	case OpFOrdGreaterThanEqual:
		BOP(>=);
		break;

	case OpULessThan:
	case OpSLessThan:
	case OpFOrdLessThan:
		BOP(<);
		break;

	case OpULessThanEqual:
	case OpSLessThanEqual:
	case OpFOrdLessThanEqual:
		BOP(<=);
		break;

	// Derivatives
	case OpDPdx:
		UFOP(dfdx);
		break;

	case OpDPdy:
		UFOP(dfdy);
		break;

	case OpImageQuerySize:
	{
		auto &type = expression_type(ops[2]);
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];

		if (type.basetype == SPIRType::Image)
		{
			string img_exp = to_expression(ops[2]);
			auto &img_type = type.image;
			switch (img_type.dim)
			{
			case Dim1D:
				if (img_type.arrayed)
					emit_op(result_type, id, join("uint2(", img_exp, ".get_width(), ", img_exp, ".get_array_size())"),
					        false);
				else
					emit_op(result_type, id, join(img_exp, ".get_width()"), true);
				break;

			case Dim2D:
			case DimCube:
				if (img_type.arrayed)
					emit_op(result_type, id, join("uint3(", img_exp, ".get_width(), ", img_exp, ".get_height(), ",
					                              img_exp, ".get_array_size())"),
					        false);
				else
					emit_op(result_type, id, join("uint2(", img_exp, ".get_width(), ", img_exp, ".get_height())"),
					        false);
				break;

			case Dim3D:
				emit_op(result_type, id,
				        join("uint3(", img_exp, ".get_width(), ", img_exp, ".get_height(), ", img_exp, ".get_depth())"),
				        false);
				break;

			default:
				break;
			}
		}
		else
			SPIRV_CROSS_THROW("Invalid type for OpImageQuerySize.");
		break;
	}

	default:
		CompilerGLSL::emit_instruction(instruction);
		break;
	}
}

// Override for MSL-specific extension syntax instructions
void CompilerMSL::emit_glsl_op(uint32_t result_type, uint32_t id, uint32_t eop, const uint32_t *args, uint32_t count)
{
	GLSLstd450 op = static_cast<GLSLstd450>(eop);

	switch (op)
	{
	case GLSLstd450Atan2:
		emit_binary_func_op(result_type, id, args[0], args[1], "atan2");
		break;

	default:
		CompilerGLSL::emit_glsl_op(result_type, id, eop, args, count);
		break;
	}
}

// Emit a structure declaration for the specified interface variable.
void CompilerMSL::emit_interface_block(uint32_t ib_var_id)
{
	if (ib_var_id)
	{
		auto &ib_var = get<SPIRVariable>(ib_var_id);
		auto &ib_type = get<SPIRType>(ib_var.basetype);
		auto &m = meta.at(ib_type.self);
		if (m.members.size() > 0)
			emit_struct(ib_type);
	}
}

// Emits the declaration signature of the specified function.
// If this is the entry point function, Metal-specific return value and function arguments are added.
void CompilerMSL::emit_function_prototype(SPIRFunction &func, uint64_t)
{
	local_variable_names = resource_names;
	string decl;

	processing_entry_point = (func.self == entry_point);

	auto &type = get<SPIRType>(func.return_type);
	decl += func_type_decl(type);
	decl += " ";
	decl += clean_func_name(to_name(func.self));

	decl += "(";

	if (processing_entry_point)
	{
		decl += entry_point_args(!func.arguments.empty());

		// If entry point function has a output interface struct, set its initializer.
		// This is done at this late stage because the initialization expression is
		// cleared after each compilation pass.
		if (stage_out_var_id)
		{
			auto &so_var = get<SPIRVariable>(stage_out_var_id);
			auto &so_type = get<SPIRType>(so_var.basetype);
			set<SPIRExpression>(so_var.initializer, "{}", so_type.self, true);
		}
	}

	for (auto &arg : func.arguments)
	{
		add_local_variable_name(arg.id);

		string address_space = "thread";

		auto *var = maybe_get<SPIRVariable>(arg.id);
		if (var)
		{
			var->parameter = &arg; // Hold a pointer to the parameter so we can invalidate the readonly field if needed.
			address_space = get_argument_address_space(*var);
		}

		decl += address_space + " ";
		decl += argument_decl(arg);

		// Manufacture automatic sampler arg for SampledImage texture
		auto &arg_type = get<SPIRType>(arg.type);
		if (arg_type.basetype == SPIRType::SampledImage)
			decl += ", thread const sampler& " + to_sampler_expression(arg.id);

		if (&arg != &func.arguments.back())
			decl += ", ";
	}

	decl += ")";
	statement(decl);
}

// Returns the texture sampling function string for the specified image and sampling characteristics.
string CompilerMSL::to_function_name(uint32_t img, const SPIRType &, bool is_fetch, bool is_gather, bool, bool, bool,
                                     bool, bool has_dref, uint32_t)
{
	// Texture reference
	string fname = to_expression(img) + ".";

	// Texture function and sampler
	if (is_fetch)
		fname += "read";
	else if (is_gather)
		fname += "gather";
	else
		fname += "sample";

	if (has_dref)
		fname += "_compare";

	return fname;
}

// Returns the function args for a texture sampling function for the specified image and sampling characteristics.
string CompilerMSL::to_function_args(uint32_t img, const SPIRType &imgtype, bool is_fetch, bool, bool is_proj,
                                     uint32_t coord, uint32_t, uint32_t dref, uint32_t grad_x, uint32_t grad_y,
                                     uint32_t lod, uint32_t coffset, uint32_t offset, uint32_t bias, uint32_t comp,
                                     uint32_t sample, bool *p_forward)
{
	string farg_str;
	if (!is_fetch)
		farg_str += to_sampler_expression(img);

	// Texture coordinates
	bool forward = should_forward(coord);
	auto coord_expr = to_enclosed_expression(coord);
	auto &coord_type = expression_type(coord);

	string tex_coords = coord_expr;
	string face_expr;
	const char *alt_coord = "";

	switch (imgtype.image.dim)
	{

	case Dim1D:
		if (coord_type.vecsize > 1)
		{
			tex_coords += ".x";
			alt_coord = ".y";
		}

		if (is_fetch)
			tex_coords = "uint(" + tex_coords + ")";

		break;

	case DimBuffer:
		if (coord_type.vecsize > 1)
		{
			tex_coords += ".x";
			alt_coord = ".y";
		}

		if (is_fetch)
			tex_coords = "uint2(" + tex_coords + ", 0)"; // Metal textures are 2D

		break;

	case Dim2D:
		if (options.flip_frag_y)
		{
			string coord_x = coord_expr + ".x";
			string coord_y = coord_expr + ".y";
			if (is_fetch)
				tex_coords = "uint2(" + coord_x + ", (1.0 - " + coord_y + "))";
			else
				tex_coords = "float2(" + coord_x + ", (1.0 - " + coord_y + "))";
		}
		else
		{
			if (coord_type.vecsize > 2)
				tex_coords += ".xy";

			if (is_fetch)
				tex_coords = "uint2(" + tex_coords + ")";
		}

		alt_coord = ".z";

		break;

	case Dim3D:
		if (options.flip_frag_y)
		{
			string coord_x = coord_expr + ".x";
			string coord_y = coord_expr + ".y";
			string coord_z = coord_expr + ".z";
			if (is_fetch)
				tex_coords = "uint3(" + coord_x + ", (1.0 - " + coord_y + "), " + coord_z + ")";
			else
				tex_coords = "float3(" + coord_x + ", (1.0 - " + coord_y + "), " + coord_z + ")";
		}
		else
		{
			if (coord_type.vecsize > 3)
				tex_coords += ".xyz";

			if (is_fetch)
				tex_coords = "uint3(" + tex_coords + ")";
		}

		alt_coord = ".w";

		break;

	case DimCube:
		if (options.flip_frag_y)
		{
			string coord_x = coord_expr + ".x";
			string coord_y = coord_expr + ".y";
			string coord_z = coord_expr + ".z";

			if (is_fetch)
			{
				tex_coords = "uint2(" + coord_x + ", (1.0 - " + coord_y + "))";
				face_expr = coord_z;
			}
			else
				tex_coords = "float3(" + coord_x + ", (1.0 - " + coord_y + "), " + coord_z + ")";
		}
		else
		{
			if (is_fetch)
			{
				tex_coords = "uint2(" + tex_coords + ".xy)";
				face_expr = coord_expr + ".z";
			}
			else
			{
				if (coord_type.vecsize > 3)
					tex_coords += ".xyz";
			}
		}

		alt_coord = ".w";

		break;

	default:
		break;
	}

	// If projection, use alt coord as divisor
	if (is_proj)
		tex_coords += " / " + coord_expr + alt_coord;

	if (!farg_str.empty())
		farg_str += ", ";
	farg_str += tex_coords;

	// If fetch from cube, add face explicitly
	if (!face_expr.empty())
		farg_str += ", uint(" + face_expr + ")";

	// If array, use alt coord
	if (imgtype.image.arrayed)
		farg_str += ", uint(" + coord_expr + alt_coord + ")";

	// Depth compare reference value
	if (dref)
	{
		forward = forward && should_forward(dref);
		farg_str += ", ";
		farg_str += to_expression(dref);
	}

	// LOD Options
	if (bias)
	{
		forward = forward && should_forward(bias);
		farg_str += ", bias(" + to_expression(bias) + ")";
	}

	if (lod)
	{
		forward = forward && should_forward(lod);
		if (is_fetch)
		{
			farg_str += ", " + to_expression(lod);
		}
		else
		{
			farg_str += ", level(" + to_expression(lod) + ")";
		}
	}

	if (grad_x || grad_y)
	{
		forward = forward && should_forward(grad_x);
		forward = forward && should_forward(grad_y);
		string grad_opt;
		switch (imgtype.image.dim)
		{
		case Dim2D:
			grad_opt = "2d";
			break;
		case Dim3D:
			grad_opt = "3d";
			break;
		case DimCube:
			grad_opt = "cube";
			break;
		default:
			grad_opt = "unsupported_gradient_dimension";
			break;
		}
		farg_str += ", gradient" + grad_opt + "(" + to_expression(grad_x) + ", " + to_expression(grad_y) + ")";
	}

	// Add offsets
	string offset_expr;
	if (coffset)
	{
		forward = forward && should_forward(coffset);
		offset_expr = to_expression(coffset);
	}
	else if (offset)
	{
		forward = forward && should_forward(offset);
		offset_expr = to_expression(offset);
	}

	if (!offset_expr.empty())
	{
		switch (imgtype.image.dim)
		{
		case Dim2D:
			if (options.flip_frag_y)
			{
				string coord_x = offset_expr + ".x";
				string coord_y = offset_expr + ".y";
				offset_expr = "float2(" + coord_x + ", (1.0 - " + coord_y + "))";
			}
			else
			{
				if (coord_type.vecsize > 2)
					offset_expr += ".xy";
			}

			farg_str += ", " + offset_expr;
			break;

		case Dim3D:
			if (options.flip_frag_y)
			{
				string coord_x = offset_expr + ".x";
				string coord_y = offset_expr + ".y";
				string coord_z = offset_expr + ".z";
				offset_expr = "float3(" + coord_x + ", (1.0 - " + coord_y + "), " + coord_z + ")";
			}
			else
			{
				if (coord_type.vecsize > 3)
					offset_expr += ".xyz";
			}

			farg_str += ", " + offset_expr;
			break;

		default:
			break;
		}
	}

	if (comp)
	{
		forward = forward && should_forward(comp);
		farg_str += ", " + to_component_argument(comp);
	}

	if (sample)
	{
		farg_str += ", ";
		farg_str += to_expression(sample);
	}

	*p_forward = forward;

	return farg_str;
}

// Returns a string to use in an image sampling function argument.
// The ID must be a scalar constant.
string CompilerMSL::to_component_argument(uint32_t id)
{
	if (ids[id].get_type() != TypeConstant)
	{
		SPIRV_CROSS_THROW("ID " + to_string(id) + " is not an OpConstant.");
		return "component::x";
	}

	uint32_t component_index = get<SPIRConstant>(id).scalar();
	switch (component_index)
	{
	case 0:
		return "component::x";
	case 1:
		return "component::y";
	case 2:
		return "component::z";
	case 3:
		return "component::w";

	default:
		SPIRV_CROSS_THROW("The value (" + to_string(component_index) + ") of OpConstant ID " + to_string(id) +
		                  " is not a valid Component index, which must be one of 0, 1, 2, or 3.");
		return "component::x";
	}
}

// Establish sampled image as expression object and assign the sampler to it.
void CompilerMSL::emit_sampled_image_op(uint32_t result_type, uint32_t result_id, uint32_t image_id, uint32_t samp_id)
{
	set<SPIRExpression>(result_id, to_expression(image_id), result_type, true);
	meta[result_id].sampler = samp_id;
}

// Returns a string representation of the ID, usable as a function arg.
// Manufacture automatic sampler arg for SampledImage texture.
string CompilerMSL::to_func_call_arg(uint32_t id)
{
	string arg_str = CompilerGLSL::to_func_call_arg(id);

	// Manufacture automatic sampler arg if the arg is a SampledImage texture.
	Variant &id_v = ids[id];
	if (id_v.get_type() == TypeVariable)
	{
		auto &var = id_v.get<SPIRVariable>();
		auto &type = get<SPIRType>(var.basetype);
		if (type.basetype == SPIRType::SampledImage)
			arg_str += ", " + to_sampler_expression(id);
	}

	return arg_str;
}

// If the ID represents a sampled image that has been assigned a sampler already,
// generate an expression for the sampler, otherwise generate a fake sampler name
// by appending a suffix to the expression constructed from the ID.
string CompilerMSL::to_sampler_expression(uint32_t id)
{
	uint32_t samp_id = meta[id].sampler;
	return samp_id ? to_expression(samp_id) : to_expression(id) + sampler_name_suffix;
}

// Called automatically at the end of the entry point function
void CompilerMSL::emit_fixup()
{
	auto &execution = get_entry_point();

	if ((execution.model == ExecutionModelVertex) && stage_out_var_id && !qual_pos_var_name.empty())
	{
		if (CompilerGLSL::options.vertex.fixup_clipspace)
		{
			statement(qual_pos_var_name, ".z = (", qual_pos_var_name, ".z + ", qual_pos_var_name,
			          ".w) * 0.5;       // Adjust clip-space for Metal");
		}

		if (options.flip_vert_y)
			statement(qual_pos_var_name, ".y = -(", qual_pos_var_name, ".y);", "    // Invert Y-axis for Metal");
	}
}

// Emit a structure member, padding and packing to maintain the correct memeber alignments.
void CompilerMSL::emit_struct_member(const SPIRType &type, uint32_t member_type_id, uint32_t index,
                                     const string &qualifier)
{
	auto &membertype = get<SPIRType>(member_type_id);

	// If this member requires padding to maintain alignment, emit a dummy padding member.
	MSLStructMemberKey key = get_struct_member_key(type.self, index);
	uint32_t pad_len = struct_member_padding[key];
	if (pad_len > 0)
		statement("char pad", to_string(index), "[", to_string(pad_len), "];");

	// If this member is packed, mark it as so.
	string pack_pfx = member_is_packed_type(type, index) ? "packed_" : "";

	statement(pack_pfx, type_to_glsl(membertype), " ", qualifier, to_member_name(type, index),
	          type_to_array_glsl(membertype), member_attribute_qualifier(type, index), ";");
}

// Return a MSL qualifier for the specified function attribute member
string CompilerMSL::member_attribute_qualifier(const SPIRType &type, uint32_t index)
{
	auto &execution = get_entry_point();

	BuiltIn builtin;
	bool is_builtin = is_member_builtin(type, index, &builtin);

	// Vertex function inputs
	if (execution.model == ExecutionModelVertex && type.storage == StorageClassInput)
	{
		if (is_builtin)
		{
			switch (builtin)
			{
			case BuiltInVertexId:
			case BuiltInVertexIndex:
			case BuiltInInstanceId:
			case BuiltInInstanceIndex:
				return string(" [[") + builtin_qualifier(builtin) + "]]";

			default:
				return "";
			}
		}
		uint32_t locn = get_ordered_member_location(type.self, index);
		if (locn != k_unknown_location)
			return string(" [[attribute(") + convert_to_string(locn) + ")]]";
	}

	// Vertex function outputs
	if (execution.model == ExecutionModelVertex && type.storage == StorageClassOutput)
	{
		if (is_builtin)
		{
			switch (builtin)
			{
			case BuiltInClipDistance:
				return " /* [[clip_distance]] built-in not yet supported under Metal. */";

			case BuiltInPointSize: // Must output only if really rendering points
				return options.is_rendering_points ? (string(" [[") + builtin_qualifier(builtin) + "]]") : "";

			case BuiltInPosition:
			case BuiltInLayer:
				return string(" [[") + builtin_qualifier(builtin) + "]]";

			default:
				return "";
			}
		}
		uint32_t locn = get_ordered_member_location(type.self, index);
		if (locn != k_unknown_location)
			return string(" [[user(locn") + convert_to_string(locn) + ")]]";
	}

	// Fragment function inputs
	if (execution.model == ExecutionModelFragment && type.storage == StorageClassInput)
	{
		if (is_builtin)
		{
			switch (builtin)
			{
			case BuiltInFrontFacing:
			case BuiltInPointCoord:
			case BuiltInFragCoord:
			case BuiltInSampleId:
			case BuiltInSampleMask:
			case BuiltInLayer:
				return string(" [[") + builtin_qualifier(builtin) + "]]";

			default:
				return "";
			}
		}
		uint32_t locn = get_ordered_member_location(type.self, index);
		if (locn != k_unknown_location)
			return string(" [[user(locn") + convert_to_string(locn) + ")]]";
	}

	// Fragment function outputs
	if (execution.model == ExecutionModelFragment && type.storage == StorageClassOutput)
	{
		if (is_builtin)
		{
			switch (builtin)
			{
			case BuiltInSampleMask:
			case BuiltInFragDepth:
				return string(" [[") + builtin_qualifier(builtin) + "]]";

			default:
				return "";
			}
		}
		uint32_t locn = get_ordered_member_location(type.self, index);
		if (locn != k_unknown_location)
			return string(" [[color(") + convert_to_string(locn) + ")]]";
	}

	// Compute function inputs
	if (execution.model == ExecutionModelGLCompute && type.storage == StorageClassInput)
	{
		if (is_builtin)
		{
			switch (builtin)
			{
			case BuiltInGlobalInvocationId:
			case BuiltInLocalInvocationId:
			case BuiltInLocalInvocationIndex:
				return string(" [[") + builtin_qualifier(builtin) + "]]";

			default:
				return "";
			}
		}
	}

	return "";
}

// Returns the location decoration of the member with the specified index in the specified type.
// If the location of the member has been explicitly set, that location is used. If not, this
// function assumes the members are ordered in their location order, and simply returns the
// index as the location.
uint32_t CompilerMSL::get_ordered_member_location(uint32_t type_id, uint32_t index)
{
	auto &m = meta.at(type_id);
	if (index < m.members.size())
	{
		auto &dec = m.members[index];
		if (dec.decoration_flags & (1ull << DecorationLocation))
			return dec.location;
	}

	return index;
}

string CompilerMSL::constant_expression(const SPIRConstant &c)
{
	if (!c.subconstants.empty())
	{
		// Handles Arrays and structures.
		string res = "{";
		for (auto &elem : c.subconstants)
		{
			res += constant_expression(get<SPIRConstant>(elem));
			if (&elem != &c.subconstants.back())
				res += ", ";
		}
		res += "}";
		return res;
	}
	else if (c.columns() == 1)
	{
		return constant_expression_vector(c, 0);
	}
	else
	{
		string res = type_to_glsl(get<SPIRType>(c.constant_type)) + "(";
		for (uint32_t col = 0; col < c.columns(); col++)
		{
			res += constant_expression_vector(c, col);
			if (col + 1 < c.columns())
				res += ", ";
		}
		res += ")";
		return res;
	}
}

// Returns the type declaration for a function, including the
// entry type if the current function is the entry point function
string CompilerMSL::func_type_decl(SPIRType &type)
{
	auto &execution = get_entry_point();
	// The regular function return type. If not processing the entry point function, that's all we need
	string return_type = type_to_glsl(type);
	if (!processing_entry_point)
		return return_type;

	// If an outgoing interface block has been defined, override the entry point return type
	if (stage_out_var_id)
	{
		auto &so_var = get<SPIRVariable>(stage_out_var_id);
		auto &so_type = get<SPIRType>(so_var.basetype);
		return_type = type_to_glsl(so_type);
	}

	// Prepend a entry type, based on the execution model
	string entry_type;
	switch (execution.model)
	{
	case ExecutionModelVertex:
		entry_type = "vertex";
		break;
	case ExecutionModelFragment:
		entry_type = (execution.flags & (1ull << ExecutionModeEarlyFragmentTests)) ?
		                 "fragment [[ early_fragment_tests ]]" :
		                 "fragment";
		break;
	case ExecutionModelGLCompute:
	case ExecutionModelKernel:
		entry_type = "kernel";
		break;
	default:
		entry_type = "unknown";
		break;
	}

	return entry_type + " " + return_type;
}

// Ensures the function name is not "main", which is illegal in MSL
string CompilerMSL::clean_func_name(string func_name)
{
	auto iter = func_name_overrides.find(func_name);
	return (iter != func_name_overrides.end()) ? iter->second : func_name;
}

// In MSL address space qualifiers are required for all pointer or reference arguments
string CompilerMSL::get_argument_address_space(const SPIRVariable &argument)
{
	const auto &type = get<SPIRType>(argument.basetype);

	if ((type.basetype == SPIRType::Struct) &&
	    (type.storage == StorageClassUniform || type.storage == StorageClassUniformConstant ||
	     type.storage == StorageClassPushConstant))
	{
		if ((meta[type.self].decoration.decoration_flags & (1ull << DecorationBufferBlock)) != 0 &&
		    (meta[argument.self].decoration.decoration_flags & (1ull << DecorationNonWritable)) == 0)
		{
			return "device";
		}
		else
		{
			return "constant";
		}
	}

	return "thread";
}

// Returns a string containing a comma-delimited list of args for the entry point function
string CompilerMSL::entry_point_args(bool append_comma)
{
	string ep_args;

	// Stage-in structure
	if (stage_in_var_id)
	{
		auto &var = get<SPIRVariable>(stage_in_var_id);
		auto &type = get<SPIRType>(var.basetype);

		if (!ep_args.empty())
			ep_args += ", ";

		ep_args += type_to_glsl(type) + " " + to_name(var.self) + " [[stage_in]]";
	}

	// Non-stage-in vertex attribute structures
	for (auto &nsi_var : non_stage_in_input_var_ids)
	{
		auto &var = get<SPIRVariable>(nsi_var.second);
		auto &type = get<SPIRType>(var.basetype);

		if (!ep_args.empty())
			ep_args += ", ";

		ep_args += "device " + type_to_glsl(type) + "* " + to_name(var.self) + " [[buffer(" +
		           convert_to_string(nsi_var.first) + ")]]";
	}

	// Uniforms
	for (auto &id : ids)
	{
		if (id.get_type() == TypeVariable)
		{
			auto &var = id.get<SPIRVariable>();
			auto &type = get<SPIRType>(var.basetype);

			if ((var.storage == StorageClassUniform || var.storage == StorageClassUniformConstant ||
			     var.storage == StorageClassPushConstant) &&
			    !is_hidden_variable(var))
			{
				switch (type.basetype)
				{
				case SPIRType::Struct:
				{
					auto &m = meta.at(type.self);
					if (m.members.size() == 0)
						break;
					if (!ep_args.empty())
						ep_args += ", ";
					ep_args += get_argument_address_space(var) + " " + type_to_glsl(type) + "& " + to_name(var.self);
					ep_args += " [[buffer(" + convert_to_string(get_metal_resource_index(var, type.basetype)) + ")]]";
					break;
				}
				case SPIRType::Sampler:
					if (!ep_args.empty())
						ep_args += ", ";
					ep_args += type_to_glsl(type) + " " + to_name(var.self);
					ep_args += " [[sampler(" + convert_to_string(get_metal_resource_index(var, type.basetype)) + ")]]";
					break;
				case SPIRType::Image:
					if (!ep_args.empty())
						ep_args += ", ";
					ep_args += type_to_glsl(type) + " " + to_name(var.self);
					ep_args += " [[texture(" + convert_to_string(get_metal_resource_index(var, type.basetype)) + ")]]";
					break;
				case SPIRType::SampledImage:
					if (!ep_args.empty())
						ep_args += ", ";
					ep_args += type_to_glsl(type) + " " + to_name(var.self);
					ep_args +=
					    " [[texture(" + convert_to_string(get_metal_resource_index(var, SPIRType::Image)) + ")]]";
					if (type.image.dim != DimBuffer)
					{
						ep_args += ", sampler " + to_sampler_expression(var.self);
						ep_args +=
						    " [[sampler(" + convert_to_string(get_metal_resource_index(var, SPIRType::Sampler)) + ")]]";
					}
					break;
				default:
					break;
				}
			}
			if (var.storage == StorageClassInput && is_builtin_variable(var))
			{
				if (!ep_args.empty())
					ep_args += ", ";
				BuiltIn bi_type = meta[var.self].decoration.builtin_type;
				ep_args += builtin_type_decl(bi_type) + " " + to_expression(var.self);
				ep_args += " [[" + builtin_qualifier(bi_type) + "]]";
			}
		}
	}

	// Vertex and instance index built-ins
	if (needs_vertex_idx_arg)
		ep_args += built_in_func_arg(BuiltInVertexIndex, !ep_args.empty());

	if (needs_instance_idx_arg)
		ep_args += built_in_func_arg(BuiltInInstanceIndex, !ep_args.empty());

	if (!ep_args.empty() && append_comma)
		ep_args += ", ";

	return ep_args;
}

// Returns the Metal index of the resource of the specified type as used by the specified variable.
uint32_t CompilerMSL::get_metal_resource_index(SPIRVariable &var, SPIRType::BaseType basetype)
{
	auto &execution = get_entry_point();
	auto &var_dec = meta[var.self].decoration;
	uint32_t var_desc_set = (var.storage == StorageClassPushConstant) ? kPushConstDescSet : var_dec.set;
	uint32_t var_binding = (var.storage == StorageClassPushConstant) ? kPushConstBinding : var_dec.binding;

	// If a matching binding has been specified, find and use it
	for (auto p_res_bind : resource_bindings)
	{
		if (p_res_bind->stage == execution.model && p_res_bind->desc_set == var_desc_set &&
		    p_res_bind->binding == var_binding)
		{

			p_res_bind->used_by_shader = true;
			switch (basetype)
			{
			case SPIRType::Struct:
				return p_res_bind->msl_buffer;
			case SPIRType::Image:
				return p_res_bind->msl_texture;
			case SPIRType::Sampler:
				return p_res_bind->msl_sampler;
			default:
				return 0;
			}
		}
	}

	// If a binding has not been specified, revert to incrementing resource indices
	switch (basetype)
	{
	case SPIRType::Struct:
		return next_metal_resource_index.msl_buffer++;
	case SPIRType::Image:
		return next_metal_resource_index.msl_texture++;
	case SPIRType::Sampler:
		return next_metal_resource_index.msl_sampler++;
	default:
		return 0;
	}
}

// Returns the name of the entry point of this shader
string CompilerMSL::get_entry_point_name()
{
	return clean_func_name(to_name(entry_point));
}

string CompilerMSL::argument_decl(const SPIRFunction::Parameter &arg)
{
	auto &type = expression_type(arg.id);
	bool constref = !arg.alias_global_variable && (!type.pointer || arg.write_count == 0);

	// TODO: Check if this arg is an uniform pointer
	bool pointer = type.storage == StorageClassUniformConstant;

	auto &var = get<SPIRVariable>(arg.id);
	return join(constref ? "const " : "", type_to_glsl(type), pointer ? " " : "& ", to_name(var.self),
	            type_to_array_glsl(type));
}

// If we're currently in the entry point function, and the object
// has a qualified name, use it, otherwise use the standard name.
string CompilerMSL::to_name(uint32_t id, bool allow_alias) const
{
	if (current_function && (current_function->self == entry_point))
	{
		string qual_name = meta.at(id).decoration.qualified_alias;
		if (!qual_name.empty())
			return qual_name;
	}
	return Compiler::to_name(id, allow_alias);
}

// Returns a name that combines the name of the struct with the name of the member, except for Builtins
string CompilerMSL::to_qualified_member_name(const SPIRType &type, uint32_t index)
{
	// Don't qualify Builtin names because they are unique and are treated as such when building expressions
	BuiltIn builtin;
	if (is_member_builtin(type, index, &builtin))
		return builtin_to_glsl(builtin);

	// Strip any underscore prefix from member name
	string mbr_name = to_member_name(type, index);
	size_t startPos = mbr_name.find_first_not_of("_");
	mbr_name = (startPos != string::npos) ? mbr_name.substr(startPos) : "";
	return join(to_name(type.self), "_", mbr_name);
}

// Ensures that the specified name is permanently usable by prepending a prefix
// if the first chars are _ and a digit, which indicate a transient name.
string CompilerMSL::ensure_valid_name(string name, string pfx)
{
	if (name.size() >= 2 && name[0] == '_' && isdigit(name[1]))
	{
		return join(pfx, name);
	}
	else
	{
		auto iter = var_name_overrides.find(name);
		return (iter != var_name_overrides.end()) ? iter->second : name;
	}
}

// Returns an MSL string describing  the SPIR-V type
string CompilerMSL::type_to_glsl(const SPIRType &type)
{
	// Ignore the pointer type since GLSL doesn't have pointers.

	switch (type.basetype)
	{
	case SPIRType::Struct:
		// Need OpName lookup here to get a "sensible" name for a struct.
		return to_name(type.self);

	case SPIRType::Image:
	case SPIRType::SampledImage:
		return image_type_glsl(type);

	case SPIRType::Sampler:
		return "sampler";

	case SPIRType::Void:
		return "void";

	default:
		break;
	}

	if (is_scalar(type)) // Scalar builtin
	{
		switch (type.basetype)
		{
		case SPIRType::Boolean:
			return "bool";
		case SPIRType::Char:
			return "char";
		case SPIRType::Int:
			return (type.width == 16 ? "short" : "int");
		case SPIRType::UInt:
			return (type.width == 16 ? "ushort" : "uint");
		case SPIRType::AtomicCounter:
			return "atomic_uint";
		case SPIRType::Float:
			return (type.width == 16 ? "half" : "float");
		default:
			return "unknown_type";
		}
	}
	else if (is_vector(type)) // Vector builtin
	{
		switch (type.basetype)
		{
		case SPIRType::Boolean:
			return join("bool", type.vecsize);
		case SPIRType::Char:
			return join("char", type.vecsize);
		case SPIRType::Int:
			return join((type.width == 16 ? "short" : "int"), type.vecsize);
		case SPIRType::UInt:
			return join((type.width == 16 ? "ushort" : "uint"), type.vecsize);
		case SPIRType::Float:
			return join((type.width == 16 ? "half" : "float"), type.vecsize);
		default:
			return "unknown_type";
		}
	}
	else
	{
		switch (type.basetype)
		{
		case SPIRType::Boolean:
		case SPIRType::Int:
		case SPIRType::UInt:
		case SPIRType::Float:
			return join((type.width == 16 ? "half" : "float"), type.columns, "x", type.vecsize);
		default:
			return "unknown_type";
		}
	}
}

// Returns an MSL string describing  the SPIR-V image type
string CompilerMSL::image_type_glsl(const SPIRType &type)
{
	string img_type_name;

	auto &img_type = type.image;
	if (img_type.depth)
	{
		switch (img_type.dim)
		{
		case spv::Dim2D:
			img_type_name += (img_type.ms ? "depth2d_ms" : (img_type.arrayed ? "depth2d_array" : "depth2d"));
			break;
		case spv::DimCube:
			img_type_name += (img_type.arrayed ? "depthcube_array" : "depthcube");
			break;
		default:
			img_type_name += "unknown_depth_texture_type";
			break;
		}
	}
	else
	{
		switch (img_type.dim)
		{
		case spv::Dim1D:
			img_type_name += (img_type.arrayed ? "texture1d_array" : "texture1d");
			break;
		case spv::DimBuffer:
		case spv::Dim2D:
			img_type_name += (img_type.ms ? "texture2d_ms" : (img_type.arrayed ? "texture2d_array" : "texture2d"));
			break;
		case spv::Dim3D:
			img_type_name += "texture3d";
			break;
		case spv::DimCube:
			img_type_name += (img_type.arrayed ? "texturecube_array" : "texturecube");
			break;
		default:
			img_type_name += "unknown_texture_type";
			break;
		}
	}

	// Append the pixel type
	auto &img_pix_type = get<SPIRType>(img_type.type);
	img_type_name += "<" + type_to_glsl(img_pix_type) + ">";

	return img_type_name;
}

// Returns an MSL string identifying the name of a SPIR-V builtin.
// Output builtins are qualified with the name of the stage out structure.
string CompilerMSL::builtin_to_glsl(BuiltIn builtin)
{
	switch (builtin)
	{

	// Override GLSL compiler strictness
	case BuiltInVertexId:
		return "gl_VertexID";
	case BuiltInInstanceId:
		return "gl_InstanceID";
	case BuiltInVertexIndex:
		return "gl_VertexIndex";
	case BuiltInInstanceIndex:
		return "gl_InstanceIndex";

	// Output builtins qualified with output struct when used in the entry function
	case BuiltInPosition:
	case BuiltInPointSize:
	case BuiltInClipDistance:
	case BuiltInLayer:
		if (current_function && (current_function->self == entry_point))
			return stage_out_var_name + "." + CompilerGLSL::builtin_to_glsl(builtin);
		else
			return CompilerGLSL::builtin_to_glsl(builtin);

	default:
		return CompilerGLSL::builtin_to_glsl(builtin);
	}
}

// Returns an MSL string attribute qualifer for a SPIR-V builtin
string CompilerMSL::builtin_qualifier(BuiltIn builtin)
{
	auto &execution = get_entry_point();

	switch (builtin)
	{
	// Vertex function in
	case BuiltInVertexId:
		return "vertex_id";
	case BuiltInVertexIndex:
		return "vertex_id";
	case BuiltInInstanceId:
		return "instance_id";
	case BuiltInInstanceIndex:
		return "instance_id";

	// Vertex function out
	case BuiltInClipDistance:
		return "clip_distance";
	case BuiltInPointSize:
		return "point_size";
	case BuiltInPosition:
		return "position";
	case BuiltInLayer:
		return "render_target_array_index";

	// Fragment function in
	case BuiltInFrontFacing:
		return "front_facing";
	case BuiltInPointCoord:
		return "point_coord";
	case BuiltInFragCoord:
		return "position";
	case BuiltInSampleId:
		return "sample_id";
	case BuiltInSampleMask:
		return "sample_mask";

	// Fragment function out
	case BuiltInFragDepth:
	{
		if (execution.flags & (1ull << ExecutionModeDepthGreater))
			return "depth(greater)";

		if (execution.flags & (1ull << ExecutionModeDepthLess))
			return "depth(less)";

		if (execution.flags & (1ull << ExecutionModeDepthUnchanged))
			return "depth(any)";
	}

	// Compute function in
	case BuiltInGlobalInvocationId:
		return "thread_position_in_grid";

	case BuiltInLocalInvocationId:
		return "thread_position_in_threadgroup";

	case BuiltInLocalInvocationIndex:
		return "thread_index_in_threadgroup";

	default:
		return "unsupported-built-in";
	}
}

// Returns an MSL string type declaration for a SPIR-V builtin
string CompilerMSL::builtin_type_decl(BuiltIn builtin)
{
	switch (builtin)
	{
	// Vertex function in
	case BuiltInVertexId:
		return "uint";
	case BuiltInVertexIndex:
		return "uint";
	case BuiltInInstanceId:
		return "uint";
	case BuiltInInstanceIndex:
		return "uint";

	// Vertex function out
	case BuiltInClipDistance:
		return "float";
	case BuiltInPointSize:
		return "float";
	case BuiltInPosition:
		return "float4";

	// Fragment function in
	case BuiltInFrontFacing:
		return "bool";
	case BuiltInPointCoord:
		return "float2";
	case BuiltInFragCoord:
		return "float4";
	case BuiltInSampleId:
		return "uint";
	case BuiltInSampleMask:
		return "uint";

	// Compute function in
	case BuiltInGlobalInvocationId:
		return "uint3";
	case BuiltInLocalInvocationId:
		return "uint3";
	case BuiltInLocalInvocationIndex:
		return "uint";

	default:
		return "unsupported-built-in-type";
	}
}

// Returns the declaration of a built-in argument to a function
string CompilerMSL::built_in_func_arg(BuiltIn builtin, bool prefix_comma)
{
	string bi_arg;
	if (prefix_comma)
		bi_arg += ", ";
	bi_arg += builtin_type_decl(builtin);
	bi_arg += " " + builtin_to_glsl(builtin);
	bi_arg += " [[" + builtin_qualifier(builtin) + "]]";
	return bi_arg;
}

// Returns the byte size of a struct member.
size_t CompilerMSL::get_declared_struct_member_size(const SPIRType &struct_type, uint32_t index) const
{
	uint32_t type_id = struct_type.member_types[index];
	auto dec_mask = get_member_decoration_mask(struct_type.self, index);
	return get_declared_type_size(type_id, dec_mask);
}

// Returns the effective size of a variable type.
size_t CompilerMSL::get_declared_type_size(uint32_t type_id) const
{
	return get_declared_type_size(type_id, get_decoration_mask(type_id));
}

// Returns the effective size in bytes of a variable type
// or member type, taking into consideration the decorations mask.
size_t CompilerMSL::get_declared_type_size(uint32_t type_id, uint64_t dec_mask) const
{
	auto &type = get<SPIRType>(type_id);

	switch (type.basetype)
	{
	case SPIRType::Unknown:
	case SPIRType::Void:
	case SPIRType::AtomicCounter:
	case SPIRType::Image:
	case SPIRType::SampledImage:
	case SPIRType::Sampler:
		SPIRV_CROSS_THROW("Querying size of opaque object.");

	case SPIRType::Struct:
		return get_declared_struct_size(type);

	default:
	{
		size_t component_size = type.width / 8;
		unsigned vecsize = type.vecsize;
		unsigned columns = type.columns;

		if (!type.array.empty())
		{
			// For arrays, we can use ArrayStride to get an easy check if it has been populated.
			// ArrayStride is part of the array type not OpMemberDecorate.
			auto &dec = meta[type_id].decoration;
			if (dec.decoration_flags & (1ull << DecorationArrayStride))
				return dec.array_stride * to_array_size_literal(type, uint32_t(type.array.size()) - 1);
		}

		if (columns == 1) // An unpacked 3-element vector is the same size as a 4-element vector.
		{
			if (!(dec_mask & (1ull << DecorationCPacked)))
			{
				if (vecsize == 3)
					vecsize = 4;
			}
		}
		else // For matrices, a 3-element column is the same size as a 4-element column.
		{
			if (dec_mask & (1ull << DecorationColMajor))
			{
				if (vecsize == 3)
					vecsize = 4;
			}
			else if (dec_mask & (1ull << DecorationRowMajor))
			{
				if (columns == 3)
					columns = 4;
			}
		}

		return vecsize * columns * component_size;
	}
	}
}

// Returns the byte alignment of a struct member.
size_t CompilerMSL::get_declared_struct_member_alignment(const SPIRType &struct_type, uint32_t index) const
{
	uint32_t type_id = struct_type.member_types[index];
	auto dec_mask = get_member_decoration_mask(struct_type.self, index);
	return get_declared_type_alignment(type_id, dec_mask);
}

// Returns the effective alignment in bytes of a variable type
// or member type, taking into consideration the decorations mask.
size_t CompilerMSL::get_declared_type_alignment(uint32_t type_id, uint64_t dec_mask) const
{
	auto &type = get<SPIRType>(type_id);

	switch (type.basetype)
	{
	case SPIRType::Unknown:
	case SPIRType::Void:
	case SPIRType::AtomicCounter:
	case SPIRType::Image:
	case SPIRType::SampledImage:
	case SPIRType::Sampler:
		SPIRV_CROSS_THROW("Querying alignment of opaque object.");

	case SPIRType::Struct:
		return 16; // Per Vulkan spec section 14.5.4

	default:
	{
		// Alignment of packed type is the same as the underlying component size.
		// Alignment of unpacked type is the same as the type size (or one matrix column).
		if (dec_mask & (1ull << DecorationCPacked))
			return type.width / 8;
		else
			return get_declared_type_size(type_id, dec_mask) / type.columns;
	}
	}
}

bool CompilerMSL::OpCodePreprocessor::handle(Op opcode, const uint32_t * /*args*/, uint32_t /*length*/)
{
	switch (opcode)
	{
	// If an opcode requires a bespoke custom function be output, remember it.
	case OpFMod:
		compiler.custom_function_ops.insert(uint32_t(opcode));
		break;

	// Since MSL exists in a single execution scope, function prototype declarations are not
	// needed, and clutter the output. If secondary functions are output (as indicated by the
	// presence of OpFunctionCall, then suppress compiler warnings of missing function prototypes.
	case OpFunctionCall:
		suppress_missing_prototypes = true;
		break;

	default:
		break;
	}
	return true;
}

// Sort both type and meta member content based on builtin status (put builtins at end),
// then by the required sorting aspect.
void CompilerMSL::MemberSorter::sort()
{
	// Create a temporary array of consecutive member indices and sort it based on how
	// the members should be reordered, based on builtin and sorting aspect meta info.
	size_t mbr_cnt = type.member_types.size();
	vector<uint32_t> mbr_idxs(mbr_cnt);
	iota(mbr_idxs.begin(), mbr_idxs.end(), 0); // Fill with consecutive indices
	std::sort(mbr_idxs.begin(), mbr_idxs.end(), *this); // Sort member indices based on sorting aspect

	// Move type and meta member info to the order defined by the sorted member indices.
	// This is done by creating temporary copies of both member types and meta, and then
	// copying back to the original content at the sorted indices.
	auto mbr_types_cpy = type.member_types;
	auto mbr_meta_cpy = meta.members;
	for (uint32_t mbr_idx = 0; mbr_idx < mbr_cnt; mbr_idx++)
	{
		type.member_types[mbr_idx] = mbr_types_cpy[mbr_idxs[mbr_idx]];
		meta.members[mbr_idx] = mbr_meta_cpy[mbr_idxs[mbr_idx]];
	}
}

// Sort first by builtin status (put builtins at end), then by the sorting aspect.
bool CompilerMSL::MemberSorter::operator()(uint32_t mbr_idx1, uint32_t mbr_idx2)
{
	auto &mbr_meta1 = meta.members[mbr_idx1];
	auto &mbr_meta2 = meta.members[mbr_idx2];
	if (mbr_meta1.builtin != mbr_meta2.builtin)
		return mbr_meta2.builtin;
	else
		switch (sort_aspect)
		{
		case Location:
			return mbr_meta1.location < mbr_meta2.location;
		case LocationReverse:
			return mbr_meta1.location > mbr_meta2.location;
		case Offset:
			return mbr_meta1.offset < mbr_meta2.offset;
		case OffsetThenLocationReverse:
			return (mbr_meta1.offset < mbr_meta2.offset) ||
			       ((mbr_meta1.offset == mbr_meta2.offset) && (mbr_meta1.location > mbr_meta2.location));
		case Alphabetical:
			return mbr_meta1.alias > mbr_meta2.alias;
		default:
			return false;
		}
}

CompilerMSL::MemberSorter::MemberSorter(SPIRType &t, Meta &m, SortAspect sa)
    : type(t)
    , meta(m)
    , sort_aspect(sa)
{
	// Ensure enough meta info is available
	meta.members.resize(max(type.member_types.size(), meta.members.size()));
}
