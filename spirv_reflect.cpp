/*
 * Copyright 2015-2018 ARM Limited
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

#include "spirv_reflect.hpp"
#include <cassert>

using namespace spv;
using namespace spirv_cross;
using namespace std;

#ifdef SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS
static inline void THROW(const char *str)
{
	fprintf(stderr, "SPIRV-Cross will abort: %s\n", str);
	fflush(stderr);
	abort();
}
#else
#define THROW(x) throw runtime_error(x)
#endif

void CompilerReflection::set_format(const std::string &format)
{
	if (format != "json")
	{
		THROW("Unsupported format");
	}
}

// Hackery to emit JSON without using
void CompilerReflection::begin_json_array()
{
	if (!json_state.empty() && json_state.top().second)
	{
		statement_inner(",\n");
	}
	statement("[");
	++indent;
	json_state.emplace(JsonType::Array, false);
}

void CompilerReflection::end_json_array()
{
	if (json_state.empty() || json_state.top().first != JsonType::Array)
		SPIRV_CROSS_THROW("Invalid JSON state");
	if (json_state.top().second)
	{
		statement_inner("\n");
	}
	--indent;
	statement_no_return("]");
	json_state.pop();
	if (!json_state.empty())
	{
		json_state.top().second = true;
	}
}

void CompilerReflection::emit_json_array_value(const std::string &value)
{
	if (json_state.empty() || json_state.top().first != JsonType::Array)
		SPIRV_CROSS_THROW("Invalid JSON state");

	if (json_state.top().second)
		statement_inner(",\n");

	statement_no_return("\"", value, "\"");
	json_state.top().second = true;
}

void CompilerReflection::emit_json_array_value(uint32_t value)
{
	if (json_state.empty() || json_state.top().first != JsonType::Array)
		SPIRV_CROSS_THROW("Invalid JSON state");
	if (json_state.top().second)
		statement_inner(",\n");
	statement_no_return(std::to_string(value));
	json_state.top().second = true;
}

void CompilerReflection::begin_json_object()
{
	if (!json_state.empty() && json_state.top().second)
	{
		statement_inner(",\n");
	}
	Parent::begin_scope();
	json_state.emplace(JsonType::Object, false);
}

void CompilerReflection::end_json_object()
{
	if (json_state.empty() || json_state.top().first != JsonType::Object)
		SPIRV_CROSS_THROW("Invalid JSON state");
	if (json_state.top().second)
	{
		statement_inner("\n");
	}
	--indent;
	statement_no_return("}");
	json_state.pop();
	if (!json_state.empty())
	{
		json_state.top().second = true;
	}
}

void CompilerReflection::emit_json_key(const std::string &key)
{
	if (json_state.empty() || json_state.top().first != JsonType::Object)
		SPIRV_CROSS_THROW("Invalid JSON state");

	if (json_state.top().second)
		statement_inner(",\n");
	statement_no_return("\"", key, "\" : ");
	json_state.top().second = true;
}

void CompilerReflection::emit_json_key_value(const std::string &key, const std::string &value)
{
	emit_json_key(key);
	statement_inner("\"", value, "\"");
}

void CompilerReflection::emit_json_key_value(const std::string &key, uint32_t value)
{
	emit_json_key(key);
	statement_inner(value);
}

void CompilerReflection::emit_json_key_value(const std::string &key, bool value)
{
	emit_json_key(key);
	statement_inner(value ? "true" : "false");
}

void CompilerReflection::emit_json_key_object(const std::string &key)
{
	emit_json_key(key);
	statement_inner("{\n");
	++indent;
	json_state.emplace(JsonType::Object, false);
}

void CompilerReflection::emit_json_key_array(const std::string &key)
{
	emit_json_key(key);
	statement_inner("[\n");
	++indent;
	json_state.emplace(JsonType::Array, false);
}

string CompilerReflection::compile()
{
	// force a GLSL compilation in the parent class in order to set all our internal state
	CompilerGLSL::compile();

    buffer.reset();

	// Force a classic "C" locale, reverts when function returns
	ClassicLocale classic_locale;

	// Move constructor for this type is broken on GCC 4.9 ...
	buffer = unique_ptr<ostringstream>(new ostringstream());
	begin_json_object();
	emit_extensions();
	emit_types();
	emit_resources();
	end_json_object();

	return buffer->str();
}

void CompilerReflection::emit_extensions()
{
	if (forced_extensions.empty())
		return;

	emit_json_key_array("extensions");
	for (const auto &ext : forced_extensions)
		emit_json_array_value(ext);
	end_json_array();
}

void CompilerReflection::emit_types()
{
	bool emitted_open_tag = false;
	for (auto &id : ids)
	{
		if (id.get_type() == TypeType)
		{
			auto &type = id.get<SPIRType>();
			if (type.basetype == SPIRType::Struct && type.array.empty() && !type.pointer)
			{
				emit_type(type, emitted_open_tag);
			}
		}
	}

	if (emitted_open_tag)
	{
		end_json_object();
	}
}

void CompilerReflection::emit_type(const SPIRType &type, bool &emitted_open_tag)
{
	auto name = type_to_glsl(type);

	// Struct types can be stamped out multiple times
	// with just different offsets, matrix layouts, etc ...
	// Type-punning with these types is legal, which complicates things
	// when we are storing struct and array types in an SSBO for example.
	// If the type master is packed however, we can no longer assume that the struct declaration will be redundant.
	if (type.type_alias != 0 && !has_decoration(type.type_alias, DecorationCPacked))
		return;

	//add_resource_name(type.self);
	//type.member_name_cache.clear();

	if (!emitted_open_tag)
	{
		emit_json_key_object("types");
		emitted_open_tag = true;
	}
	emit_json_key_array(name);
	auto size = type.member_types.size();
	for (uint32_t i = 0; i < size; ++i)
	{
		emit_type_member(type, i);
	}
	end_json_array();
}

void CompilerReflection::emit_type_member(const SPIRType &type, uint32_t index)
{
	auto &membertype = get<SPIRType>(type.member_types[index]);
	begin_json_object();
	auto name = to_member_name(type, index);
	emit_json_key_value("name", name);
	emit_json_key_value("type", type_to_glsl(membertype));
	emit_type_member_qualifiers(type, index);
	end_json_object();
}

void CompilerReflection::emit_type_array(const SPIRType &type)
{
	if (!type.array.empty())
	{
		emit_json_key_array("array");
		for (const auto &value : type.array)
			emit_json_array_value(value);
		end_json_array();
	}
}

void CompilerReflection::emit_type_member_qualifiers(const SPIRType &type, uint32_t index)
{
	auto flags = combined_decoration_for_member(type, index);
	if (flags.get(DecorationRowMajor))
		emit_json_key_value("row_major", true);

	auto &membertype = get<SPIRType>(type.member_types[index]);
	emit_type_array(membertype);
	auto &memb = meta[type.self].members;
	if (index < memb.size())
	{
		auto &dec = memb[index];
		if (dec.decoration_flags.get(DecorationLocation) && can_use_io_location(type.storage, true))
			emit_json_key_value("location", dec.location);
		if (has_decoration(type.self, DecorationCPacked) && dec.decoration_flags.get(DecorationOffset))
			emit_json_key_value("offset", dec.offset);
	}
}

const char *CompilerReflection::execution_model_to_str(spv::ExecutionModel model)
{
	switch (model)
	{
	case spv::ExecutionModelVertex:
		return "vertex";
	case spv::ExecutionModelTessellationControl:
		return "tessellation control";
	case ExecutionModelTessellationEvaluation:
		return "tessellation evaluation";
	case ExecutionModelGeometry:
		return "geometry";
	case ExecutionModelFragment:
		return "fragment";
	case ExecutionModelGLCompute:
		return "compute";
	default:
		return "???";
	}
}

void CompilerReflection::emit_entry_points()
{
	auto entry_points = get_entry_points_and_stages();
	if (!entry_points.empty())
	{
		emit_json_key_array("entryPoints");
		for (auto &e : entry_points)
		{
			begin_json_object();
			emit_json_key_value("name", e.name);
			emit_json_key_value("mode", execution_model_to_str(e.execution_model));
			end_json_object();
		}
		end_json_array();
	}

#if 0
#define CHECK_MODE(m)                  \
	case ExecutionMode##m:             \
		fprintf(stderr, "  %s\n", #m); \
		break

    auto &modes = get_execution_mode_bitset();
    modes.for_each_bit([&](uint32_t i) {
        auto mode = static_cast<ExecutionMode>(i);
        uint32_t arg0 = get_execution_mode_argument(mode, 0);
        uint32_t arg1 = get_execution_mode_argument(mode, 1);
        uint32_t arg2 = get_execution_mode_argument(mode, 2);

        switch (static_cast<ExecutionMode>(i)) {
        case ExecutionModeInvocations:
            fprintf(stderr, "  Invocations: %u\n", arg0);
            break;

        case ExecutionModeLocalSize:
            fprintf(stderr, "  LocalSize: (%u, %u, %u)\n", arg0, arg1, arg2);
            break;

        case ExecutionModeOutputVertices:
            fprintf(stderr, "  OutputVertices: %u\n", arg0);
            break;


            CHECK_MODE(SpacingEqual);
            CHECK_MODE(SpacingFractionalEven);
            CHECK_MODE(SpacingFractionalOdd);
            CHECK_MODE(VertexOrderCw);
            CHECK_MODE(VertexOrderCcw);
            CHECK_MODE(PixelCenterInteger);
            CHECK_MODE(OriginUpperLeft);
            CHECK_MODE(OriginLowerLeft);
            CHECK_MODE(EarlyFragmentTests);
            CHECK_MODE(PointMode);
            CHECK_MODE(Xfb);
            CHECK_MODE(DepthReplacing);
            CHECK_MODE(DepthGreater);
            CHECK_MODE(DepthLess);
            CHECK_MODE(DepthUnchanged);
            CHECK_MODE(LocalSizeHint);
            CHECK_MODE(InputPoints);
            CHECK_MODE(InputLines);
            CHECK_MODE(InputLinesAdjacency);
            CHECK_MODE(Triangles);
            CHECK_MODE(InputTrianglesAdjacency);
            CHECK_MODE(Quads);
            CHECK_MODE(Isolines);
            CHECK_MODE(OutputPoints);
            CHECK_MODE(OutputLineStrip);
            CHECK_MODE(OutputTriangleStrip);
            CHECK_MODE(VecTypeHint);
            CHECK_MODE(ContractionOff);

        default:
            break;
        }
    });
#endif
}

void CompilerReflection::emit_resources()
{
	auto res = get_shader_resources();
	emit_resources("subpass_inputs", res.subpass_inputs);
	emit_resources("inputs", res.stage_inputs);
	emit_resources("outputs", res.stage_outputs);
	emit_resources("textures", res.sampled_images);
	emit_resources("separate_images", res.separate_images);
	emit_resources("separate_samplers", res.separate_samplers);
	emit_resources("images", res.storage_images);
	emit_resources("ssbos", res.storage_buffers);
	emit_resources("ubos", res.uniform_buffers);
	emit_resources("push", res.push_constant_buffers);
	emit_resources("counters", res.atomic_counters);
}

void CompilerReflection::emit_resources(const char *tag, const vector<Resource> &resources)
{
	if (resources.empty())
	{
		return;
	}

	bool print_ssbo = !strcmp(tag, "ssbos");

	emit_json_key_array(tag);
	for (auto &res : resources)
	{
		auto qualifiers = to_qualifiers_glsl(res.id);
		auto &type = get_type(res.type_id);
		auto typeflags = meta[type.self].decoration.decoration_flags;
		auto &dec = meta[type.self].decoration;
		auto &mask = get_decoration_bitset(res.id);
		if (print_ssbo && buffer_is_hlsl_counter_buffer(res.id))
			continue;

		// If we don't have a name, use the fallback for the type instead of the variable
		// for SSBOs and UBOs since those are the only meaningful names to use externally.
		// Push constant blocks are still accessed by name and not block name, even though they are technically Blocks.
		bool is_push_constant = get_storage_class(res.id) == StorageClassPushConstant;
		bool is_block = get_decoration_bitset(type.self).get(DecorationBlock) ||
		                get_decoration_bitset(type.self).get(DecorationBufferBlock);
		uint32_t fallback_id = !is_push_constant && is_block ? res.base_type_id : res.id;
		begin_json_object();
		emit_json_key_value("type", type_to_glsl(type));
		emit_json_key_value("name", !res.name.empty() ? res.name : get_fallback_name(fallback_id));
		{
			bool push_constant_block = type.storage == StorageClassPushConstant;
			bool ssbo_block = type.storage == StorageClassStorageBuffer ||
			                  (type.storage == StorageClassUniform && typeflags.get(DecorationBufferBlock));
			if (type.storage == StorageClassUniform && typeflags.get(DecorationBlock))
			{
				if (buffer_is_packing_standard(type, BufferPackingStd140))
					emit_json_key_value("std140", true);
				else if (buffer_is_packing_standard(type, BufferPackingStd140EnhancedLayout))
					emit_json_key_value("std140", true);
			}
			else if (push_constant_block || ssbo_block)
			{
				if (buffer_is_packing_standard(type, BufferPackingStd430))
					emit_json_key_value("std430", true);
				else if (buffer_is_packing_standard(type, BufferPackingStd140))
					emit_json_key_value("std140", true);
				else if (buffer_is_packing_standard(type, BufferPackingStd140EnhancedLayout))
					emit_json_key_value("std140", true);
				else if (buffer_is_packing_standard(type, BufferPackingStd430EnhancedLayout))
					emit_json_key_value("std430", true);
			}

			if (ssbo_block)
			{
				auto bufferFlags = get_buffer_block_flags(res.id);
				if (bufferFlags.get(DecorationNonReadable))
					emit_json_key_value("writeonly", true);
				if (bufferFlags.get(DecorationNonWritable))
					emit_json_key_value("readonly", true);
				if (bufferFlags.get(DecorationRestrict))
					emit_json_key_value("restrict", true);
				if (bufferFlags.get(DecorationCoherent))
					emit_json_key_value("coherent", true);
			}
		}

		emit_type_array(type);

		{
			bool is_sized_block = is_block && (get_storage_class(res.id) == StorageClassUniform ||
			                                   get_storage_class(res.id) == StorageClassUniformConstant);
			uint32_t block_size = 0;
			if (is_sized_block)
				block_size = uint32_t(get_declared_struct_size(get_type(res.base_type_id)));
			if (is_sized_block)
				emit_json_key_value("block_size", block_size);
		}

		if (type.storage == StorageClassPushConstant)
			emit_json_key_value("push_constant", true);
		if (mask.get(DecorationLocation))
			emit_json_key_value("location", get_decoration(res.id, DecorationLocation));
		if (mask.get(DecorationRowMajor))
			emit_json_key_value("row_major", true);
		if (mask.get(DecorationColMajor))
			emit_json_key_value("column_major", true);
		if (mask.get(DecorationIndex))
			emit_json_key_value("index", get_decoration(res.id, DecorationIndex));
		if (type.storage != StorageClassPushConstant && mask.get(DecorationDescriptorSet))
			emit_json_key_value("set", get_decoration(res.id, DecorationDescriptorSet));
		if (mask.get(DecorationBinding))
			emit_json_key_value("binding", get_decoration(res.id, DecorationBinding));
		if (mask.get(DecorationInputAttachmentIndex))
			emit_json_key_value("input_attachment_index", get_decoration(res.id, DecorationInputAttachmentIndex));
		if (mask.get(DecorationOffset))
			emit_json_key_value("offset", get_decoration(res.id, DecorationOffset));

		// For images, the type itself adds a layout qualifer.
		// Only emit the format for storage images.
		if (type.basetype == SPIRType::Image && type.image.sampled == 2)
		{
			const char *fmt = format_to_glsl(type.image.format);
			if (fmt != nullptr)
				emit_json_key_value("format", std::string(fmt));
		}
		end_json_object();
	}
	end_json_array();
}
