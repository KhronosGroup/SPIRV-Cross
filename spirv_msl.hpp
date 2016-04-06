/*
 * Copyright 2015-2016 The Brenwill Workshop Ltd.
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

#ifndef SPIRV_MSL_HPP
#define SPIRV_MSL_HPP

#include "spirv_glsl.hpp"
#include <vector>
#include <set>


// Hash implementation to allow spv::BuiltIn to be used as a key in an unordered_map
namespace std {
	template <>
	struct hash<spv::BuiltIn>
	{
		std::size_t operator()(const spv::BuiltIn& k) const
		{
			return k;
		}
	};
}

namespace spirv_cross {

	// Options for compiling to Metal Shading Language
	struct MSLOptions
	{
		uint32_t vtx_attr_stage_in_binding= 0;
		bool flip_vert_y = true;
		bool flip_frag_y =true;
		bool is_rendering_points = false;
	};

	// Defines characteristics of vertex attributes at a particular location
	struct MSLVertexAttr
	{
		uint32_t location = 0;
		uint32_t buffer = 0;
		uint32_t offset = 0;
		uint32_t stride = 0;
		bool per_instance = false;
	};

	// Specifies the binding index of a Metal resource for a binding within a descriptor set.
	// Taken together, the stage, desc_set and binding combine to form a reference to a resource
	// descriptor used in a particular shading stage. Generally, only one of the buffer, texture,
	// or sampler elements will be populated.
	struct MSLResourceBinding
	{
		spv::ExecutionModel stage;
		uint32_t desc_set = 0;
		uint32_t binding = 0;

		uint32_t buffer = 0;
		uint32_t texture = 0;
		uint32_t sampler = 0;
	};

	// Special constant used in a MSLResourceBinding desc_set
	// element to indicate the bindings for the push constants.
	static const uint32_t kPushConstDescSet = UINT32_MAX;

	// Special constant used in a MSLResourceBinding binding
	// element to indicate the bindings for the push constants.
	static const uint32_t kPushConstBinding = 0;


	// Decompiles SPIR-V to Metal Shading Language
    class CompilerMSL : public CompilerGLSL
	{
        public:

		CompilerMSL(std::vector<uint32_t> spirv,
					MSLOptions* p_msl_options = nullptr,
					std::vector<MSLVertexAttr>* p_vtx_attrs = nullptr,
					std::vector<MSLResourceBinding>* p_res_bindings = nullptr);

            std::string compile() override;

        protected:
            void emit_header() override;
			void emit_function_prototype(SPIRFunction &func, uint64_t return_flags) override;
			void emit_texture_op(const Instruction &i) override;
			void emit_fixup() override;
			std::string type_to_glsl(const SPIRType &type) override;
			std::string image_type_glsl(const SPIRType &type) override;
			std::string builtin_to_glsl(spv::BuiltIn builtin) override;
			std::string member_decl(const SPIRType &type, const SPIRType &member_type, uint32_t member) override;
			std::string constant_expression(const SPIRConstant &c) override;
			size_t get_declared_struct_member_size(const SPIRType &struct_type, uint32_t index) const override;

			void post_parse();
			void extract_builtins();
			void add_interface_structs();
			void bind_vertex_attributes(std::set<uint32_t>& bindings);
			uint32_t add_interface_struct(spv::StorageClass storage, uint32_t vtx_binding = 0);
			void emit_resources();
			void emit_interface_block(uint32_t ib_var_id);
			void emit_function_prototype(SPIRFunction &func, bool is_decl);
			void emit_function_declarations();

			std::string func_type_decl(SPIRType& type);
			std::string clean_func_name(std::string func_name);
			std::string entry_point_args(bool append_comma);
			std::string get_entry_point_name();
			std::string builtin_qualifier(spv::BuiltIn builtin);
			std::string builtin_type_decl(spv::BuiltIn builtin);
			std::string member_attribute_qualifier(const SPIRType &type, uint32_t index);
            std::string argument_decl(const SPIRFunction::Parameter &arg);
			std::string get_vtx_idx_var_name(bool per_instance);
			uint32_t get_metal_resource_index(SPIRVariable& var, SPIRType::BaseType basetype);
			uint32_t get_ordered_member_location(uint32_t type_id, uint32_t index);
			uint32_t pad_to_offset(SPIRType& struct_type, uint32_t offset, uint32_t struct_size);
			SPIRType& get_pad_type(uint32_t pad_len);
			size_t get_declared_type_size(const SPIRType& type) const;
			size_t get_declared_type_size(const SPIRType& type, uint64_t dec_mask) const;

			MSLOptions msl_options;
			std::unordered_map<uint32_t, MSLVertexAttr> vtx_attrs_by_location;
			std::vector<MSLResourceBinding> resource_bindings;
			std::unordered_map<spv::BuiltIn, uint32_t> builtin_vars;
			MSLResourceBinding next_metal_resource_index;
			std::unordered_map<uint32_t, uint32_t> pad_type_ids_by_pad_len;

			std::string stage_in_var_name = "in";
			std::vector<uint32_t> stage_in_var_ids;
			std::string stage_out_var_name = "out";
			uint32_t stage_out_var_id = 0;
			std::string sampler_name_suffix = "Smplr";
			std::string qual_pos_var_name;
    };

	// Sorts the members of a SPIRType and associated Meta info based on the location
	// and builtin decorations of the members. Members are rearranged by location,
	// with all builtin members appearing a the end.
	struct MemberSorterByLocation
	{
		void sort();
		bool operator() (uint32_t mbr_idx1,uint32_t mbr_idx2);
		MemberSorterByLocation(SPIRType& type, Meta& meta) : type(type), meta(meta) {}
		SPIRType& type;
		Meta& meta;
	};

}

#endif
