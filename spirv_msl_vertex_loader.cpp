/*
 * Copyright 2023 Evan Tang for CodeWeavers
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

#include "spirv_msl.hpp"

using namespace SPIRV_CROSS_NAMESPACE;

template <typename OutType, std::size_t Size>
struct ConstexprLookupTable
{
	OutType data[Size];
	constexpr const OutType& operator[](std::size_t idx) const { return data[idx]; }
	static constexpr std::size_t size() { return Size; }
};

template <typename First, typename Second>
struct ConstexprPair
{
	First first;
	Second second;
};

#if __cplusplus >= 201300L
template <uint32_t Highest, typename OutType, uint32_t InSize, typename IndexType>
constexpr static ConstexprLookupTable<OutType, Highest + 1> gen_lookup_table(
    const ConstexprPair<IndexType, OutType> (&entry_list)[InSize])
{
#define CXPR_ASSERT_REL(x) ((x) ? static_cast<void>(0) : abort())
	bool written[Highest + 1] = {};
	ConstexprLookupTable<OutType, Highest + 1> output = {};
	for (const auto &pair : entry_list)
	{
		uint32_t index = static_cast<uint32_t>(pair.first);
		CXPR_ASSERT_REL(!written[index] && "Duplicate entry!");
		written[index] = true;
		output.data[index] = pair.second;
	}
	CXPR_ASSERT_REL(written[Highest] && "Highest too high");
#undef CXPR_ASSERT_REL
	return output;
}
#else
namespace detail
{

// From https://stackoverflow.com/a/61447603
template<class T, T... Ints>
struct integer_sequence {};

template<std::size_t... Ints>
using index_sequence = integer_sequence<std::size_t, Ints...>;

template<typename Firsts, typename Last>
struct index_sequence_eights_append;

template<std::size_t... N, std::size_t... M>
struct index_sequence_eights_append<index_sequence<N...>, index_sequence<M...>> {
	using type = index_sequence<
		N..., (sizeof...(N) + N)..., (2u * sizeof...(N) + N)..., (3u * sizeof...(N) + N)...,
		(4u * sizeof...(N) + N)..., (5u * sizeof...(N) + N)..., (6u * sizeof...(N) + N)...,
		(7u * sizeof...(N) + M)...
	>;
};

template<std::size_t N>
struct make_index_sequence_helper {
	using type = typename index_sequence_eights_append<typename make_index_sequence_helper<N / 8u>::type, typename make_index_sequence_helper<N - 7u * (N / 8u)>::type>::type;
};

template<> struct make_index_sequence_helper<0> { using type = index_sequence<>; };
template<> struct make_index_sequence_helper<1> { using type = index_sequence<0>; };
template<> struct make_index_sequence_helper<2> { using type = index_sequence<0, 1>; };
template<> struct make_index_sequence_helper<3> { using type = index_sequence<0, 1, 2>; };
template<> struct make_index_sequence_helper<4> { using type = index_sequence<0, 1, 2, 3>; };
template<> struct make_index_sequence_helper<5> { using type = index_sequence<0, 1, 2, 3, 4>; };
template<> struct make_index_sequence_helper<6> { using type = index_sequence<0, 1, 2, 3, 4, 5>; };
template<> struct make_index_sequence_helper<7> { using type = index_sequence<0, 1, 2, 3, 4, 5, 6>; };

template<std::size_t N>
using make_index_sequence = typename make_index_sequence_helper<N>::type;

template <typename OutType, uint32_t InSize, typename IndexType, uint32_t Idx>
struct get_matching_element
{
	static constexpr OutType search(const ConstexprPair<IndexType, OutType> (&entry_list)[InSize], IndexType needle)
	{
		return entry_list[Idx].first == needle ?
		           entry_list[Idx].second :
		           get_matching_element<OutType, InSize, IndexType, Idx + 1>::search(entry_list, needle);
	}
};

template <typename OutType, uint32_t InSize, typename IndexType>
struct get_matching_element<OutType, InSize, IndexType, InSize>
{
	static constexpr OutType search(const ConstexprPair<IndexType, OutType> (&)[InSize], IndexType)
	{
		return {};
	}
};

template <typename OutType, uint32_t InSize, typename IndexType, std::size_t... I>
constexpr static ConstexprLookupTable<OutType, sizeof...(I)> gen_lookup_table(
    const ConstexprPair<IndexType, OutType> (&entry_list)[InSize], index_sequence<I...>)
{
	return { get_matching_element<OutType, InSize, IndexType, 0>::search(entry_list, static_cast<IndexType>(I))... };
}

} // namespace detail

template <uint32_t Highest, typename OutType, uint32_t InSize, typename IndexType>
constexpr static ConstexprLookupTable<OutType, Highest + 1> gen_lookup_table(
    const ConstexprPair<IndexType, OutType> (&entry_list)[InSize])
{
	return detail::gen_lookup_table(entry_list, detail::make_index_sequence<Highest + 1>{});
}
#endif

const MSLFormatInfo &CompilerMSL::get_format_info(MSLFormat format)
{
	enum class Type
	{
		SFloat,
		UFloat,
		SInt,
		UInt,
		SNorm,
		UNorm,
		SRGB,
	};
	enum class PixelOrder
	{
		RGB,
		BGR,
	};
	// clang-format off
#define FORMAT(log2_align, elems, packing, type, packed, order, vk_packed) \
	MSLFormatInfo( \
		/* log2_align = */ log2_align, \
		/* num_elems = */ elems, \
		/* packing = */ MSLFormatPacking::packing, \
		/* is_float = */ (Type::type == Type::SFloat || Type::type == Type::UFloat), \
		/* is_signed = */ (Type::type == Type::SFloat || Type::type == Type::SInt || Type::type == Type::SNorm), \
		/* is_normalized = */ (Type::type == Type::SNorm || Type::type == Type::UNorm), \
		/* is_packed = */ packed, \
		/* vk_packed = */ vk_packed, \
		/* is_bgr = */ (PixelOrder::order == PixelOrder::BGR), \
		/* is_srgb = */ (Type::type == Type::SRGB) \
	)
