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

CompilerMSL::CompilerMSL(const uint32_t *ir, size_t word_count, MSLVertexAttr *p_vtx_attrs, size_t vtx_attrs_count,
                         MSLResourceBinding *p_res_bindings, size_t res_bindings_count)
    : CompilerGLSL(ir, word_count)
{
	populate_func_name_overrides();
	populate_var_name_overrides();

	if (p_vtx_attrs)
		for (size_t i = 0; i < vtx_attrs_count; i++)
			vtx_attrs_by_location[p_vtx_attrs[i].location] = &p_vtx_attrs[i];

	if (p_res_bindings)
		for (size_t i = 0; i < res_bindings_count; i++)
			resource_bindings.push_back(&p_res_bindings[i]);
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
	fixup_image_load_store_access();

    set_enabled_interface_variables(get_active_interface_variables());

	// Preprocess OpCodes to extract the need to output additional header content
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
	CompilerGLSL::options.vulkan_semantics = true;
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
		emit_specialization_constants();
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
	spv_function_implementations.clear();

	OpCodePreprocessor preproc(*this);
	traverse_all_reachable_opcodes(get<SPIRFunction>(entry_point), preproc);

	if (preproc.suppress_missing_prototypes)
		add_pragma_line("#pragma clang diagnostic ignored \"-Wmissing-prototypes\"");

	if (preproc.uses_atomics)
	{
		add_header_line("#include <metal_atomic>");
		add_pragma_line("#pragma clang diagnostic ignored \"-Wunused-variable\"");
	}
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
        active_interface_variables.insert(ib_var_id);   // Ensure will be emitted
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
				BuiltIn builtin;
				bool is_builtin = is_member_builtin(type, mbr_idx, &builtin);

				auto &mbr_type = get<SPIRType>(mbr_type_id);
				if (is_matrix(mbr_type))
				{
					exclude_member_from_stage_in(type, mbr_idx);
				}
				else if (!is_builtin || has_active_builtin(builtin, storage))
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
					else if (has_decoration(p_var->self, DecorationLocation))
					{
						// The block itself might have a location and in this case, all members of the block
						// receive incrementing locations.
						uint32_t locn = get_decoration(p_var->self, DecorationLocation) + mbr_idx;
						set_member_decoration(ib_type_id, ib_mbr_idx, DecorationLocation, locn);
						mark_location_as_used_by_shader(locn, storage);
					}

                    // Migrate whether this member should use half precision
                    if (has_member_decoration(type_id, mbr_idx, DecorationRelaxedPrecision))
                        set_member_decoration(ib_type_id, ib_mbr_idx, DecorationRelaxedPrecision);

					// Mark the member as builtin if needed
					if (is_builtin)
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
			bool is_builtin = is_builtin_variable(*p_var);
			BuiltIn builtin = BuiltIn(get_decoration(p_var->self, DecorationBuiltIn));

			if (is_matrix(type))
				exclude_from_stage_in(*p_var);

			else if (!is_builtin || has_active_builtin(builtin, storage))
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

                // Migrate whether this member should use half precision
                if (has_decoration(p_var->self, DecorationRelaxedPrecision))
                    set_member_decoration(ib_type_id, ib_mbr_idx, DecorationRelaxedPrecision);

				// Mark the member as builtin if needed
				if (is_builtin)
				{
					set_member_decoration(ib_type_id, ib_mbr_idx, DecorationBuiltIn, builtin);
					if (builtin == BuiltInPosition)
						qual_pos_var_name = qual_var_name;
				}
			}
		}
	}

	// Sort the members of the structure by their locations.
	// Oddly, Metal handles inputs better if they are sorted in reverse order,
	// particularly if the offsets are all equal.
	MemberSorter::SortAspect sort_aspect =
	    (storage == StorageClassInput) ? MemberSorter::LocationReverse : MemberSorter::Location;
	MemberSorter member_sorter(ib_type, meta[ib_type_id], sort_aspect);
	member_sorter.sort();

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

// Returns a combination of type ID and member index for use as hash key
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
	for (auto &header : pragma_lines)
		statement(header);

	if (!pragma_lines.empty())
		statement("");

	statement("#include <metal_stdlib>");
	statement("#include <simd/simd.h>");

	for (auto &header : header_lines)
		statement(header);

	statement("");
	statement("using namespace metal;");
	statement("");
}

void CompilerMSL::add_pragma_line(const string &line)
{
	pragma_lines.push_back(line);
}