#define SIMPLE_FORMAT_VK_PACKED(log2_align, elems, type, order) FORMAT(log2_align, elems, EvenAHigh, type, false, order, true)
#define SIMPLE_FORMAT(log2_align, elems, type, order) FORMAT(log2_align, elems, EvenAHigh, type, false, order, false)
#define PACKED_FORMAT(log2_align, elems, packing, type, order) FORMAT(log2_align, elems, packing, type, true, order, false)
	static constexpr ConstexprPair<MSLFormat, MSLFormatInfo> s_vertex_info_entries[] = {
		{ MSL_FORMAT_UNDEFINED, {} },
		// Packed Formats
		// (VkFormat uses big endian names, which makes the bit ordering extra fun)
		{ MSL_FORMAT_R4G4_UNORM_PACK8,           PACKED_FORMAT(0, 2, EvenALow,  UNorm,  RGB) },
		{ MSL_FORMAT_R4G4B4A4_UNORM_PACK16,      PACKED_FORMAT(1, 4, EvenALow,  UNorm,  RGB) },
		{ MSL_FORMAT_B4G4R4A4_UNORM_PACK16,      PACKED_FORMAT(1, 4, EvenALow,  UNorm,  BGR) },
		{ MSL_FORMAT_R5G6B5_UNORM_PACK16,        PACKED_FORMAT(1, 3, NoAlpha,   UNorm,  BGR) },
		{ MSL_FORMAT_B5G6R5_UNORM_PACK16,        PACKED_FORMAT(1, 3, NoAlpha,   UNorm,  RGB) },
		{ MSL_FORMAT_R5G5B5A1_UNORM_PACK16,      PACKED_FORMAT(1, 4, AlphaLow,  UNorm,  RGB) },
		{ MSL_FORMAT_B5G5R5A1_UNORM_PACK16,      PACKED_FORMAT(1, 4, AlphaLow,  UNorm,  BGR) },
		{ MSL_FORMAT_A1R5G5B5_UNORM_PACK16,      PACKED_FORMAT(1, 4, AlphaHigh, UNorm,  BGR) },
		{ MSL_FORMAT_A2R10G10B10_UNORM_PACK32,   PACKED_FORMAT(2, 4, AlphaHigh, UNorm,  BGR) },
		{ MSL_FORMAT_A2R10G10B10_SNORM_PACK32,   PACKED_FORMAT(2, 4, AlphaHigh, SNorm,  BGR) },
		{ MSL_FORMAT_A2R10G10B10_USCALED_PACK32, PACKED_FORMAT(2, 4, AlphaHigh, UInt,   BGR) },
		{ MSL_FORMAT_A2R10G10B10_SSCALED_PACK32, PACKED_FORMAT(2, 4, AlphaHigh, SInt,   BGR) },
		{ MSL_FORMAT_A2R10G10B10_UINT_PACK32,    PACKED_FORMAT(2, 4, AlphaHigh, UInt,   BGR) },
		{ MSL_FORMAT_A2R10G10B10_SINT_PACK32,    PACKED_FORMAT(2, 4, AlphaHigh, SInt,   BGR) },
		{ MSL_FORMAT_A2B10G10R10_UNORM_PACK32,   PACKED_FORMAT(2, 4, AlphaHigh, UNorm,  RGB) },
		{ MSL_FORMAT_A2B10G10R10_SNORM_PACK32,   PACKED_FORMAT(2, 4, AlphaHigh, SNorm,  RGB) },
		{ MSL_FORMAT_A2B10G10R10_USCALED_PACK32, PACKED_FORMAT(2, 4, AlphaHigh, UInt,   RGB) },
		{ MSL_FORMAT_A2B10G10R10_SSCALED_PACK32, PACKED_FORMAT(2, 4, AlphaHigh, SInt,   RGB) },
		{ MSL_FORMAT_A2B10G10R10_UINT_PACK32,    PACKED_FORMAT(2, 4, AlphaHigh, UInt,   RGB) },
		{ MSL_FORMAT_A2B10G10R10_SINT_PACK32,    PACKED_FORMAT(2, 4, AlphaHigh, SInt,   RGB) },
		{ MSL_FORMAT_B10G11R11_UFLOAT_PACK32,    PACKED_FORMAT(2, 3, NoAlpha,   UFloat, RGB) },
		{ MSL_FORMAT_E5B9G9R9_UFLOAT_PACK32,     PACKED_FORMAT(2, 3, AlphaHigh, UFloat, RGB) },

		// These formats are packed in Vulkan, but can be treated not-packed by us
		// (Since VkFormat names are big endian, RGB/BGR will be reversed from expected)
		{ MSL_FORMAT_A8B8G8R8_UNORM_PACK32,   SIMPLE_FORMAT_VK_PACKED(0, 4, UNorm, RGB) },
		{ MSL_FORMAT_A8B8G8R8_SNORM_PACK32,   SIMPLE_FORMAT_VK_PACKED(0, 4, SNorm, RGB) },
		{ MSL_FORMAT_A8B8G8R8_USCALED_PACK32, SIMPLE_FORMAT_VK_PACKED(0, 4, UInt,  RGB) },
		{ MSL_FORMAT_A8B8G8R8_SSCALED_PACK32, SIMPLE_FORMAT_VK_PACKED(0, 4, SInt,  RGB) },
		{ MSL_FORMAT_A8B8G8R8_UINT_PACK32,    SIMPLE_FORMAT_VK_PACKED(0, 4, UInt,  RGB) },
		{ MSL_FORMAT_A8B8G8R8_SINT_PACK32,    SIMPLE_FORMAT_VK_PACKED(0, 4, SInt,  RGB) },

		// 8-bit non-packed formats
		{ MSL_FORMAT_R8_UNORM,         SIMPLE_FORMAT(0, 1, UNorm, RGB) },
		{ MSL_FORMAT_R8_SNORM,         SIMPLE_FORMAT(0, 1, SNorm, RGB) },
		{ MSL_FORMAT_R8_USCALED,       SIMPLE_FORMAT(0, 1, UInt,  RGB) },
		{ MSL_FORMAT_R8_SSCALED,       SIMPLE_FORMAT(0, 1, SInt,  RGB) },
		{ MSL_FORMAT_R8_UINT,          SIMPLE_FORMAT(0, 1, UInt,  RGB) },
		{ MSL_FORMAT_R8_SINT,          SIMPLE_FORMAT(0, 1, SInt,  RGB) },
		{ MSL_FORMAT_R8G8_UNORM,       SIMPLE_FORMAT(0, 2, UNorm, RGB) },
		{ MSL_FORMAT_R8G8_SNORM,       SIMPLE_FORMAT(0, 2, SNorm, RGB) },
		{ MSL_FORMAT_R8G8_USCALED,     SIMPLE_FORMAT(0, 2, UInt,  RGB) },
		{ MSL_FORMAT_R8G8_SSCALED,     SIMPLE_FORMAT(0, 2, SInt,  RGB) },
		{ MSL_FORMAT_R8G8_UINT,        SIMPLE_FORMAT(0, 2, UInt,  RGB) },
		{ MSL_FORMAT_R8G8_SINT,        SIMPLE_FORMAT(0, 2, SInt,  RGB) },
		{ MSL_FORMAT_R8G8B8_UNORM,     SIMPLE_FORMAT(0, 3, UNorm, RGB) },
		{ MSL_FORMAT_R8G8B8_SNORM,     SIMPLE_FORMAT(0, 3, SNorm, RGB) },
		{ MSL_FORMAT_R8G8B8_USCALED,   SIMPLE_FORMAT(0, 3, UInt,  RGB) },
		{ MSL_FORMAT_R8G8B8_SSCALED,   SIMPLE_FORMAT(0, 3, SInt,  RGB) },
		{ MSL_FORMAT_R8G8B8_UINT,      SIMPLE_FORMAT(0, 3, UInt,  RGB) },
		{ MSL_FORMAT_R8G8B8_SINT,      SIMPLE_FORMAT(0, 3, SInt,  RGB) },
		{ MSL_FORMAT_B8G8R8_UNORM,     SIMPLE_FORMAT(0, 3, UNorm, BGR) },
		{ MSL_FORMAT_B8G8R8_SNORM,     SIMPLE_FORMAT(0, 3, SNorm, BGR) },
		{ MSL_FORMAT_B8G8R8_USCALED,   SIMPLE_FORMAT(0, 3, UInt,  BGR) },
		{ MSL_FORMAT_B8G8R8_SSCALED,   SIMPLE_FORMAT(0, 3, SInt,  BGR) },
		{ MSL_FORMAT_B8G8R8_UINT,      SIMPLE_FORMAT(0, 3, UInt,  BGR) },
		{ MSL_FORMAT_B8G8R8_SINT,      SIMPLE_FORMAT(0, 3, SInt,  BGR) },
		{ MSL_FORMAT_R8G8B8A8_UNORM,   SIMPLE_FORMAT(0, 4, UNorm, RGB) },
		{ MSL_FORMAT_R8G8B8A8_SNORM,   SIMPLE_FORMAT(0, 4, SNorm, RGB) },
		{ MSL_FORMAT_R8G8B8A8_USCALED, SIMPLE_FORMAT(0, 4, UInt,  RGB) },
		{ MSL_FORMAT_R8G8B8A8_SSCALED, SIMPLE_FORMAT(0, 4, SInt,  RGB) },
		{ MSL_FORMAT_R8G8B8A8_UINT,    SIMPLE_FORMAT(0, 4, UInt,  RGB) },
		{ MSL_FORMAT_R8G8B8A8_SINT,    SIMPLE_FORMAT(0, 4, SInt,  RGB) },
		{ MSL_FORMAT_B8G8R8A8_UNORM,   SIMPLE_FORMAT(0, 4, UNorm, BGR) },
		{ MSL_FORMAT_B8G8R8A8_SNORM,   SIMPLE_FORMAT(0, 4, SNorm, BGR) },
		{ MSL_FORMAT_B8G8R8A8_USCALED, SIMPLE_FORMAT(0, 4, UInt,  BGR) },
		{ MSL_FORMAT_B8G8R8A8_SSCALED, SIMPLE_FORMAT(0, 4, SInt,  BGR) },
		{ MSL_FORMAT_B8G8R8A8_UINT,    SIMPLE_FORMAT(0, 4, UInt,  BGR) },
		{ MSL_FORMAT_B8G8R8A8_SINT,    SIMPLE_FORMAT(0, 4, SInt,  BGR) },

		// 16-bit non-packed formats
		{ MSL_FORMAT_R16_UNORM,            SIMPLE_FORMAT(1, 1, UNorm,  RGB) },
		{ MSL_FORMAT_R16_SNORM,            SIMPLE_FORMAT(1, 1, SNorm,  RGB) },
		{ MSL_FORMAT_R16_USCALED,          SIMPLE_FORMAT(1, 1, UInt,   RGB) },
		{ MSL_FORMAT_R16_SSCALED,          SIMPLE_FORMAT(1, 1, SInt,   RGB) },
		{ MSL_FORMAT_R16_UINT,             SIMPLE_FORMAT(1, 1, UInt,   RGB) },
		{ MSL_FORMAT_R16_SINT,             SIMPLE_FORMAT(1, 1, SInt,   RGB) },
		{ MSL_FORMAT_R16_SFLOAT,           SIMPLE_FORMAT(1, 1, SFloat, RGB) },
		{ MSL_FORMAT_R16G16_UNORM,         SIMPLE_FORMAT(1, 2, UNorm,  RGB) },
		{ MSL_FORMAT_R16G16_SNORM,         SIMPLE_FORMAT(1, 2, SNorm,  RGB) },
		{ MSL_FORMAT_R16G16_USCALED,       SIMPLE_FORMAT(1, 2, UInt,   RGB) },
		{ MSL_FORMAT_R16G16_SSCALED,       SIMPLE_FORMAT(1, 2, SInt,   RGB) },
		{ MSL_FORMAT_R16G16_UINT,          SIMPLE_FORMAT(1, 2, UInt,   RGB) },
		{ MSL_FORMAT_R16G16_SINT,          SIMPLE_FORMAT(1, 2, SInt,   RGB) },
		{ MSL_FORMAT_R16G16_SFLOAT,        SIMPLE_FORMAT(1, 2, SFloat, RGB) },
		{ MSL_FORMAT_R16G16B16_UNORM,      SIMPLE_FORMAT(1, 3, UNorm,  RGB) },
		{ MSL_FORMAT_R16G16B16_SNORM,      SIMPLE_FORMAT(1, 3, SNorm,  RGB) },
		{ MSL_FORMAT_R16G16B16_USCALED,    SIMPLE_FORMAT(1, 3, UInt,   RGB) },
		{ MSL_FORMAT_R16G16B16_SSCALED,    SIMPLE_FORMAT(1, 3, SInt,   RGB) },
		{ MSL_FORMAT_R16G16B16_UINT,       SIMPLE_FORMAT(1, 3, UInt,   RGB) },
		{ MSL_FORMAT_R16G16B16_SINT,       SIMPLE_FORMAT(1, 3, SInt,   RGB) },
		{ MSL_FORMAT_R16G16B16_SFLOAT,     SIMPLE_FORMAT(1, 3, SFloat, RGB) },
		{ MSL_FORMAT_R16G16B16A16_UNORM,   SIMPLE_FORMAT(1, 4, UNorm,  RGB) },
		{ MSL_FORMAT_R16G16B16A16_SNORM,   SIMPLE_FORMAT(1, 4, SNorm,  RGB) },
		{ MSL_FORMAT_R16G16B16A16_USCALED, SIMPLE_FORMAT(1, 4, UInt,   RGB) },
		{ MSL_FORMAT_R16G16B16A16_SSCALED, SIMPLE_FORMAT(1, 4, SInt,   RGB) },
		{ MSL_FORMAT_R16G16B16A16_UINT,    SIMPLE_FORMAT(1, 4, UInt,   RGB) },
		{ MSL_FORMAT_R16G16B16A16_SINT,    SIMPLE_FORMAT(1, 4, SInt,   RGB) },
		{ MSL_FORMAT_R16G16B16A16_SFLOAT,  SIMPLE_FORMAT(1, 4, SFloat, RGB) },

		// 32-bit non-packed formats
		{ MSL_FORMAT_R32_UINT,             SIMPLE_FORMAT(2, 1, UInt,   RGB) },
		{ MSL_FORMAT_R32_SINT,             SIMPLE_FORMAT(2, 1, SInt,   RGB) },
		{ MSL_FORMAT_R32_SFLOAT,           SIMPLE_FORMAT(2, 1, SFloat, RGB) },
		{ MSL_FORMAT_R32G32_UINT,          SIMPLE_FORMAT(2, 2, UInt,   RGB) },
		{ MSL_FORMAT_R32G32_SINT,          SIMPLE_FORMAT(2, 2, SInt,   RGB) },
		{ MSL_FORMAT_R32G32_SFLOAT,        SIMPLE_FORMAT(2, 2, SFloat, RGB) },
		{ MSL_FORMAT_R32G32B32_UINT,       SIMPLE_FORMAT(2, 3, UInt,   RGB) },
		{ MSL_FORMAT_R32G32B32_SINT,       SIMPLE_FORMAT(2, 3, SInt,   RGB) },
		{ MSL_FORMAT_R32G32B32_SFLOAT,     SIMPLE_FORMAT(2, 3, SFloat, RGB) },
		{ MSL_FORMAT_R32G32B32A32_UINT,    SIMPLE_FORMAT(2, 4, UInt,   RGB) },
		{ MSL_FORMAT_R32G32B32A32_SINT,    SIMPLE_FORMAT(2, 4, SInt,   RGB) },
		{ MSL_FORMAT_R32G32B32A32_SFLOAT,  SIMPLE_FORMAT(2, 4, SFloat, RGB) },

		// 64-bit non-packed formats
		{ MSL_FORMAT_R64_UINT,             SIMPLE_FORMAT(3, 1, UInt,   RGB) },
		{ MSL_FORMAT_R64_SINT,             SIMPLE_FORMAT(3, 1, SInt,   RGB) },
		{ MSL_FORMAT_R64_SFLOAT,           SIMPLE_FORMAT(3, 1, SFloat, RGB) },
		{ MSL_FORMAT_R64G64_UINT,          SIMPLE_FORMAT(3, 2, UInt,   RGB) },
		{ MSL_FORMAT_R64G64_SINT,          SIMPLE_FORMAT(3, 2, SInt,   RGB) },
		{ MSL_FORMAT_R64G64_SFLOAT,        SIMPLE_FORMAT(3, 2, SFloat, RGB) },
		{ MSL_FORMAT_R64G64B64_UINT,       SIMPLE_FORMAT(3, 3, UInt,   RGB) },
		{ MSL_FORMAT_R64G64B64_SINT,       SIMPLE_FORMAT(3, 3, SInt,   RGB) },
		{ MSL_FORMAT_R64G64B64_SFLOAT,     SIMPLE_FORMAT(3, 3, SFloat, RGB) },
		{ MSL_FORMAT_R64G64B64A64_UINT,    SIMPLE_FORMAT(3, 4, UInt,   RGB) },
		{ MSL_FORMAT_R64G64B64A64_SINT,    SIMPLE_FORMAT(3, 4, SInt,   RGB) },
		{ MSL_FORMAT_R64G64B64A64_SFLOAT,  SIMPLE_FORMAT(3, 4, SFloat, RGB) },
	};

	static constexpr auto s_vertex_info = gen_lookup_table<MSL_FORMAT_E5B9G9R9_UFLOAT_PACK32>(s_vertex_info_entries);

	static constexpr ConstexprPair<MSLFormat, MSLFormatInfo> s_vertex_info_high[] = {
		{ MSL_FORMAT_A4R4G4B4_UNORM_PACK16,     PACKED_FORMAT(1, 4, EvenAHigh, UNorm, BGR) },
		{ MSL_FORMAT_A4B4G4R4_UNORM_PACK16,     PACKED_FORMAT(1, 4, EvenAHigh, UNorm, RGB) },
		{ MSL_FORMAT_A1B5G5R5_UNORM_PACK16_KHR, PACKED_FORMAT(1, 4, AlphaHigh, UNorm, RGB) },
		// Not yet supported: MSL_FORMAT_R16G16_S10_5_NV
		// Not yet supported: MSL_FORMAT_A8_UNORM_KHR
	};

#undef PACKED_FORMAT
#undef SIMPLE_FORMAT
#undef FORMAT
	// clang-format on

	if (static_cast<uint32_t>(format) < s_vertex_info.size())
		return s_vertex_info[format];
	for (const auto &pair : s_vertex_info_high)
	{
		if (pair.first == format)
			return pair.second;
	}
	// Unsupported format
	return s_vertex_info[MSL_FORMAT_UNDEFINED];
}

static uint8_t get_align_log2(uint32_t value, uint8_t max)
{
#ifndef __has_builtin
	#define __has_builtin(x) 0
#endif
#if __has_builtin(__builtin_ctz)
	return static_cast<uint8_t>(__builtin_ctz(value | (1 << max)));
#else
	for (uint8_t i = 0; i < max; i++)
		if (value & (1 << i))
			return i;
	return max;
#endif
}

void CompilerMSL::MSLVertexLoaderWriter::init(const VectorView<MSLVertexAttribute> &in_attributes,
                                              const VectorView<MSLVertexBinding> &in_bindings,
                                              bool dynamic_stride)
{
	memset(attributes, 0, sizeof(attributes));
	memset(bindings, 0, sizeof(bindings));
	for (const MSLVertexAttribute &attr : in_attributes)
	{
		if (attr.location >= MaxAttributes)
			continue;
		attributes[attr.location].binding = attr.binding;
		attributes[attr.location].format = attr.format;
		attributes[attr.location].offset = attr.offset;
	}
	for (const MSLVertexBinding &binding : in_bindings)
	{
		if (binding.binding >= MaxBindings)
			continue;
		uint32_t stride = dynamic_stride ? 0 : binding.stride;
		bindings[binding.binding].stride = stride;
		bindings[binding.binding].struct_size = stride;
		bindings[binding.binding].rate = binding.rate;
		bindings[binding.binding].divisor = binding.divisor;
		bindings[binding.binding].valid = true;
	}
	// Calculate struct alignment
	for (const MSLVertexAttribute &attr : in_attributes)
	{
		if (attr.binding >= MaxBindings)
			continue;
		MSLFormatInfo info = get_format_info(attr.format);
		uint8_t align = static_cast<uint8_t>(info.vk_align_log2());
		if (align > bindings[attr.binding].align)
			bindings[attr.binding].align = align;
	}
}

void CompilerMSL::MSLVertexLoaderWriter::load(const Meta::Decoration &meta, const SPIRType &type)
{
	uint32_t location = meta.location;
	if (location >= MaxAttributes)
		SPIRV_CROSS_THROW("Vertex input location out of range");
	Attribute &attribute = attributes[location];
	if (attribute.format == MSL_FORMAT_UNDEFINED)
		SPIRV_CROSS_THROW("Vertex input references missing attribute");
	MSLFormatInfo info = get_format_info(attribute.format);
	if (!info.is_supported())
		SPIRV_CROSS_THROW("Vertex input uses unsupported type");
	Binding &binding = bindings[attributes[location].binding];
	if (!binding.valid)
		SPIRV_CROSS_THROW("Vertex input references missing binding");
	binding.used = true;
	attribute.meta = &meta;
	attribute.load_type = &type;
	if (binding.stride == 0)
	{
		uint32_t size = info.size();
		if (!info.is_packed)
		{
			// Add space for non-packed type
			uint32_t align_4elem = info.log2_align + 2;
			if (info.num_elems == 3 && get_align_log2(attribute.offset, 5) >= align_4elem)
				size = 1 << align_4elem;
			// Many load functions prefer uints
			if (info.is_normalized || info.is_srgb)
				size = std::max<uint32_t>(4, size);
		}
		binding.struct_size = std::max(binding.struct_size, attribute.offset + size);
	}
}