// Emits any needed custom function bodies.
void CompilerMSL::emit_custom_functions()
{
	for (auto &spv_func : spv_function_implementations)
	{
		switch (spv_func)
		{
		case SPVFuncImplMod:
			statement("// Implementation of the GLSL mod() function, which is slightly different than Metal fmod()");
			statement("template<typename Tx, typename Ty>");
			statement("Tx mod(Tx x, Ty y)");
			begin_scope();
			statement("return x - y * floor(x / y);");
			end_scope();
			statement("");
			break;

		case SPVFuncImplRadians:
			statement("// Implementation of the GLSL radians() function");
			statement("template<typename T>");
			statement("T radians(T d)");
			begin_scope();
			statement("return d * 0.01745329251;");
			end_scope();
			statement("");
			break;

		case SPVFuncImplDegrees:
			statement("// Implementation of the GLSL degrees() function");
			statement("template<typename T>");
			statement("T degrees(T r)");
			begin_scope();
			statement("return r * 57.2957795131;");
			end_scope();
			statement("");
			break;

		case SPVFuncImplFindILsb:
			statement("// Implementation of the GLSL findLSB() function");
			statement("template<typename T>");
			statement("T findLSB(T x)");
			begin_scope();
			statement("return select(ctz(x), -1, x == 0);");
			end_scope();
			statement("");
			break;

		case SPVFuncImplFindUMsb:
			statement("// Implementation of the unsigned GLSL findMSB() function");
			statement("template<typename T>");
			statement("T findUMSB(T x)");
			begin_scope();
			statement("return select(clz(0) - (clz(x) + 1), -1, x == 0);");
			end_scope();
			statement("");
			break;

		case SPVFuncImplFindSMsb:
			statement("// Implementation of the signed GLSL findMSB() function");
			statement("template<typename T>");
			statement("T findSMSB(T x)");
			begin_scope();
			statement("T v = select(x, -1 - x, x < 0);");
			statement("return select(clz(0) - (clz(v) + 1), -1, v == 0);");
			end_scope();
			statement("");
			break;

		case SPVFuncImplInverse4x4:
			statement("// Returns the determinant of a 2x2 matrix.");
			statement("inline float spvDet2x2(float a1, float a2, float b1, float b2)");
			begin_scope();
			statement("return a1 * b2 - b1 * a2;");
			end_scope();
			statement("");
			statement("// Returns the determinant of a 3x3 matrix.");
			statement("inline float spvDet3x3(float a1, float a2, float a3, float b1, float b2, float b3, float c1, "
			          "float c2, float c3)");
			begin_scope();
			statement("return a1 * spvDet2x2(b2, b3, c2, c3) - b1 * spvDet2x2(a2, a3, c2, c3) + c1 * spvDet2x2(a2, a3, "
			          "b2, b3);");
			end_scope();
			statement("");
			statement("// Returns the inverse of a matrix, by using the algorithm of calculating the classical");
			statement("// adjoint and dividing by the determinant. The contents of the matrix are changed.");
			statement("float4x4 spvInverse4x4(float4x4 m)");
			begin_scope();
			statement("float4x4 adj;	// The adjoint matrix (inverse after dividing by determinant)");
			statement("");
			statement("// Create the transpose of the cofactors, as the classical adjoint of the matrix.");
			statement("adj[0][0] =  spvDet3x3(m[1][1], m[1][2], m[1][3], m[2][1], m[2][2], m[2][3], m[3][1], m[3][2], "
			          "m[3][3]);");
			statement("adj[0][1] = -spvDet3x3(m[0][1], m[0][2], m[0][3], m[2][1], m[2][2], m[2][3], m[3][1], m[3][2], "
			          "m[3][3]);");
			statement("adj[0][2] =  spvDet3x3(m[0][1], m[0][2], m[0][3], m[1][1], m[1][2], m[1][3], m[3][1], m[3][2], "
			          "m[3][3]);");
			statement("adj[0][3] = -spvDet3x3(m[0][1], m[0][2], m[0][3], m[1][1], m[1][2], m[1][3], m[2][1], m[2][2], "
			          "m[2][3]);");
			statement("");
			statement("adj[1][0] = -spvDet3x3(m[1][0], m[1][2], m[1][3], m[2][0], m[2][2], m[2][3], m[3][0], m[3][2], "
			          "m[3][3]);");
			statement("adj[1][1] =  spvDet3x3(m[0][0], m[0][2], m[0][3], m[2][0], m[2][2], m[2][3], m[3][0], m[3][2], "
			          "m[3][3]);");
			statement("adj[1][2] = -spvDet3x3(m[0][0], m[0][2], m[0][3], m[1][0], m[1][2], m[1][3], m[3][0], m[3][2], "
			          "m[3][3]);");
			statement("adj[1][3] =  spvDet3x3(m[0][0], m[0][2], m[0][3], m[1][0], m[1][2], m[1][3], m[2][0], m[2][2], "
			          "m[2][3]);");
			statement("");
			statement("adj[2][0] =  spvDet3x3(m[1][0], m[1][1], m[1][3], m[2][0], m[2][1], m[2][3], m[3][0], m[3][1], "
			          "m[3][3]);");
			statement("adj[2][1] = -spvDet3x3(m[0][0], m[0][1], m[0][3], m[2][0], m[2][1], m[2][3], m[3][0], m[3][1], "
			          "m[3][3]);");
			statement("adj[2][2] =  spvDet3x3(m[0][0], m[0][1], m[0][3], m[1][0], m[1][1], m[1][3], m[3][0], m[3][1], "
			          "m[3][3]);");
			statement("adj[2][3] = -spvDet3x3(m[0][0], m[0][1], m[0][3], m[1][0], m[1][1], m[1][3], m[2][0], m[2][1], "
			          "m[2][3]);");
			statement("");
			statement("adj[3][0] = -spvDet3x3(m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2], m[3][0], m[3][1], "
			          "m[3][2]);");
			statement("adj[3][1] =  spvDet3x3(m[0][0], m[0][1], m[0][2], m[2][0], m[2][1], m[2][2], m[3][0], m[3][1], "
			          "m[3][2]);");
			statement("adj[3][2] = -spvDet3x3(m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[3][0], m[3][1], "
			          "m[3][2]);");
			statement("adj[3][3] =  spvDet3x3(m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], "
			          "m[2][2]);");
			statement("");
			statement("// Calculate the determinant as a combination of the cofactors of the first row.");
			statement("float det = (adj[0][0] * m[0][0]) + (adj[0][1] * m[1][0]) + (adj[0][2] * m[2][0]) + (adj[0][3] "
			          "* m[3][0]);");
			statement("");
			statement("// Divide the classical adjoint matrix by the determinant.");
			statement("// If determinant is zero, matrix is not invertable, so leave it unchanged.");
			statement("return (det != 0.0f) ? (adj * (1.0f / det)) : m;");
			end_scope();
			statement("");
			break;

		case SPVFuncImplInverse3x3:
			statement("// Returns the determinant of a 2x2 matrix.");
			statement("inline float spvDet2x2(float a1, float a2, float b1, float b2)");
			begin_scope();
			statement("return a1 * b2 - b1 * a2;");
			end_scope();
			statement("");
			statement("// Returns the inverse of a matrix, by using the algorithm of calculating the classical");
			statement("// adjoint and dividing by the determinant. The contents of the matrix are changed.");
			statement("float3x3 spvInverse3x3(float3x3 m)");
			begin_scope();
			statement("float3x3 adj;	// The adjoint matrix (inverse after dividing by determinant)");
			statement("");
			statement("// Create the transpose of the cofactors, as the classical adjoint of the matrix.");
			statement("adj[0][0] =  spvDet2x2(m[1][1], m[1][2], m[2][1], m[2][2]);");
			statement("adj[0][1] = -spvDet2x2(m[0][1], m[0][2], m[2][1], m[2][2]);");
			statement("adj[0][2] =  spvDet2x2(m[0][1], m[0][2], m[1][1], m[1][2]);");
			statement("");
			statement("adj[1][0] = -spvDet2x2(m[1][0], m[1][2], m[2][0], m[2][2]);");
			statement("adj[1][1] =  spvDet2x2(m[0][0], m[0][2], m[2][0], m[2][2]);");
			statement("adj[1][2] = -spvDet2x2(m[0][0], m[0][2], m[1][0], m[1][2]);");
			statement("");
			statement("adj[2][0] =  spvDet2x2(m[1][0], m[1][1], m[2][0], m[2][1]);");
			statement("adj[2][1] = -spvDet2x2(m[0][0], m[0][1], m[2][0], m[2][1]);");
			statement("adj[2][2] =  spvDet2x2(m[0][0], m[0][1], m[1][0], m[1][1]);");
			statement("");
			statement("// Calculate the determinant as a combination of the cofactors of the first row.");
			statement("float det = (adj[0][0] * m[0][0]) + (adj[0][1] * m[1][0]) + (adj[0][2] * m[2][0]);");
			statement("");
			statement("// Divide the classical adjoint matrix by the determinant.");
			statement("// If determinant is zero, matrix is not invertable, so leave it unchanged.");
			statement("return (det != 0.0f) ? (adj * (1.0f / det)) : m;");
			end_scope();
			statement("");
			break;

		case SPVFuncImplInverse2x2:
			statement("// Returns the inverse of a matrix, by using the algorithm of calculating the classical");
			statement("// adjoint and dividing by the determinant. The contents of the matrix are changed.");
			statement("float2x2 spvInverse2x2(float2x2 m)");
			begin_scope();
			statement("float2x2 adj;	// The adjoint matrix (inverse after dividing by determinant)");
			statement("");
			statement("// Create the transpose of the cofactors, as the classical adjoint of the matrix.");
			statement("adj[0][0] =  m[1][1];");
			statement("adj[0][1] = -m[0][1];");
			statement("");
			statement("adj[1][0] = -m[1][0];");
			statement("adj[1][1] =  m[0][0];");
			statement("");
			statement("// Calculate the determinant as a combination of the cofactors of the first row.");
			statement("float det = (adj[0][0] * m[0][0]) + (adj[0][1] * m[1][0]);");
			statement("");
			statement("// Divide the classical adjoint matrix by the determinant.");
			statement("// If determinant is zero, matrix is not invertable, so leave it unchanged.");
			statement("return (det != 0.0f) ? (adj * (1.0f / det)) : m;");
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

// Emit declarations for the specialization Metal function constants
void CompilerMSL::emit_specialization_constants()
{
	const vector<SpecializationConstant> spec_consts = get_specialization_constants();

	if (spec_consts.empty())
		return;

	for (auto &sc : spec_consts)
	{
		string sc_type_name = type_to_glsl(expression_type(sc.id));
		string sc_name = to_name(sc.id);
		string sc_tmp_name = to_name(sc.id) + "_tmp";

		statement("constant ", sc_type_name, " ", sc_tmp_name, " [[function_constant(",
		          convert_to_string(sc.constant_id), ")]];");
		statement("constant ", sc_type_name, " ", sc_name, " = is_function_constant_defined(", sc_tmp_name, ") ? ",
		          sc_tmp_name, " : ", constant_expression(get<SPIRConstant>(sc.id)), ";");
	}
	statement("");
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
	case OpDPdxFine:
	case OpDPdxCoarse:
		UFOP(dfdx);
		break;

	case OpDPdy:
	case OpDPdyFine:
	case OpDPdyCoarse:
		UFOP(dfdy);
		break;

	// Bitfield
	case OpBitFieldInsert:
		QFOP(insert_bits);
		break;

	case OpBitFieldSExtract:
	case OpBitFieldUExtract:
		TFOP(extract_bits);
		break;

	case OpBitReverse:
		UFOP(reverse_bits);
		break;

	case OpBitCount:
		UFOP(popcount);
		break;

	// Atomics
	case OpAtomicExchange:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t ptr = ops[2];
		uint32_t mem_sem = ops[4];
		uint32_t val = ops[5];
		emit_atomic_func_op(result_type, id, "atomic_exchange_explicit", mem_sem, mem_sem, false, ptr, val);
		break;
	}

	case OpAtomicCompareExchange:
	case OpAtomicCompareExchangeWeak:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t ptr = ops[2];
		uint32_t mem_sem_pass = ops[4];
		uint32_t mem_sem_fail = ops[5];
		uint32_t val = ops[6];
		uint32_t comp = ops[7];
		emit_atomic_func_op(result_type, id, "atomic_compare_exchange_weak_explicit", mem_sem_pass, mem_sem_fail, true,
		                    ptr, comp, true, val);
		break;
	}

	case OpAtomicLoad:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t ptr = ops[2];
		uint32_t mem_sem = ops[4];
		emit_atomic_func_op(result_type, id, "atomic_load_explicit", mem_sem, mem_sem, false, ptr, 0);
		break;
	}

	case OpAtomicStore:
	{
		uint32_t result_type = expression_type(ops[0]).self;
		uint32_t id = ops[0];
		uint32_t ptr = ops[0];
		uint32_t mem_sem = ops[2];
		uint32_t val = ops[3];
		emit_atomic_func_op(result_type, id, "atomic_store_explicit", mem_sem, mem_sem, false, ptr, val);
		break;
	}

#define AFMOImpl(op, valsrc)                                                                                      \
	do                                                                                                            \
	{                                                                                                             \
		uint32_t result_type = ops[0];                                                                            \
		uint32_t id = ops[1];                                                                                     \
		uint32_t ptr = ops[2];                                                                                    \
		uint32_t mem_sem = ops[4];                                                                                \
		uint32_t val = valsrc;                                                                                    \
		emit_atomic_func_op(result_type, id, "atomic_fetch_" #op "_explicit", mem_sem, mem_sem, false, ptr, val); \
	} while (false)

#define AFMO(op) AFMOImpl(op, ops[5])
#define AFMIO(op) AFMOImpl(op, 1)

	case OpAtomicIIncrement:
		AFMIO(add);
		break;

	case OpAtomicIDecrement:
		AFMIO(sub);
		break;

	case OpAtomicIAdd:
		AFMO(add);
		break;

	case OpAtomicISub:
		AFMO(sub);
		break;

	case OpAtomicSMin:
	case OpAtomicUMin:
		AFMO(min);
		break;

	case OpAtomicSMax:
	case OpAtomicUMax:
		AFMO(max);
		break;

	case OpAtomicAnd:
		AFMO(and);
		break;

	case OpAtomicOr:
		AFMO(or);
		break;

	case OpAtomicXor:
		AFMO (xor);
		break;

	// Images

	// Reads == Fetches in Metal
	case OpImageRead:
	{
		// Mark that this shader reads from this image
		uint32_t img_id = ops[2];
		auto *p_var = maybe_get_backing_variable(img_id);
		if (p_var)
			unset_decoration(p_var->self, DecorationNonReadable);

		emit_texture_op(instruction);
		break;
	}

	case OpImageWrite:
	{
		uint32_t img_id = ops[0];
		uint32_t coord_id = ops[1];
		uint32_t texel_id = ops[2];
		const uint32_t *opt = &ops[3];
		uint32_t length = instruction.length - 4;

		// Bypass pointers because we need the real image struct
		auto &type = expression_type(img_id);
		auto &img_type = get<SPIRType>(type.self);

		// Ensure this image has been marked as being written to and force a
		// recommpile so that the image type output will include write access
		auto *p_var = maybe_get_backing_variable(img_id);
		if (p_var && has_decoration(p_var->self, DecorationNonWritable))
		{
			unset_decoration(p_var->self, DecorationNonWritable);
			force_recompile = true;
		}

		bool forward = false;
		uint32_t bias = 0;
		uint32_t lod = 0;
		uint32_t flags = 0;

		if (length)
		{
			flags = *opt++;
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

		statement(join(
		    to_expression(img_id), ".write(", to_expression(texel_id), ", ",
		    to_function_args(img_id, img_type, true, false, false, coord_id, 0, 0, 0, 0, lod, 0, 0, 0, 0, 0, &forward),
		    ");"));

		if (p_var && variable_storage_is_aliased(*p_var))
			flush_all_aliased_variables();

		break;
	}

	case OpImageQuerySize:
	case OpImageQuerySizeLod:
	{
		uint32_t rslt_type_id = ops[0];
		auto &rslt_type = get<SPIRType>(rslt_type_id);

		uint32_t id = ops[1];

		uint32_t img_id = ops[2];
		string img_exp = to_expression(img_id);
		auto &img_type = expression_type(img_id);
		Dim img_dim = img_type.image.dim;
		bool is_array = img_type.image.arrayed;

		if (img_type.basetype != SPIRType::Image)
			SPIRV_CROSS_THROW("Invalid type for OpImageQuerySize.");

		string lod;
		if (opcode == OpImageQuerySizeLod)
		{
			// LOD index defaults to zero, so don't bother outputing level zero index
			string decl_lod = to_expression(ops[3]);
			if (decl_lod != "0")
				lod = decl_lod;
		}

		string expr = type_to_glsl(rslt_type) + "(";
		expr += img_exp + ".get_width(" + lod + ")";

		if (img_dim == Dim2D || img_dim == DimCube || img_dim == Dim3D)
			expr += ", " + img_exp + ".get_height(" + lod + ")";

		if (img_dim == Dim3D)
			expr += ", " + img_exp + ".get_depth(" + lod + ")";

		if (is_array)
			expr += ", " + img_exp + ".get_array_size()";

		expr += ")";

		emit_op(rslt_type_id, id, expr, should_forward(img_id));

		break;
	}

#define ImgQry(qrytype)                                                                     \
	do                                                                                      \
	{                                                                                       \
		uint32_t rslt_type_id = ops[0];                                                     \
		auto &rslt_type = get<SPIRType>(rslt_type_id);                                      \
		uint32_t id = ops[1];                                                               \
		uint32_t img_id = ops[2];                                                           \
		string img_exp = to_expression(img_id);                                             \
		string expr = type_to_glsl(rslt_type) + "(" + img_exp + ".get_num_" #qrytype "())"; \
		emit_op(rslt_type_id, id, expr, should_forward(img_id));                            \
	} while (false)

	case OpImageQueryLevels:
		ImgQry(mip_levels);
		break;

	case OpImageQuerySamples:
		ImgQry(samples);
		break;

	// Casting
	case OpQuantizeToF16:
	{
		uint32_t result_type = ops[0];
		uint32_t id = ops[1];
		uint32_t arg = ops[2];

		string exp;
		auto &type = get<SPIRType>(result_type);

		switch (type.vecsize)
		{
		case 1:
			exp = join("float(half(", to_expression(arg), "))");
			break;
		case 2:
			exp = join("float2(half2(", to_expression(arg), "))");
			break;
		case 3:
			exp = join("float3(half3(", to_expression(arg), "))");
			break;
		case 4:
			exp = join("float4(half4(", to_expression(arg), "))");
			break;
		default:
			SPIRV_CROSS_THROW("Illegal argument to OpQuantizeToF16.");
		}

		emit_op(result_type, id, exp, should_forward(arg));
		break;
	}

	// OpOuterProduct

	default:
		CompilerGLSL::emit_instruction(instruction);
		break;
	}
}

// Emits one of the atomic functions. In MSL, the atomic functions operate on pointers
void CompilerMSL::emit_atomic_func_op(uint32_t result_type, uint32_t result_id, const char *op, uint32_t mem_order_1,
                                      uint32_t mem_order_2, bool has_mem_order_2, uint32_t obj, uint32_t op1,
                                      bool op1_is_pointer, uint32_t op2)
{
	forced_temporaries.insert(result_id);

	bool fwd_obj = should_forward(obj);
	bool fwd_op1 = op1 ? should_forward(op1) : true;
	bool fwd_op2 = op2 ? should_forward(op2) : true;

	bool forward = fwd_obj && fwd_op1 && fwd_op2;

	string exp = string(op) + "(";

	auto &type = expression_type(obj);
	exp += "(volatile ";
	exp += "device";
	exp += " atomic_";
	exp += type_to_glsl(type);
	exp += "*)";

	exp += "&(";
	exp += to_expression(obj);
	exp += ")";

	if (op1)
	{
		if (op1_is_pointer)
		{
			statement(declare_temporary(expression_type(op2).self, op1), to_expression(op1), ";");
			exp += ", &(" + to_name(op1) + ")";
		}
		else
			exp += ", " + to_expression(op1);
	}

	if (op2)
		exp += ", " + to_expression(op2);

	exp += string(", ") + get_memory_order(mem_order_1);

	if (has_mem_order_2)
		exp += string(", ") + get_memory_order(mem_order_2);

	exp += ")";
	emit_op(result_type, result_id, exp, forward);

	inherit_expression_dependencies(result_id, obj);
	if (op1)
		inherit_expression_dependencies(result_id, op1);
	if (op2)
		inherit_expression_dependencies(result_id, op2);

	flush_all_atomic_capable_variables();
}

// Metal only supports relaxed memory order for now
const char *CompilerMSL::get_memory_order(uint32_t)
{
	return "memory_order_relaxed";
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
	case GLSLstd450InverseSqrt:
		emit_unary_func_op(result_type, id, args[0], "rsqrt");
		break;
	case GLSLstd450RoundEven:
		emit_unary_func_op(result_type, id, args[0], "rint");
		break;

	case GLSLstd450FindSMsb:
		emit_unary_func_op(result_type, id, args[0], "findSMSB");
		break;
	case GLSLstd450FindUMsb:
		emit_unary_func_op(result_type, id, args[0], "findUMSB");
		break;

	case GLSLstd450PackSnorm4x8:
		emit_unary_func_op(result_type, id, args[0], "pack_float_to_snorm4x8");
		break;
	case GLSLstd450PackUnorm4x8:
		emit_unary_func_op(result_type, id, args[0], "pack_float_to_unorm4x8");
		break;
	case GLSLstd450PackSnorm2x16:
		emit_unary_func_op(result_type, id, args[0], "pack_float_to_snorm2x16");
		break;
	case GLSLstd450PackUnorm2x16:
		emit_unary_func_op(result_type, id, args[0], "pack_float_to_unorm2x16");
		break;
	case GLSLstd450PackHalf2x16:
		emit_unary_func_op(result_type, id, args[0], "pack_half_to_snorm2x16");
		break;

	case GLSLstd450UnpackSnorm4x8:
		emit_unary_func_op(result_type, id, args[0], "unpack_snorm4x8_to_float");
		break;
	case GLSLstd450UnpackUnorm4x8:
		emit_unary_func_op(result_type, id, args[0], "unpack_unorm4x8_to_float");
		break;
	case GLSLstd450UnpackSnorm2x16:
		emit_unary_func_op(result_type, id, args[0], "unpack_snorm2x16_to_float");
		break;
	case GLSLstd450UnpackUnorm2x16:
		emit_unary_func_op(result_type, id, args[0], "unpack_unorm2x16_to_float");
		break;
	case GLSLstd450UnpackHalf2x16:
		emit_unary_func_op(result_type, id, args[0], "unpack_snorm2x16_to_half");
		break;

	case GLSLstd450PackDouble2x32:
		emit_unary_func_op(result_type, id, args[0], "unsupported_GLSLstd450PackDouble2x32"); // Currently unsupported
		break;
	case GLSLstd450UnpackDouble2x32:
		emit_unary_func_op(result_type, id, args[0], "unsupported_GLSLstd450UnpackDouble2x32"); // Currently unsupported
		break;

	case GLSLstd450MatrixInverse:
	{
		auto &mat_type = get<SPIRType>(result_type);
		switch (mat_type.columns)
		{
		case 2:
			emit_unary_func_op(result_type, id, args[0], "spvInverse2x2");
			break;
		case 3:
			emit_unary_func_op(result_type, id, args[0], "spvInverse3x3");
			break;
		case 4:
			emit_unary_func_op(result_type, id, args[0], "spvInverse4x4");
			break;
		default:
			break;
		}
		break;
	}

	// TODO:
	//        GLSLstd450InterpolateAtCentroid (centroid_no_perspective qualifier)
	//        GLSLstd450InterpolateAtSample (sample_no_perspective qualifier)
	//        GLSLstd450InterpolateAtOffset

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
	decl += func_type_decl(type, func.self);
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
	bool coord_is_fp = (coord_type.basetype == SPIRType::Float) || (coord_type.basetype == SPIRType::Double);
	bool is_cube_fetch = false;

	string tex_coords = coord_expr;
	const char *alt_coord = "";
    const char *coord_cast = "";

	switch (imgtype.image.dim)
	{

	case Dim1D:
		if (coord_type.vecsize > 1)
			tex_coords += ".x";

		if (is_fetch)
			tex_coords = "uint(" + round_fp_tex_coords(tex_coords, coord_is_fp) + ")";

		alt_coord = ".y";
        coord_cast = "float";

		break;

	case DimBuffer:
		if (coord_type.vecsize > 1)
			tex_coords += ".x";

		if (is_fetch)
			tex_coords = "uint2(" + round_fp_tex_coords(tex_coords, coord_is_fp) + ", 0)"; // Metal textures are 2D

		alt_coord = ".y";
        coord_cast = "float";

		break;

	case Dim2D:
		if (coord_type.vecsize > 2)
			tex_coords += ".xy";

		if (is_fetch)
			tex_coords = "uint2(" + round_fp_tex_coords(tex_coords, coord_is_fp) + ")";

		alt_coord = ".z";
        coord_cast = "float2";

		break;

	case Dim3D:
		if (coord_type.vecsize > 3)
			tex_coords += ".xyz";

		if (is_fetch)
			tex_coords = "uint3(" + round_fp_tex_coords(tex_coords, coord_is_fp) + ")";

		alt_coord = ".w";
        coord_cast = "float3";

		break;

	case DimCube:
		if (is_fetch)
		{
			is_cube_fetch = true;
			tex_coords += ".xy";
			tex_coords = "uint2(" + round_fp_tex_coords(tex_coords, coord_is_fp) + ")";
		}
		else
		{
			if (coord_type.vecsize > 3)
				tex_coords += ".xyz";
		}

		alt_coord = ".w";
        coord_cast = "float3";

		break;

	default:
		break;
	}

	// If projection, use alt coord as divisor
	if (is_proj)
		tex_coords += " / " + coord_expr + alt_coord;

    // If the coords are half precision, cast them to full precision.
    if (!is_fetch && has_decoration(coord, DecorationRelaxedPrecision))
        tex_coords = string(coord_cast) + "(" + tex_coords + ")";

	if (!farg_str.empty())
		farg_str += ", ";
	farg_str += tex_coords;

	// If fetch from cube, add face explicitly
	if (is_cube_fetch)
		farg_str += ", uint(" + round_fp_tex_coords(coord_expr + ".z", coord_is_fp) + ")";

	// If array, use alt coord
	if (imgtype.image.arrayed)
		farg_str += ", uint(" + round_fp_tex_coords(coord_expr + alt_coord, coord_is_fp) + ")";

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
			if (coord_type.vecsize > 2)
				offset_expr += ".xy";

			farg_str += ", " + offset_expr;
			break;

		case Dim3D:
			if (coord_type.vecsize > 3)
				offset_expr += ".xyz";

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

// If the texture coordinates are floating point, invokes MSL round() function to round them.
string CompilerMSL::round_fp_tex_coords(string tex_coords, bool coord_is_fp)
{
	return coord_is_fp ? ("round(" + tex_coords + ")") : tex_coords;
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

	statement(pack_pfx, type_to_glsl(membertype, type.self, index), " ", qualifier, to_member_name(type, index),
	          member_attribute_qualifier(type, index), type_to_array_glsl(membertype), ";");
}

// Return a MSL qualifier for the specified function attribute member
string CompilerMSL::member_attribute_qualifier(const SPIRType &type, uint32_t index)
{
	auto &execution = get_entry_point();

	uint32_t mbr_type_id = type.member_types[index];
	auto &mbr_type = get<SPIRType>(mbr_type_id);

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
			case BuiltInPointSize:
				// Only mark the PointSize builtin if really rendering points.
				// Some shaders may include a PointSize builtin even when used to render
				// non-point topologies, and Metal will reject this builtin when compiling
				// the shader into a render pipeline that uses a non-point topology.
				return options.enable_point_size_builtin ? (string(" [[") + builtin_qualifier(builtin) + "]]") : "";

			case BuiltInPosition:
			case BuiltInLayer:
			case BuiltInClipDistance:
				return string(" [[") + builtin_qualifier(builtin) + "]]" + (mbr_type.array.empty() ? "" : " ");

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
string CompilerMSL::func_type_decl(SPIRType &type, uint32_t id)
{
	auto &execution = get_entry_point();
	// The regular function return type. If not processing the entry point function, that's all we need
	string return_type = type_to_glsl(type, id);
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
		return ((meta[type.self].decoration.decoration_flags & (1ull << DecorationBufferBlock)) != 0 &&
		        (meta[argument.self].decoration.decoration_flags & (1ull << DecorationNonWritable)) == 0) ?
		           "device" :
		           "constant";
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

			uint32_t var_id = var.self;

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
					ep_args += get_argument_address_space(var) + " " + type_to_glsl(type) + "& " + to_name(var_id);
					ep_args += " [[buffer(" + convert_to_string(get_metal_resource_index(var, type.basetype)) + ")]]";
					break;
				}
				case SPIRType::Sampler:
					if (!ep_args.empty())
						ep_args += ", ";
					ep_args += type_to_glsl(type) + " " + to_name(var_id);
					ep_args += " [[sampler(" + convert_to_string(get_metal_resource_index(var, type.basetype)) + ")]]";
					break;
				case SPIRType::Image:
					if (!ep_args.empty())
						ep_args += ", ";
					ep_args += type_to_glsl(type, var_id) + " " + to_name(var_id);
					ep_args += " [[texture(" + convert_to_string(get_metal_resource_index(var, type.basetype)) + ")]]";
					break;
				case SPIRType::SampledImage:
					if (!ep_args.empty())
						ep_args += ", ";
					ep_args += type_to_glsl(type, var_id) + " " + to_name(var_id);
					ep_args +=
					    " [[texture(" + convert_to_string(get_metal_resource_index(var, SPIRType::Image)) + ")]]";
					if (type.image.dim != DimBuffer)
					{
						ep_args += ", sampler " + to_sampler_expression(var_id);
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
				BuiltIn bi_type = meta[var_id].decoration.builtin_type;
				ep_args += builtin_type_decl(bi_type) + " " + to_expression(var_id);
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

// The optional id parameter indicates the object whose type we are trying
// to find the description for. It is optional. Most type descriptions do not
// depend on a specific object's use of that type.
string CompilerMSL::type_to_glsl(const SPIRType &type, uint32_t id, uint32_t member_index)
{
	// Ignore the pointer type since GLSL doesn't have pointers.

	string type_name;

	switch (type.basetype)
	{
	case SPIRType::Struct:
		// Need OpName lookup here to get a "sensible" name for a struct.
		return to_name(type.self);

	case SPIRType::Image:
	case SPIRType::SampledImage:
		return image_type_glsl(type, id);

	case SPIRType::Sampler:
		return "sampler";

	case SPIRType::Void:
		return "void";

	case SPIRType::AtomicCounter:
		return "atomic_uint";

	// Scalars
	case SPIRType::Boolean:
		type_name = "bool";
		break;
	case SPIRType::Char:
		type_name = "char";
		break;
	case SPIRType::Int:
		type_name = is_medium_precision(type, id, member_index) ? "short" : "int";
		break;
	case SPIRType::UInt:
		type_name = is_medium_precision(type, id, member_index) ? "ushort" : "uint";
		break;
	case SPIRType::Int64:
		type_name = "long"; // Currently unsupported
		break;
	case SPIRType::UInt64:
		type_name = "size_t";
		break;
	case SPIRType::Float:
		type_name = is_medium_precision(type, id, member_index) ? "half" : "float";
		break;
	case SPIRType::Double:
		type_name = "double"; // Currently unsupported
		break;

	default:
		return "unknown_type";
	}

	// Matrix?
	if (type.columns > 1)
		type_name += to_string(type.columns) + "x";

	// Vector or Matrix?
	if (type.vecsize > 1)
		type_name += to_string(type.vecsize);

	return type_name;
}

// Returns whether the type, as applied to an object, should use half-precision (short/half)
// instead of high precision (int/float). If id references a SPIRType, then member_index is
// used to reference the member.
bool CompilerMSL::is_medium_precision(const SPIRType &type, uint32_t id, uint32_t member_index)
{
    // If the type width is explicitely 16 bits, it defines a half-precision type.
    if (type.width == 16 && (type.basetype == SPIRType::Int ||
                             type.basetype == SPIRType::UInt ||
                             type.basetype == SPIRType::Float))
        return true;

    // If the object whose type is being
    auto *var_type = maybe_get<SPIRType>(id);
    if (var_type)
        return has_member_decoration(var_type->self, member_index, DecorationRelaxedPrecision);
    else
        return has_decoration(id, DecorationRelaxedPrecision);

}

// Returns an MSL string describing  the SPIR-V image type
string CompilerMSL::image_type_glsl(const SPIRType &type, uint32_t id)
{
	string img_type_name;

	// Bypass pointers because we need the real image struct
	auto &img_type = get<SPIRType>(type.self).image;

	if (img_type.depth)
	{
		switch (img_type.dim)
		{
		case Dim2D:
			img_type_name += (img_type.ms ? "depth2d_ms" : (img_type.arrayed ? "depth2d_array" : "depth2d"));
			break;
		case DimCube:
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
		case Dim1D:
			img_type_name += (img_type.arrayed ? "texture1d_array" : "texture1d");
			break;
		case DimBuffer:
		case Dim2D:
			img_type_name += (img_type.ms ? "texture2d_ms" : (img_type.arrayed ? "texture2d_array" : "texture2d"));
			break;
		case Dim3D:
			img_type_name += "texture3d";
			break;
		case DimCube:
			img_type_name += (img_type.arrayed ? "texturecube_array" : "texturecube");
			break;
		default:
			img_type_name += "unknown_texture_type";
			break;
		}
	}

	// Append the pixel type
	img_type_name += "<";
	img_type_name += type_to_glsl(get<SPIRType>(img_type.type), id);

	// For unsampled images, append the sample/read/write access qualifier.
	// For kernel images, the access qualifier my be supplied directly by SPIR-V.
	// Otherwise it may be set based on whether the image is read from or written to within the shader.
	if (type.basetype == SPIRType::Image && type.image.sampled == 2)
	{
		switch (img_type.access)
		{
		case AccessQualifierReadOnly:
			img_type_name += ", access::read";
			break;

		case AccessQualifierWriteOnly:
			img_type_name += ", access::write";
			break;

		case AccessQualifierReadWrite:
			img_type_name += ", access::read_write";
			break;

		default:
		{
			auto *p_var = maybe_get_backing_variable(id);
			if (p_var && !has_decoration(p_var->self, DecorationNonWritable))
			{
				img_type_name += ", access::";

				if (!has_decoration(p_var->self, DecorationNonReadable))
					img_type_name += "read_";

				img_type_name += "write";
			}
			break;
		}
		}
	}

	img_type_name += ">";

	return img_type_name;
}

string CompilerMSL::bitcast_glsl_op(const SPIRType &out_type, const SPIRType &in_type, uint32_t id)
{
	if ((out_type.basetype == SPIRType::UInt && in_type.basetype == SPIRType::Int) ||
	    (out_type.basetype == SPIRType::Int && in_type.basetype == SPIRType::UInt) ||
	    (out_type.basetype == SPIRType::UInt64 && in_type.basetype == SPIRType::Int64) ||
	    (out_type.basetype == SPIRType::Int64 && in_type.basetype == SPIRType::UInt64))
		return type_to_glsl(out_type, id);

	if ((out_type.basetype == SPIRType::UInt && in_type.basetype == SPIRType::Float) ||
	    (out_type.basetype == SPIRType::Int && in_type.basetype == SPIRType::Float) ||
	    (out_type.basetype == SPIRType::Float && in_type.basetype == SPIRType::UInt) ||
	    (out_type.basetype == SPIRType::Float && in_type.basetype == SPIRType::Int) ||
	    (out_type.basetype == SPIRType::Int64 && in_type.basetype == SPIRType::Double) ||
	    (out_type.basetype == SPIRType::UInt64 && in_type.basetype == SPIRType::Double) ||
	    (out_type.basetype == SPIRType::Double && in_type.basetype == SPIRType::Int64) ||
	    (out_type.basetype == SPIRType::Double && in_type.basetype == SPIRType::UInt64))
		return "as_type<" + type_to_glsl(out_type, id) + ">";

	return "";
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
		else if (execution.flags & (1ull << ExecutionModeDepthLess))
			return "depth(less)";
		else if (execution.flags & (1ull << ExecutionModeDepthUnchanged))
			return "depth(any)";
		else
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
	auto dec_mask = get_member_decoration_mask(struct_type.self, index);
	auto &type = get<SPIRType>(struct_type.member_types[index]);

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

		// For arrays, we can use ArrayStride to get an easy check.
		if (!type.array.empty())
			return type_struct_member_array_stride(struct_type, index) * type.array.back();

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
	auto &type = get<SPIRType>(struct_type.member_types[index]);

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
		auto dec_mask = get_member_decoration_mask(struct_type.self, index);
		if (dec_mask & (1ull << DecorationCPacked))
			return type.width / 8;
		else
			return get_declared_struct_member_size(struct_type, index) / type.columns;
	}
	}
}

bool CompilerMSL::skip_argument(uint32_t) const
{
	return false;
}

bool CompilerMSL::OpCodePreprocessor::handle(Op opcode, const uint32_t *args, uint32_t /*length*/)
{
	// Since MSL exists in a single execution scope, function prototype declarations are not
	// needed, and clutter the output. If secondary functions are output (either as a SPIR-V
	// function implementation or as indicated by the presence of OpFunctionCall), then set
	// suppress_missing_prototypes to suppress compiler warnings of missing function prototypes.

	// Mark if the input requires the implementation of an SPIR-V function that does not exist in Metal.
	SPVFuncImpl spv_func = compiler.get_spv_func_impl(opcode, args);
	if (spv_func != SPVFuncImplNone)
	{
		compiler.spv_function_implementations.insert(spv_func);
		suppress_missing_prototypes = true;
		return true;
	}

	switch (opcode)
	{

	case OpFunctionCall:
		suppress_missing_prototypes = true;
		break;

	case OpAtomicExchange:
	case OpAtomicCompareExchange:
	case OpAtomicCompareExchangeWeak:
	case OpAtomicLoad:
	case OpAtomicIIncrement:
	case OpAtomicIDecrement:
	case OpAtomicIAdd:
	case OpAtomicISub:
	case OpAtomicSMin:
	case OpAtomicUMin:
	case OpAtomicSMax:
	case OpAtomicUMax:
	case OpAtomicAnd:
	case OpAtomicOr:
	case OpAtomicXor:
		uses_atomics = true;
		break;

	default:
		break;
	}

	return true;
}

// Returns an enumeration of a SPIR-V function that needs to be output for certain Op codes.
CompilerMSL::SPVFuncImpl CompilerMSL::get_spv_func_impl(Op opcode, const uint32_t *args)
{
	switch (opcode)
	{
	case OpFMod:
		return SPVFuncImplMod;

	case OpExtInst:
	{
		uint32_t extension_set = args[2];
		if (get<SPIRExtension>(extension_set).ext == SPIRExtension::GLSL)
		{
			GLSLstd450 op_450 = static_cast<GLSLstd450>(args[3]);
			switch (op_450)
			{
			case GLSLstd450Radians:
				return SPVFuncImplRadians;
			case GLSLstd450Degrees:
				return SPVFuncImplDegrees;
			case GLSLstd450FindILsb:
				return SPVFuncImplFindILsb;
			case GLSLstd450FindSMsb:
				return SPVFuncImplFindSMsb;
			case GLSLstd450FindUMsb:
				return SPVFuncImplFindUMsb;
			case GLSLstd450MatrixInverse:
			{
				auto &mat_type = get<SPIRType>(args[0]);
				switch (mat_type.columns)
				{
				case 2:
					return SPVFuncImplInverse2x2;
				case 3:
					return SPVFuncImplInverse3x3;
				case 4:
					return SPVFuncImplInverse4x4;
				default:
					break;
				}
				break;
			}
			default:
				break;
			}
		}
		break;
	}

	default:
		break;
	}
	return SPVFuncImplNone;
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