CompilerMSL::SPVFuncImpl CompilerMSL::MSLVertexLoaderWriter::get_function_for_loading_vertex(uint32_t attribute, bool has_pixel_type_loads)
{
	assert(attributes[attribute].load_type);
	MSLFormat format = attributes[attribute].format;
	const SPIRType &type = *attributes[attribute].load_type;
	const MSLFormatInfo &info = get_format_info(format);
	if (info.is_packed)
	{
		switch (info.log2_align)
		{
		case 0:
			assert(format == MSL_FORMAT_R4G4_UNORM_PACK8);
			return SPVFuncImplLoadVertexRG4;
		case 1:
			assert(!info.is_float);
			assert(!info.is_signed);
			// clang-format off
			switch (info.packing)
			{
			case MSLFormatPacking::Invalid:   assert(0);
			case MSLFormatPacking::NoAlpha:   return SPVFuncImplNone; // Metal has this one
			case MSLFormatPacking::AlphaLow:  return SPVFuncImplLoadVertexRGB5A1BE;
			case MSLFormatPacking::AlphaHigh: return SPVFuncImplLoadVertexRGB5A1LE;
			case MSLFormatPacking::EvenALow:  return SPVFuncImplLoadVertexRGBA4BE;
			case MSLFormatPacking::EvenAHigh: return SPVFuncImplLoadVertexRGBA4LE;
			}
			// clang-format on
			break;
		case 2:
			if (info.is_float)
			{
				if (has_pixel_type_loads)
					return SPVFuncImplNone; // Metal has these
				if (format == MSL_FORMAT_E5B9G9R9_UFLOAT_PACK32)
					return type.basetype == SPIRType::Half ? SPVFuncImplLoadVertexRGB9E5Half :
					                                         SPVFuncImplLoadVertexRGB9E5Float;
				if (format == MSL_FORMAT_B10G11R11_UFLOAT_PACK32)
					return SPVFuncImplLoadVertexRG11B10Half;
				assert(0);
			}
			else
			{
				assert(info.packing == MSLFormatPacking::AlphaHigh);
				if (info.is_normalized && !info.is_signed)
					return SPVFuncImplNone; // Metal has this one
				return info.is_signed ? SPVFuncImplLoadVertexRGB10A2SInt : SPVFuncImplLoadVertexRGB10A2UInt;
			}
			break;
		default:
			assert(0);
			break;
		}
	}
	return SPVFuncImplNone;
}

class CompilerMSL::MSLVertexLoaderWriter::Impl
{
	// Goal: Generate nice looking, useful structs
	// The closer the struct is to the vertex format, the more readable it is in the Xcode GPU debugger, which makes our life easier later
	// Each vertex binding will be converted to a struct, either a nice one like
	//     struct { packed_float2 something; packed_float2 something_else; };
	// or a less nice one if attributes reach past the stride of the struct, like
	//     struct { uint data[4]; };
	// To do this, we first go through all the attributes, and mark the bytes they cover in BindingInfo::usage.
	// Attributes that don't overlap will mark their preferred type down along with the source attribute,
	// while overlaps will mark without the source attribute, and possibly compromise on the type (with uints used if no one can agree)
	// Then, we go through the usage info and generate struct definitions for each used binding based off the usage info
	// Finally we use both the usage info and the attribute definitions to generate the vertex loader functions,
	// using either reinterpret_cast or as_type casts where needed.

	/// The metal type to use in the vertex buffer struct
	struct Storage
	{
		enum : uint8_t
		{
			PIXEL = 0x80,
			FLOAT = 0x40,
			SIGNED = 0x20,
		};
		enum PixelFormat : uint8_t
		{
			RGBA8Unorm,
			RGBA8Snorm,
			RGBA16Unorm,
			RGBA16Snorm,
			RGB10A2,
			RG11B10F,
			RGB9E5,
			SRGBA8Unorm,
		};

		uint8_t value;

		static Storage Simple(uint32_t elements, uint32_t log2_size, bool store_float, bool store_signed)
		{
			assert(elements >= 1 && elements <= 4);
			uint8_t value = static_cast<uint8_t>(elements - 1);
			value |= log2_size << 2;
			if (store_float)
				value |= FLOAT;
			else if (store_signed)
				value |= SIGNED;
			return { value };
		}

		static Storage Packed(uint32_t elements, PixelFormat format, bool load_float)
		{
			assert(elements >= 1 && elements <= 4);
			uint8_t value = PIXEL;
			value |= static_cast<uint8_t>(elements - 1);
			value |= format << 2;
			if (load_float)
				value |= FLOAT;
			return { value };
		}

		static Storage FromMSLFormat(const MSLFormatInfo &info, bool allow_pixel_types, bool load_float)
		{
			if (allow_pixel_types)
			{
				if (info.is_packed)
				{
					MSLFormatInfo no_bgr = info;
					no_bgr.is_bgr = false;
					if (no_bgr == get_format_info(MSL_FORMAT_A2B10G10R10_UNORM_PACK32))
						return Packed(1, RGB10A2, load_float);
					if (no_bgr == get_format_info(MSL_FORMAT_B10G11R11_UFLOAT_PACK32))
						return Packed(1, RG11B10F, load_float);
					if (no_bgr == get_format_info(MSL_FORMAT_E5B9G9R9_UFLOAT_PACK32))
						return Packed(1, RGB9E5, load_float);
				}
				else if (info.is_srgb)
				{
					assert(info.log2_align == 0);
					return Packed(info.num_elems, SRGBA8Unorm, load_float);
				}
				else if (info.is_normalized)
				{
					assert(info.log2_align == 0 || info.log2_align == 1);
					PixelFormat fmt = static_cast<PixelFormat>((info.log2_align << 1) | info.is_signed);
					static_assert(RGBA8Unorm  == 0, "Above calculation");
					static_assert(RGBA8Snorm  == 1, "Above calculation");
					static_assert(RGBA16Unorm == 2, "Above calculation");
					static_assert(RGBA16Snorm == 3, "Above calculation");
					return Packed(info.num_elems, fmt, load_float);
				}
			}
			uint32_t elems = info.is_packed ? 1 : info.num_elems;
			bool is_float = info.is_float & !info.is_packed;
			return Simple(elems, info.log2_align, is_float, info.is_signed);
		}

		static Storage FromSPIRBaseType(const SPIRType::BaseType type)
		{
			// clang-format off
			switch (type)
			{
			case SPIRType::SByte:  return Simple(1, 0, false, true);
			case SPIRType::UByte:  return Simple(1, 0, false, false);
			case SPIRType::Short:  return Simple(1, 1, false, true);
			case SPIRType::UShort: return Simple(1, 1, false, false);
			case SPIRType::Int:    return Simple(1, 2, false, true);
			case SPIRType::UInt:   return Simple(1, 2, false, false);
			case SPIRType::Int64:  return Simple(1, 3, false, true);
			case SPIRType::UInt64: return Simple(1, 3, false, false);
			case SPIRType::Half:   return Simple(1, 1, true,  false);
			case SPIRType::Float:  return Simple(1, 2, true,  false);
			case SPIRType::Double: return Simple(1, 3, true,  false);
			default:
				assert(0);
				return Simple(1, 0, false, false);
			}
			// clang-format on
		}

		static Storage FromSPIRType(const SPIRType *type)
		{
			assert(type->vecsize <= 4);
			uint8_t value = FromSPIRBaseType(type->basetype).value;
			value |= std::min<uint32_t>(type->vecsize - 1, 3);
			return { value };
		}

		static Storage Fill(uint32_t log2_size)
		{
			assert(log2_size <= 4);
			uint32_t elements = 1;
			if (log2_size > 2)
			{
				elements = 1 << (log2_size - 2);
				log2_size = 2;
			}
			return Simple(elements, log2_size, false, false);
		}

		bool operator==(Storage other) const { return other.value == value; }
		bool operator!=(Storage other) const { return other.value != value; }
		bool is_pixel_format() const { return value & PIXEL; }
		bool is_float() const { return value & FLOAT; }
		bool is_signed() const
		{
			assert(!is_pixel_format());
			return value & SIGNED;
		}
		PixelFormat pixel_format() const
		{
			assert(is_pixel_format());
			return static_cast<PixelFormat>((value >> 2) & 0xf);
		}
		uint32_t log2_elem_size() const
		{
			if (value & PIXEL)
			{
				switch (pixel_format())
				{
				case RGBA8Snorm:
				case RGBA8Unorm:
				case SRGBA8Unorm:
					return 0;
				case RGBA16Snorm:
				case RGBA16Unorm:
					return 1;
				case RGB10A2:
				case RG11B10F:
				case RGB9E5:
					return 2;
				}
				assert(0);
				return 0;
			}
			else
			{
				return (value >> 2) & 3;
			}
		}
		uint32_t non_packed_align() const
		{
			uint32_t align = log2_elem_size();
			uint32_t elems = elements();
			if (!is_pixel_format() && elems > 1)
				align += elems == 2 ? 1 : 2;
			return align;
		}
		uint32_t elements() const { return (value & 3) + 1; }
		uint32_t load_elements() const
		{
			if (value & PIXEL)
			{
				switch (pixel_format())
				{
				case RGB10A2:
					return 4;
				case RG11B10F:
				case RGB9E5:
					return 3;
				default:
					break;
				}
			}
			return elements();
		}
		uint32_t size() const { return elements() * (1 << log2_elem_size()); }
		/// Are the two types the same aside from their element counts?
		bool types_match(Storage other) const { return (value & ~3) == (other.value & ~3); }
		/// Pick the best storage for a space shared by two types, where `this->size() > other.size()`
		Storage make_compatible(Storage other) const
		{
			uint32_t elems = elements();
			if (types_match(other))
				return *this;
			if (is_pixel_format() && other.is_pixel_format())
			{
				uint8_t ret = value;
				PixelFormat format = pixel_format();
				PixelFormat other_format = other.pixel_format();
				if ((value & FLOAT) != (other.value & FLOAT))
					ret |= FLOAT;
				if (format == other_format)
					return { ret };
				if (format != other_format)
				{
					switch (other_format)
					{
					case RGBA8Snorm:
					case RGBA8Unorm:
					case SRGBA8Unorm:
						if (format == RGBA8Snorm || format == RGBA8Unorm || format == SRGBA8Unorm)
							return Packed(elems, RGBA8Unorm, ret & FLOAT);
						break;
					case RGBA16Snorm:
					case RGBA16Unorm:
						if (format == RGBA8Snorm || format == RGBA8Unorm)
							return Packed(elems, RGBA8Unorm, ret & FLOAT);
						break;
					case RGB10A2:
					case RG11B10F:
					case RGB9E5:
						break;
					}
				}
			}
			// Force uint
			if (is_pixel_format())
			{
				return Simple(elems, log2_elem_size(), false, false);
			}
			else
			{
				uint8_t ret = value;
				ret &= ~(FLOAT | SIGNED);
				return { ret };
			}
		}
		Storage adjust_element_count(uint32_t new_count)
		{
			assert(new_count >= 1 && new_count <= 4);
			uint8_t new_value = value & ~3;
			new_value |= new_count - 1;
			return { new_value };
		}
		/// Can you use a simple cast (e.g. `uint3(int3_val)` between the two types?)
		bool can_cast_from(Storage other) const
		{
			uint8_t my_value = value;
			if ((my_value & PIXEL) != (other.value & PIXEL))
				return false;
			if (my_value & PIXEL)
			{
				my_value &= ~3;
				other.value &= ~3;
			}
			else
			{
				my_value &= FLOAT | 0xc;
				other.value &= FLOAT | 0xc;
			}
			return my_value == other.value;
		}

	private:
		// clang-format off
		static const char* _msl_name(PixelFormat format)
		{
			switch (format)
			{
			case RGBA8Unorm:  return "8unorm";
			case RGBA8Snorm:  return "8snorm";
			case RGBA16Unorm: return "16unorm";
			case RGBA16Snorm: return "16snorm";
			case RGB10A2:     return "rgb10a2";
			case RG11B10F:    return "rg11b10f";
			case RGB9E5:      return "rgb9e5";
			case SRGBA8Unorm: return "srgba8unorm";
			}
			assert(0);
			return "unknown_pixel_format";
		}
		static const char* _int_name(uint32_t log2_size)
		{
			switch (log2_size)
			{
			case 0: return "char";
			case 1: return "short";
			case 2: return "int";
			case 3: return "long";
			default: assert(0); return "too_long";
			}
		}
		static const char* _float_name(uint32_t log2_size)
		{
			switch (log2_size)
			{
			case 0: assert(0); return "too_short";
			case 1: return "half";
			case 2: return "float";
			case 3: return "double";
			default: assert(0); return "too_long";
			}
		}
		// clang-format on
	public:
		std::string msl_name() const
		{
			std::string res;
			uint32_t elems = load_elements();
			if (value & PIXEL)
			{
				PixelFormat format = pixel_format();
				if (format == RGBA8Unorm || format == RGBA8Snorm || format == RGBA16Unorm || format == RGBA16Snorm)
					res.append("rgba", elems);
				res.append(_msl_name(format));
				res.push_back('<');
				res.append(value & FLOAT ? "float" : "half");
				if (elems > 1)
					res.push_back(static_cast<char>('0' + elems));
				res.push_back('>');
			}
			else
			{
				uint32_t size = log2_elem_size();
				if (value & FLOAT)
				{
					res.append(_float_name(size));
				}
				else
				{
					if (!(value & SIGNED))
						res.push_back('u');
					res.append(_int_name(size));
				}
				if (elems > 1)
					res.push_back(static_cast<char>('0' + elems));
			}
			return res;
		}
		void write_cast(std::string &target) const
		{
			std::string name = msl_name();
			name.push_back('(');
			target.insert(0, name);
			target.push_back(')');
		}
	};
	struct ByteUsage
	{
		bool used;              ///< Whether this byte is being used by a vertex attribute
		Storage storage;        ///< What MSL type is being stored here
		uint8_t offset;         ///< The offset from the beginning of the MSL type being stored here
		uint8_t name_attribute; ///< If there's only
		enum : uint8_t { NAME_NONE = UINT8_MAX };
	};
	struct BindingInfo
	{
		struct FreeDelete { void operator()(void* arr) const { free(arr); } };
		uint8_t base_align;       ///< Maximum align supported (based on stride)
		uint8_t min_load_align;   ///< Alignment of the least aligned load that needs to happen on the binding
		uint8_t max_needed_align; ///< Most alignment that would be useful to the binding
		bool read_past_stride;    ///< Whether any attributes go past the stride of the binding
		std::unique_ptr<ByteUsage[], FreeDelete> usage; ///< Tracks which bytes of the binding are used for what types
		/// Checks that nothing is broken (for use in asserts)
		bool is_valid(uint32_t length) const
		{
			ByteUsage current;
			uint32_t remaining = 0;
			for (uint32_t i = 0; i < length; i++)
			{
				const ByteUsage &next = usage[i];
				if (remaining > 0)
				{
					current.offset++;
					remaining--;
					if (!next.used || next.storage != current.storage || next.offset != current.offset ||
					    next.name_attribute != current.name_attribute)
					{
						return false;
					}
				}
				else if (next.used)
				{
					if (next.offset != 0)
						return false;
					current = next;
					remaining = current.storage.size() - 1;
				}
			}
			return true;
		}
		/// Should/will the given element be packed?  (Returns true for types that are always packed, and false for single-element types)
		bool should_pack_element(uint32_t index, uint32_t end, uint32_t max_align) const
		{
			Storage storage = usage[index].storage;
			uint32_t elems = storage.elements();
			if (elems <= 1)
				return false;
			if (storage.is_pixel_format())
				return true;
			max_align = std::min<uint32_t>(max_align, get_align_log2(index, 5));
			uint32_t size = storage.log2_elem_size();
			if (elems == 2 && size + 1 <= max_align)
				return false;
			if (elems == 4 && size + 2 <= max_align)
				return false;
			if (elems == 3 && size + 2 <= max_align)
			{
				uint32_t new_end = index + 4 * (1 << size);
				if (new_end > end)
					return true;
				return is_range_used(index + 3 * (1 << size), new_end);
			}
			return true;
		}
		/// Are any of the bytes in the given range used by anything?
		bool is_range_used(uint32_t begin, uint32_t end) const
		{
			for (uint32_t i = begin; i < end; i++)
				if (usage[i].used)
					return true;
			return false;
		}
		/// Set usage values for the full size of `storage`
		void set(uint32_t index, Storage storage, uint8_t name_attribute)
		{
			uint32_t size = storage.size();
			for (uint32_t i = 0; i < size; i++)
			{
				ByteUsage &byte = usage[index + i];
				byte.used = true;
				byte.storage = storage;
				byte.offset = static_cast<uint8_t>(i);
				byte.name_attribute = name_attribute;
			}
		}
		/// Fills the area with some uint values
		void generic_fill(uint32_t index, uint32_t length, uint32_t max_align)
		{
			while (length > 0)
			{
				for (uint32_t i = 1; i < length; i++)
				{
					if (usage[index + i].used && usage[index + i].offset == 0)
					{
						// Preserve element start
						length = i;
						break;
					}
				}
				uint32_t align = std::min<uint32_t>(get_align_log2(index, 5), max_align);
				uint32_t size;
				while ((size = 1 << align) > length)
					align--;
				Storage storage = Storage::Fill(align);
				assert(storage.size() == size);
				set(index, storage, ByteUsage::NAME_NONE);
				index += size;
				length -= size;
			}
		}
		/// Fixes up the area after a `generic_fill` in case you partially trampled an existing entry
		void fixup_fill(uint32_t index, uint32_t end, uint32_t max_align)
		{
			if (index >= end)
				return;
			const ByteUsage &byte = usage[index];
			if (!byte.used || byte.offset == 0)
				return;
			generic_fill(index, byte.storage.size() - byte.offset, max_align);
		}
	};

	bool pixel_types_in_storage; ///< Should MSL pixel types (e.g. `rgba8unorm<float4>`) be used in struct definitions?
	bool pixel_type_loads;       ///< Should MSL pixel types be used for loading values?
	Binding (&bindings)[MaxBindings];
	Attribute (&attributes)[MaxAttributes];
	CompilerMSL &out;
	BindingInfo info_list[MaxBindings];

public:
	Impl(MSLVertexLoaderWriter &writer, CompilerMSL &out_)
	    : bindings(writer.bindings)
	    , attributes(writer.attributes)
	    , out(out_)
	{
		// Only apple GPUs support loading from pixel structs (e.g. rgba8unorm<float4>)
		// But use them in structs anyways, to improve decoding when viewed in Xcode GPU captures
		// As a bonus, they're one of the few types AMD's horribly fussy backend compiler will convert unorm unpacks to tbuffer loads with
		// (e.g. unpack_unorm4x8_to_float(reinterpret_cast<const device uint&>(x)) will generate
		//  tbuffer_load_format_xyzw vOut, format:[BUF_FORMAT_8_8_8_8_UNORM] if x is uint or rgba8unorm<float4>, but will generate
		//  buffer_load_dword vTmp; vOut = <float4(vTmp) / 255>; if x is uchar4 or uint2)
		const uint32_t pixel_type_cutoff = out.msl_options.make_msl_version(2, out.msl_options.is_ios() ? 0 : 3);
		pixel_types_in_storage = out.msl_options.msl_version >= pixel_type_cutoff;
		pixel_type_loads = pixel_types_in_storage && out.msl_options.use_pixel_type_loads;
		// Initialize binding info
		for (uint32_t i = 0; i < MaxBindings; i++)
		{
			if (!bindings[i].used)
				continue;
			info_list[i].usage.reset(static_cast<ByteUsage *>(std::calloc(bindings[i].struct_size, sizeof(ByteUsage))));
			info_list[i].base_align = std::min(bindings[i].align, get_align_log2(bindings[i].stride, 5));
			info_list[i].min_load_align = info_list[i].base_align;
			info_list[i].max_needed_align = 0;
			info_list[i].read_past_stride = false;
		}
		// Vulkan allows unaligned attribute offsets as long as the vertex buffer offset is also unaligned such that they're aligned when added
		// If this is the case, we need to reduce the alignment of the struct
		for (uint32_t i = 0; i < MaxAttributes; i++)
		{
			if (!attributes[i].load_type)
				continue;
			const MSLFormatInfo &format = get_format_info(attributes[i].format);
			uint8_t offset_align = get_align_log2(attributes[i].offset, 5);
			BindingInfo &info = info_list[attributes[i].binding];
			if (format.log2_align > offset_align)
				info.base_align = info.min_load_align = std::min(info.base_align, offset_align);
		}
	}

	void decide_attribute_storage()
	{
		for (uint32_t i = 0; i < MaxAttributes; i++)
		{
			Attribute &attribute = attributes[i];
			if (!attribute.load_type)
				continue;
			Binding &binding = bindings[attribute.binding];
			BindingInfo &info = info_list[attribute.binding];
			MSLFormatInfo format = get_format_info(attribute.format);
			uint32_t size = format.size();
			uint32_t min_align = format.log2_align;
			uint32_t max_align = min_align;
			if (!format.is_packed)
			{
				max_align = min_align + (format.num_elems <= 1 ? 0 : format.num_elems == 2 ? 1 : 2);
				if (format.is_normalized || format.is_srgb)
				{
					if (pixel_type_loads)
						max_align = min_align; // MSL pixel-typed loads don't need any additional alignment
					else if (max_align < 2)
						max_align = 2; // MSL pixel unpack functions work with uints
				}
			}
			uint8_t base_align = info.base_align;
			uint8_t load_align = std::min(get_align_log2(attribute.offset, 5), base_align);
			uint8_t needed_align = std::min(load_align, static_cast<uint8_t>(max_align));
			info.max_needed_align = std::max(needed_align, info.max_needed_align);
			info.min_load_align = std::min<uint8_t>(load_align, info.min_load_align);
			if (attribute.offset + size > binding.struct_size)
				info.read_past_stride = true;
			if (min_align > load_align)
				// Load is unaligned!  This will mess up fancy struct generation and break everything
				info.read_past_stride = true;
			if (info.read_past_stride)
				continue; // read-past-end structs aren't as fancy
			Storage storage = Storage::FromMSLFormat(format, pixel_types_in_storage, attribute.load_type->basetype != SPIRType::Half);
			assert(storage.size() == size);
			if (info.usage[attribute.offset].used && info.usage[attribute.offset].offset != 0)
			{
				// Overlaps with the middle of an existing entry
				const ByteUsage &other = info.usage[attribute.offset];
				uint32_t begin = attribute.offset - other.offset;
				uint32_t end = std::max(begin + other.storage.size(), attribute.offset + size);
				info.generic_fill(begin, other.offset, base_align);
				info.generic_fill(attribute.offset, end - attribute.offset, base_align);
				info.fixup_fill(end, binding.struct_size, base_align);
			}
			else
			{
				bool all_clear = true;
				for (uint32_t j = 0; j < size; j++)
				{
					const ByteUsage &other = info.usage[attribute.offset + j];
					if (other.used && other.offset != j)
					{
						// Middle overlaps with an existing entry
						all_clear = false;
						info.generic_fill(attribute.offset, j, base_align);
						info.generic_fill(attribute.offset + j, size - j, base_align);
						info.fixup_fill(attribute.offset + size, binding.struct_size, base_align);
						break;
					}
				}
				if (all_clear)
				{
					if (info.usage[attribute.offset].used)
					{
						// Existing entry starts at the same spot, attempt to share storage
						const ByteUsage &other = info.usage[attribute.offset];
						assert(other.offset == 0 && "Other cases should have been handled already");
						Storage other_storage = other.storage;
						uint32_t other_size = other_storage.size();
						Storage larger_storage = other_size > size ? other_storage : storage;
						Storage smaller_storage = other_size > size ? storage : other_storage;
						info.set(attribute.offset, larger_storage.make_compatible(smaller_storage), ByteUsage::NAME_NONE);
					}
					else
					{
						// No overlap
						info.set(attribute.offset, storage, static_cast<uint8_t>(i));
					}
				}
			}
			assert(info.is_valid(binding.struct_size));
		}
	}

	void generate_vertex_data_structs()
	{
		for (uint32_t i = 0; i < MaxBindings; i++)
		{
			Binding &binding = bindings[i];
			BindingInfo &info = info_list[i];
			uint32_t struct_size = binding.struct_size;
			if (!binding.used)
				continue;
			uint32_t max_storage_alignment = 0;
			if (info.read_past_stride)
			{
				max_storage_alignment = info.min_load_align;
			}
			else
			{
				uint32_t offset = 0;
				while (offset < struct_size)
				{
					uint32_t size = 1;
					if (info.usage[offset].used)
					{
						assert(info.usage[offset].offset == 0);
						Storage storage = info.usage[offset].storage;
						size = storage.size();
						uint32_t align = storage.log2_elem_size();
						uint32_t elems = storage.elements();
						if (elems > 1 && !info.should_pack_element(offset, struct_size, info.base_align))
							align += elems == 2 ? 1 : 2;
						max_storage_alignment = std::max(align, max_storage_alignment);
					}
					offset += size;
				}
			}

			std::string align_str;
			if (max_storage_alignment != info.base_align)
			{
				assert(max_storage_alignment <= info.base_align);
				align_str.append(" alignas(");
				align_str.append(std::to_string(1 << info.base_align));
				align_str.push_back(')');
			}
			out.statement("struct", align_str, " spvVertexData", std::to_string(i));
			out.begin_scope();

			if (info.read_past_stride)
			{
				Storage storage = Storage::Fill(max_storage_alignment);
				out.statement(storage.msl_name(), " data[", std::to_string(struct_size / (1 << max_storage_alignment)), "];");
			}
			else
			{
				uint32_t offset = 0;
				while (offset < struct_size)
				{
					uint32_t size = 1;
					if (info.usage[offset].used)
					{
						assert(info.usage[offset].offset == 0);
						Storage storage = info.usage[offset].storage;
						// The following Storage types aren't supported in Metal:
						// - 3-element normalized pixel types (e.g. rgb8unorm)
						// - SRGB pixel types other than srgba8unorm
						// Either grow them to 4 elements if there's space, or replace with a supported type
						if (storage.is_pixel_format())
						{
							uint32_t elems = storage.elements();
							Storage::PixelFormat format = storage.pixel_format();
							if (elems == 3 || (format == Storage::SRGBA8Unorm && elems < 4))
							{
								uint32_t elem_size = storage.log2_elem_size();
								uint32_t old_end = (1 << elem_size) * elems;
								uint32_t new_end = (1 << elem_size) * 4;
								if (offset + new_end > struct_size || info.is_range_used(offset + old_end, offset + new_end))
									storage = Storage::Simple(elems, elem_size, false, false);
								else
									storage = storage.adjust_element_count(4);
								info.set(offset, storage, info.usage[offset].name_attribute);
							}
						}
						bool should_pack = info.should_pack_element(offset, struct_size, info.base_align);
						const char *pack = !storage.is_pixel_format() && should_pack ? "packed_" : "";
						std::string name;
						if (info.usage[offset].name_attribute == ByteUsage::NAME_NONE)
							name = "spvData" + std::to_string(offset);
						else
							name = attributes[info.usage[offset].name_attribute].meta->alias;
						out.statement(pack, storage.msl_name(), " ", name, ";");
						uint32_t size_in_elements = storage.elements();
						if (size_in_elements == 3 && !should_pack)
							size_in_elements = 4;
						size = size_in_elements * (1 << storage.log2_elem_size());
					}
					else
					{
						while (offset + size < struct_size && !info.usage[offset + size].used)
							size++;
						uint32_t align = std::min<uint32_t>(info.base_align, std::min(get_align_log2(offset, 2), get_align_log2(size, 2)));
						Storage storage = Storage::Simple(1, align, false, false);
						uint32_t count = size >> align;
						if (count == 1)
							out.statement(storage.msl_name(), " spvPad", std::to_string(offset), ";");
						else
							out.statement(storage.msl_name(), " spvPad", std::to_string(offset), "[", std::to_string(count), "];");
					}
					offset += size;
				}
				assert(info.is_valid(struct_size));
			}

			out.end_scope_decl();
			out.statement("static_assert(alignof(spvVertexData", std::to_string(i), ") == ", std::to_string(1 << info.base_align), ", \"Unexpected alignment\");");
			if (binding.stride != 0)
				out.statement("static_assert(sizeof(spvVertexData", std::to_string(i), ") == ", std::to_string(struct_size), ", \"Unexpected size\");");
			out.statement("");
		}
	}

	void generate_loader(TypeID main_struct)
	{
		std::string tmp_string, line; // Declare tmps early to reduce allocation count
		for (uint32_t i = 0; i < MaxBindings; i++)
		{
			if (!bindings[i].used)
				continue;
			if (!tmp_string.empty())
				tmp_string.append(", ");
			tmp_string.append("const device spvVertexData");
			tmp_string.append(std::to_string(i));
			tmp_string.append("& data");
			tmp_string.append(std::to_string(i));
		}
		out.statement(out.get_name(main_struct), " spvLoadVertex(", tmp_string, ")");
		out.begin_scope();
		out.statement(out.get_name(main_struct), " out;");
		for (uint32_t i = 0; i < MaxAttributes; i++)
		{
			if (!attributes[i].load_type)
				continue;
			// Loading a vertex can require the following steps:
			// 1. Reinterpret cast to an approprate type to load
			// 2. as_type cast to adjust type post-load (load type may be different to give the compiler better alignment info)
			// 3. Running a conversion function like spvLoadVertexRGBA4LE
			// 4. Normalizing values
			// 5. Casting to the final shader type
			// 6. Removing or adding elements to match the shader type
			const Attribute &attribute = attributes[i];
			const Binding &binding = bindings[attribute.binding];
			const BindingInfo &info = info_list[attribute.binding];
			MSLFormatInfo format = get_format_info(attribute.format);
			uint32_t align = std::min<uint32_t>(info.base_align, get_align_log2(attribute.offset, 5));
			bool wants_half = attribute.load_type->basetype == SPIRType::Half;
			Storage stored_type;
			Storage convert_out_type = Storage::FromMSLFormat(format, pixel_type_loads, !wants_half);
			Storage convert_in_type = convert_out_type;
			Storage load_type = convert_in_type;
			const char *convert_function = nullptr;
			const char *normalize_value = nullptr;
			bool normalize_clamp = false;
			bool stored_packed = false;
			bool load_packed = false;
			// Get stored type
			if (info.read_past_stride)
			{
				stored_type = Storage::Fill(info.min_load_align);
			}
			else
			{
				stored_type = info.usage[attribute.offset].storage;
				stored_packed = info.should_pack_element(attribute.offset, binding.struct_size, info.base_align);
			}
			// Pick conversion functions, normalization, load_type, and convert_in/out_type
			if (convert_in_type.is_pixel_format())
			{
				stored_packed = false;
				uint32_t elems = load_type.elements();
				Storage::PixelFormat px_fmt = load_type.pixel_format();
				if (elems == 3 || (px_fmt == Storage::SRGBA8Unorm && elems < 4))
					load_type = load_type.adjust_element_count(4); // Legalize (see comment in generate_vertex_structs)
				if (stored_type.types_match(load_type) && stored_type.elements() > load_type.elements())
					load_type = stored_type; // Less reinterpret_cast, more `.rgb`
				convert_in_type = load_type;
				convert_out_type = Storage::Simple(load_type.load_elements(), wants_half ? 1 : 2, true, false);
			}
			else
			{
				if (format.is_packed)
				{
					convert_out_type = Storage::Simple(format.num_elems, wants_half ? 1 : 2, true, false);
					convert_in_type = Storage::Simple(1, format.log2_align, false, false);
					switch (format.log2_align)
					{
					case 0:
						assert(attribute.format == MSL_FORMAT_R4G4_UNORM_PACK8);
						convert_function = "spvLoadVertexRG4";
						break;
					case 1:
						assert(!format.is_float);
						assert(!format.is_signed);
						convert_out_type = Storage::Simple(format.num_elems, wants_half ? 1 : 2, true, false);
						switch (format.packing)
						{
						case MSLFormatPacking::Invalid:
							assert(0);
							break;
						case MSLFormatPacking::EvenAHigh:
							convert_function = "spvLoadVertexRGBA4LE";
							normalize_value = "15";
							break;
						case MSLFormatPacking::EvenALow:
							convert_function = "spvLoadVertexRGBA4BE";
							normalize_value = "15";
							break;
						case MSLFormatPacking::NoAlpha:
							convert_function = wants_half ? "unpack_unorm565_to_half" : "unpack_unorm565_to_float";
							break;
						case MSLFormatPacking::AlphaHigh:
							convert_function = "spvLoadVertexRGB5A1BE";
							normalize_value = wants_half ? "half4(31, 31, 31, 1)" : "float4(31, 31, 31, 1)";
							break;
						case MSLFormatPacking::AlphaLow:
							convert_function = "spvLoadVertexRGB5A1LE";
							normalize_value = wants_half ? "half4(31, 31, 31, 1)" : "float4(31, 31, 31, 1)";
							break;
						}
						break;
					case 2:
						if (format.is_float)
						{
							switch (attribute.format)
							{
							case MSL_FORMAT_E5B9G9R9_UFLOAT_PACK32:
								convert_function = wants_half ? "spvLoadVertexRGB9E5Half" : "spvLoadVertexRGB9E5Float";
								break;
							case MSL_FORMAT_B10G11R11_UFLOAT_PACK32:
								convert_function = "spvLoadVertexRG11B10Half";
								convert_out_type = Storage::Simple(format.num_elems, 1, true, false);
								break;
							default:
								assert(0);
								break;
							}
						}
						else
						{
							assert(format.packing == MSLFormatPacking::AlphaHigh);
							if (format.is_signed)
							{
								convert_function = "spvLoadVertexRGB10A2SInt";
								if (format.is_normalized)
									normalize_value = wants_half ? "half4(511, 511, 511, 1)" : "float4(511, 511, 511, 1)";
								else
									convert_out_type = Storage::Simple(4, 2, false, true);
							}
							else if (format.is_normalized)
							{
								convert_function = wants_half ? "unpack_unorm10a2_to_half" : "unpack_unorm10a2_to_float";
							}
							else
							{
								convert_function = "spvLoadVertexRGB10A2UInt";
								convert_out_type = Storage::Simple(4, 2, false, false);
							}
						}
						break;
					default:
						assert(0);
						break;
					}
				}
				else if (format.is_srgb)
				{
					assert(format.log2_align == 0);
					convert_out_type = Storage::Simple(4, wants_half ? 1 : 2, true, false);
					convert_in_type = Storage::Simple(1, 2, false, false);
					convert_function = wants_half ? "unpack_unorm4x8_srgb_to_half" : "unpack_unorm4x8_srgb_to_float";
				}
				else if (format.is_normalized)
				{
					if (format.log2_align == 0)
					{
						if (align >= 2)
						{
							convert_out_type = Storage::Simple(4, wants_half ? 1 : 2, true, false);
							convert_in_type = Storage::Simple(1, 2, false, false);
							if (format.is_signed)
								convert_function = wants_half ? "unpack_snorm4x8_to_half" : "unpack_snorm4x8_to_float";
							else
								convert_function = wants_half ? "unpack_unorm4x8_to_half" : "unpack_unorm4x8_to_float";
						}
						else
						{
							convert_out_type = Storage::Simple(format.num_elems, wants_half ? 1 : 2, true, false);
							convert_in_type = Storage::Simple(format.num_elems, 0, false, format.is_signed);
							if (format.is_signed)
							{
								normalize_clamp = true;
								normalize_value = "127";
							}
							else
							{
								normalize_value = "255";
							}
						}
					}
					else
					{
						assert(format.log2_align == 1);
						if (format.num_elems <= 2 && align >= 2)
						{
							convert_out_type = Storage::Simple(2, wants_half ? 1 : 2, true, false);
							convert_in_type = Storage::Simple(1, 2, false, false);
							if (format.is_signed)
								convert_function = wants_half ? "unpack_snorm2x16_to_half" : "unpack_snorm2x16_to_float";
							else
								convert_function = wants_half ? "unpack_unorm2x16_to_half" : "unpack_unorm2x16_to_float";
						}
						else
						{
							// Force float as half doesn't have enough precision for the normalization
							convert_out_type = Storage::Simple(format.num_elems, 2, true, false);
							convert_in_type = Storage::Simple(format.num_elems, 1, false, format.is_signed);
							if (format.is_signed)
							{
								normalize_clamp = true;
								normalize_value = "32767";
							}
							else
							{
								normalize_value = "65535";
							}
						}
					}
				}

				if (convert_in_type.log2_elem_size() <= align)
					load_type = convert_in_type; // Prefer convert_in_type as long as the load is sufficiently aligned
				uint32_t stored_align = stored_packed ? stored_type.log2_elem_size() : stored_type.non_packed_align();
				uint32_t wanted_align = std::min<uint32_t>(load_type.non_packed_align(), align);
				bool big_enough = stored_type.size() >= load_type.size();
				bool not_too_big = stored_type.log2_elem_size() <= load_type.log2_elem_size() + 2;
				if (!stored_type.is_pixel_format() && stored_align >= wanted_align && big_enough && not_too_big)
				{
					load_type = stored_type; // Less reinterpret casts
					load_packed = stored_packed;
				}
				else
				{
					uint32_t elems = load_type.elements();
					uint32_t size = load_type.log2_elem_size();
					if (elems == 2)
					{
						load_packed = align <= size + 1;
					}
					else if (elems >= 3)
					{
						if (align <= size + 2)
							load_packed = true;
						// FB13188413: AMD compiler miscompiles as_type<uchar4>(packed_ushort2), so avoid that
						// Reproducing case:
						// kernel void test(uint pos [[thread_position_in_grid]], device uint4* out, device const packed_ushort2* in) {
						//     out[pos] = uint4(as_type<uchar4>(in[pos])); // Top half is zero on AMD
						// }
						if (align == size + 1 && align == 2)
							// Many GPUs are better at loading uints than smaller sizes so load a larger type
							load_type = Storage::Simple(2, size + 1, false, false);
					}
				}
			}

			// Write out loading code
			line.clear();
			line.append("data");
			line.append(std::to_string(attribute.binding));
			if (info.read_past_stride)
			{
				line.append(".data[");
				line.append(std::to_string(attribute.offset / stored_type.size()));
				line.push_back(']');
			}
			else if (info.usage[attribute.offset].name_attribute == ByteUsage::NAME_NONE)
			{
				line.append(".spvData");
				line.append(std::to_string(attribute.offset));
			}
			else
			{
				assert(info.usage[attribute.offset].name_attribute == i);
				line.push_back('.');
				line.append(attribute.meta->alias);
			}
			// example line: data7.data[6]    (storage: uchar,  vertex: MSL_FORMAT_B8G8R8A8_UINT      to uint2)
			// example line: data6.spvData8   (storage: uint2,  vertex: MSL_FORMAT_B16G16R16A16_UNORM to float2)
			// example line: data5.attribute3 (storage: uchar2, vertex: MSL_FORMAT_R8G8_SRGB          to float2, AMD GPU)
			// example line: data5.attribute3 (storage: uchar2, vertex: MSL_FORMAT_R8G8_SRGB          to float2, Apple GPU)
			if (load_type != stored_type || load_packed != stored_packed)
			{
				tmp_string.clear();
				tmp_string.append("reinterpret_cast<const device ");
				if (load_packed)
					tmp_string.append("packed_");
				tmp_string.append(load_type.msl_name());
				tmp_string.append("&>(");
				line.insert(0, tmp_string);
				line.push_back(')');
			}

			// Packed types don't support `.rgba` syntax or as_type on older Metal versions
			if (load_packed && !out.msl_options.supports_msl_version(2, 1))
				load_type.write_cast(line);

			// example line: reinterpret_cast<const device packed_ushort2&>(data7.attribute4)
			// example line: data6.spvData8
			// example line: data5.attribute3
			// example line: reinterpret_cast<const device srgba8unorm<float4>&>(data5.attribute3)
			if (load_type != convert_in_type)
			{
				assert(!load_type.is_pixel_format());
				assert(!convert_in_type.is_pixel_format());
				if (!convert_in_type.can_cast_from(load_type))
				{
					// Need an as_type cast
					uint32_t load_elem_size = 1 << load_type.log2_elem_size();
					uint32_t target_elem_size = 1 << convert_in_type.log2_elem_size();
					uint32_t load_size = load_elem_size * (!load_packed && load_type.elements() == 3 ? 4 : load_type.elements());
					uint32_t target_size = target_elem_size * (convert_in_type.elements() == 3 ? 4 : convert_in_type.elements());
					Storage cast_type = convert_in_type;
					if (load_size < target_size)
					{
						if (convert_in_type.size() == load_size)
						{
							// Load is a packed 3-element vector getting casted to a non-packed 3-element vector
							load_type.write_cast(line);
						}
						else
						{
							// need to zero fill for use with e.g. srgb conversion function
							uint32_t extra_count = 1 << (convert_in_type.log2_elem_size() - load_type.log2_elem_size());
							load_type.adjust_element_count(extra_count).write_cast(line);
							line.pop_back();
							for (uint32_t j = load_type.elements(); j < extra_count; j++)
								line.append(", 0");
							line.push_back(')');
						}
					}
					else if (load_size > target_size)
					{
						uint32_t src_elems = 0;
						uint32_t dst_elems = 0;
						// Attempt to match sizes by removing elements
						switch (load_type.log2_elem_size() + 1 - convert_in_type.log2_elem_size())
						{
						case 0:
							// load_type is half the size of convert_in_type
							// for sizes to not match, convert_in_type must have one element
							assert(convert_in_type.elements() == 1);
							src_elems = 2;
							dst_elems = 1;
							break;
						case 1:
							// Sizes equal
							src_elems = dst_elems = convert_in_type.elements();
							break;
						case 2:
							// load_type is double the size of convert_in_type
							src_elems = convert_in_type.elements() <= 2 ? 1 : 2;
							dst_elems = src_elems * 2;
							break;
						case 3:
							// load_type is 4x the size of convert_in_type
							src_elems = 1;
							dst_elems = 4;
							break;
						default:
							// Too big or too small
							assert(0);
							break;
						}
						if (src_elems != load_type.elements())
							line.append(".rgba", src_elems + 1);
						cast_type = convert_in_type.adjust_element_count(dst_elems);
					}
					tmp_string.clear();
					tmp_string.append("as_type<");
					tmp_string.append(cast_type.msl_name());
					tmp_string.append(">(");
					line.insert(0, tmp_string);
					line.push_back(')');
					if (cast_type != convert_in_type)
					{
						assert(cast_type.elements() > convert_in_type.elements());
						line.append(".rgba", convert_in_type.elements() + 1);
					}
				}
				else
				{
					assert(convert_in_type.elements() <= load_type.elements());
					if (!convert_in_type.types_match(load_type))
						convert_in_type.adjust_element_count(load_type.elements()).write_cast(line);
					if (convert_in_type.elements() != load_type.elements())
						line.append(".rgba", convert_in_type.elements() + 1);
				}
			}
			// example line: as_type<uchar4>(reinterpret_cast<const device packed_ushort2&>(data7.data[6]))
			// example line: as_type<ushort4>(data6.spvData8)
			// example line: as_type<uint>(uchar4(data5.attribute3, 0, 0))
			// example line: reinterpret_cast<const device srgba8unorm<float4>&>(data5.attribute3)
			if (convert_function)
			{
				tmp_string.clear();
				tmp_string.append(convert_function);
				tmp_string.push_back('(');
				line.insert(0, tmp_string);
				line.push_back(')');
			}
			// example line: as_type<uchar4>(reinterpret_cast<const device packed_ushort2&>(data7.data[6]))
			// example line: as_type<ushort4>(data6.spvData8)
			// example line: unpack_unorm4x8_srgb_to_float(as_type<uint>(uchar4(data5.attribute3, 0, 0)))
			// example line: reinterpret_cast<const device srgba8unorm<float4>&>(data5.attribute3)
			bool needs_parentheses = false;
			if (normalize_value)
			{
				convert_out_type.write_cast(line);
				line.append(" * (1.");
				char suffix = convert_out_type.log2_elem_size() == 2 ? 'f' : 'h';
				line.push_back(suffix);
				line.append(" / ");
				line.append(normalize_value);
				line.push_back(')');
				if (normalize_clamp)
				{
					line.insert(0, "max(");
					line.append(", -1.");
					line.push_back(suffix);
					line.push_back(')');
				}
				else
				{
					needs_parentheses = true;
				}
			}
			// example line: as_type<uchar4>(reinterpret_cast<const device packed_ushort2&>(data7.data[6]))
			// example line: float4(as_type<ushort4>(data6.spvData8)) * (1.f / 65535)
			// example line: unpack_unorm4x8_srgb_to_float(as_type<uint>(uchar4(data5.attribute3, 0, 0)))
			// example line: reinterpret_cast<const device srgba8unorm<float4>&>(data5.attribute3)
			Storage final_type = Storage::FromSPIRType(attribute.load_type);
			if (!final_type.types_match(convert_out_type))
			{
				final_type.adjust_element_count(convert_out_type.elements()).write_cast(line);
				needs_parentheses = false;
			}
			// example line: uint4(as_type<uchar4>(reinterpret_cast<const device packed_ushort2&>(data7.data[6])))
			// example line: float4(as_type<ushort4>(data6.spvData8)) * (1.f / 65535)
			// example line: unpack_unorm4x8_srgb_to_float(as_type<uint>(uchar4(data5.attribute3, 0, 0)))
			// example line: reinterpret_cast<const device srgba8unorm<float4>&>(data5.attribute3)
			uint32_t wanted_elements = std::min<uint32_t>(format.num_elems, attribute.load_type->vecsize);
			if (wanted_elements < convert_out_type.elements() || format.is_bgr)
			{
				if (convert_in_type.is_pixel_format())
				{
					// Pixel formats can't use the `.rgb` syntax
					convert_out_type.write_cast(line);
				}
				else if (needs_parentheses)
				{
					line.insert(0, "(");
					line.push_back(')');
				}
				line.append(format.is_bgr ? ".bgra" : ".rgba", wanted_elements + 1);
			}
			// example line: uint4(as_type<uchar4>(reinterpret_cast<const device packed_ushort2&>(data7.data[6]))).bg
			// example line: (float4(as_type<ushort4>(data6.spvData8)) * (1.f / 65535)).bgr
			// example line: unpack_unorm4x8_srgb_to_float(as_type<uint>(uchar4(data5.attribute3, 0, 0))).rg
			// example line: float4(reinterpret_cast<const device srgba8unorm<float4>&>(data5.attribute3)).rg
			if (final_type.elements() > wanted_elements)
			{
				final_type.write_cast(line);
				line.pop_back();
				for (uint32_t j = wanted_elements; j < final_type.elements(); j++)
					line.append(j == 3 ? ", 1" : ", 0");
				line.push_back(')');
			}
			// example line: uint4(as_type<uchar4>(reinterpret_cast<const device packed_ushort2&>(data7.data[6]))).bg
			// example line: (float4(as_type<ushort4>(data6.spvData8)) * (1.f / 65535)).bgr
			// example line: float4(unpack_unorm4x8_srgb_to_float(as_type<uint>(uchar4(data5.attribute3, 0, 0))).rg, 0, 1)
			// example line: float4(float4(reinterpret_cast<const device srgba8unorm<float4>&>(data5.attribute3)).rg, 0, 1)
			out.statement("out.", attributes[i].meta->alias, " = ", line, ";");
		}
		out.statement("return out;");
		out.end_scope();
		out.statement("");
	}
};

void CompilerMSL::MSLVertexLoaderWriter::generate(CompilerMSL &out, TypeID main_struct)
{
	Impl impl(*this, out);
	impl.decide_attribute_storage();
	impl.generate_vertex_data_structs();
	impl.generate_loader(main_struct);
}
