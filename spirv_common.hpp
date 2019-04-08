/*
 * Copyright 2015-2019 Arm Limited
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

#ifndef SPIRV_CROSS_COMMON_HPP
#define SPIRV_CROSS_COMMON_HPP

#include "spirv.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <stack>
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>

// A bit crude, but allows projects which embed SPIRV-Cross statically to
// effectively hide all the symbols from other projects.
// There is a case where we have:
// - Project A links against SPIRV-Cross statically.
// - Project A links against Project B statically.
// - Project B links against SPIRV-Cross statically (might be a different version).
// This leads to a conflict with extremely bizarre results.
// By overriding the namespace in one of the project builds, we can work around this.
// If SPIRV-Cross is embedded in dynamic libraries,
// prefer using -fvisibility=hidden on GCC/Clang instead.
#ifdef SPIRV_CROSS_NAMESPACE_OVERRIDE
#define SPIRV_CROSS_NAMESPACE SPIRV_CROSS_NAMESPACE_OVERRIDE
#else
#define SPIRV_CROSS_NAMESPACE spirv_cross
#endif

namespace SPIRV_CROSS_NAMESPACE
{

#ifdef SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS
#ifndef _MSC_VER
[[noreturn]]
#endif
inline void
report_and_abort(const std::string &msg)
{
#ifdef NDEBUG
	(void)msg;
#else
	fprintf(stderr, "There was a compiler error: %s\n", msg.c_str());
#endif
	fflush(stderr);
	abort();
}

#define SPIRV_CROSS_THROW(x) report_and_abort(x)
#else
class CompilerError : public std::runtime_error
{
public:
	explicit CompilerError(const std::string &str)
	    : std::runtime_error(str)
	{
	}
};

#define SPIRV_CROSS_THROW(x) throw CompilerError(x)
#endif

//#define SPIRV_CROSS_COPY_CONSTRUCTOR_SANITIZE

// MSVC 2013 does not have noexcept. We need this for Variant to get move constructor to work correctly
// instead of copy constructor.
// MSVC 2013 ignores that move constructors cannot throw in std::vector, so just don't define it.
#if defined(_MSC_VER) && _MSC_VER < 1900
#define SPIRV_CROSS_NOEXCEPT

// Alignas is broken in MSVC 2013, work around it with compiler specifics.
#define SPIRV_CROSS_ALIGNAS(N)
#define SPIRV_CROSS_CLASS_ALIGN(N) __declspec(align(N))
#else
#define SPIRV_CROSS_NOEXCEPT noexcept
#define SPIRV_CROSS_ALIGNAS(N) alignas(N)
#define SPIRV_CROSS_CLASS_ALIGN(N)
#endif

#if __cplusplus >= 201402l
#define SPIRV_CROSS_DEPRECATED(reason) [[deprecated(reason)]]
#elif defined(__GNUC__)
#define SPIRV_CROSS_DEPRECATED(reason) __attribute__((deprecated))
#elif defined(_MSC_VER)
#define SPIRV_CROSS_DEPRECATED(reason) __declspec(deprecated(reason))
#else
#define SPIRV_CROSS_DEPRECATED(reason)
#endif

// std::aligned_storage does not support size == 0, so roll our own.
template <typename T, size_t N>
SPIRV_CROSS_CLASS_ALIGN(8) class AlignedBuffer
{
public:
	T *data()
	{
		return reinterpret_cast<T *>(aligned_char);
	}

private:
	SPIRV_CROSS_ALIGNAS(T) char aligned_char[sizeof(T) * N];
};

template <typename T>
class AlignedBuffer<T, 0>
{
public:
	T *data()
	{
		return nullptr;
	}
};

// An immutable version of SmallVector which erases type information about storage.
template <typename T>
class VectorView
{
public:
	T &operator[](size_t i)
	{
		return ptr[i];
	}

	const T &operator[](size_t i) const
	{
		return ptr[i];
	}

	bool empty() const
	{
		return buffer_size == 0;
	}

	size_t size() const
	{
		return buffer_size;
	}

	T *data()
	{
		return ptr;
	}

	const T *data() const
	{
		return ptr;
	}

	T *begin()
	{
		return ptr;
	}

	T *end()
	{
		return ptr + buffer_size;
	}

	const T *begin() const
	{
		return ptr;
	}

	const T *end() const
	{
		return ptr + buffer_size;
	}

	T &front()
	{
		return ptr[0];
	}

	const T &front() const
	{
		return ptr[0];
	}

	T &back()
	{
		return ptr[buffer_size - 1];
	}

	const T &back() const
	{
		return ptr[buffer_size - 1];
	}

	// Avoid sliced copies. Base class should only be read as a reference.
	VectorView(const VectorView &) = delete;
	void operator=(const VectorView &) = delete;

protected:
	VectorView() = default;
	T *ptr = nullptr;
	size_t buffer_size = 0;
};

// Simple vector which supports up to N elements inline, without malloc/free.
// We use a lot of throwaway vectors all over the place which triggers allocations.
// This class only implements the subset of std::vector we need in SPIRV-Cross.
// It is *NOT* a drop-in replacement.
template <typename T, size_t N = 8>
class SmallVector : public VectorView<T>
{
public:
	SmallVector()
	{
		this->ptr = stack_storage.data();
		buffer_capacity = N;
	}

	SmallVector(const T *arg_list_begin, const T *arg_list_end)
	    : SmallVector()
	{
		auto count = size_t(arg_list_end - arg_list_begin);
		reserve(count);
		for (size_t i = 0; i < count; i++, arg_list_begin++)
			new (&this->ptr[i]) T(*arg_list_begin);
		this->buffer_size = count;
	}

	SmallVector(SmallVector &&other) SPIRV_CROSS_NOEXCEPT : SmallVector()
	{
		*this = std::move(other);
	}

	SmallVector &operator=(SmallVector &&other) SPIRV_CROSS_NOEXCEPT
	{
		clear();
		if (other.ptr != other.stack_storage.data())
		{
			// Pilfer allocated pointer.
			if (this->ptr != stack_storage.data())
				free(this->ptr);
			this->ptr = other.ptr;
			this->buffer_size = other.buffer_size;
			buffer_capacity = other.buffer_capacity;
			other.ptr = nullptr;
			other.buffer_size = 0;
			other.buffer_capacity = 0;
		}
		else
		{
			// Need to move the stack contents individually.
			reserve(other.buffer_size);
			for (size_t i = 0; i < other.buffer_size; i++)
			{
				new (&this->ptr[i]) T(std::move(other.ptr[i]));
				other.ptr[i].~T();
			}
			this->buffer_size = other.buffer_size;
			other.buffer_size = 0;
		}
		return *this;
	}

	SmallVector(const SmallVector &other)
	    : SmallVector()
	{
		*this = other;
	}

	SmallVector &operator=(const SmallVector &other)
	{
		clear();
		reserve(other.buffer_size);
		for (size_t i = 0; i < other.buffer_size; i++)
			new (&this->ptr[i]) T(other.ptr[i]);
		this->buffer_size = other.buffer_size;
		return *this;
	}

	explicit SmallVector(size_t count)
	    : SmallVector()
	{
		resize(count);
	}

	~SmallVector()
	{
		clear();
		if (this->ptr != stack_storage.data())
			free(this->ptr);
	}

	void clear()
	{
		for (size_t i = 0; i < this->buffer_size; i++)
			this->ptr[i].~T();
		this->buffer_size = 0;
	}

	void push_back(const T &t)
	{
		reserve(this->buffer_size + 1);
		new (&this->ptr[this->buffer_size]) T(t);
		this->buffer_size++;
	}

	void push_back(T &&t)
	{
		reserve(this->buffer_size + 1);
		new (&this->ptr[this->buffer_size]) T(std::move(t));
		this->buffer_size++;
	}

	void pop_back()
	{
		resize(this->buffer_size - 1);
	}

	template <typename... Ts>
	void emplace_back(Ts &&... ts)
	{
		reserve(this->buffer_size + 1);
		new (&this->ptr[this->buffer_size]) T(std::forward<Ts>(ts)...);
		this->buffer_size++;
	}

	void reserve(size_t count)
	{
		if (count > buffer_capacity)
		{
			size_t target_capacity = buffer_capacity;
			if (target_capacity == 0)
				target_capacity = 1;
			if (target_capacity < N)
				target_capacity = N;

			while (target_capacity < count)
				target_capacity <<= 1u;

			T *new_buffer =
			    target_capacity > N ? static_cast<T *>(malloc(target_capacity * sizeof(T))) : stack_storage.data();

			if (!new_buffer)
				SPIRV_CROSS_THROW("Out of memory.");

			// In case for some reason two allocations both come from same stack.
			if (new_buffer != this->ptr)
			{
				// We don't deal with types which can throw in move constructor.
				for (size_t i = 0; i < this->buffer_size; i++)
				{
					new (&new_buffer[i]) T(std::move(this->ptr[i]));
					this->ptr[i].~T();
				}
			}

			if (this->ptr != stack_storage.data())
				free(this->ptr);
			this->ptr = new_buffer;
			buffer_capacity = target_capacity;
		}
	}

	void insert(T *itr, const T *insert_begin, const T *insert_end)
	{
		if (itr == this->end())
		{
			auto count = size_t(insert_end - insert_begin);
			reserve(this->buffer_size + count);
			for (size_t i = 0; i < count; i++, insert_begin++)
				new (&this->ptr[this->buffer_size + i]) T(*insert_begin);
			this->buffer_size += count;
		}
		else
			SPIRV_CROSS_THROW("Mid-insert not implemented.");
	}

	T *erase(T *itr)
	{
		std::move(itr + 1, this->end(), itr);
		this->ptr[--this->buffer_size].~T();
		return itr;
	}

	void erase(T *start_erase, T *end_erase)
	{
		if (end_erase != this->end())
			SPIRV_CROSS_THROW("Mid-erase not implemented.");
		resize(size_t(start_erase - this->begin()));
	}

	void resize(size_t new_size)
	{
		if (new_size < this->buffer_size)
		{
			for (size_t i = new_size; i < this->buffer_size; i++)
				this->ptr[i].~T();
		}
		else if (new_size > this->buffer_size)
		{
			reserve(new_size);
			for (size_t i = this->buffer_size; i < new_size; i++)
				new (&this->ptr[i]) T();
		}

		this->buffer_size = new_size;
	}

private:
	size_t buffer_capacity = 0;
	AlignedBuffer<T, N> stack_storage;
};

// A vector without stack storage.
// Could also be a typedef-ed to std::vector,
// but might as well use the one we have.
template <typename T>
using Vector = SmallVector<T, 0>;

// An object pool which we use for allocating IVariant-derived objects.
// We know we are going to allocate a bunch of objects of each type,
// so amortize the mallocs.
class ObjectPoolBase
{
public:
	virtual ~ObjectPoolBase() = default;
	virtual void free_opaque(void *ptr) = 0;
};

template <typename T>
class ObjectPool : public ObjectPoolBase
{
public:
	explicit ObjectPool(unsigned start_object_count_ = 16)
	    : start_object_count(start_object_count_)
	{
	}

	template <typename... P>
	T *allocate(P &&... p)
	{
		if (vacants.empty())
		{
			unsigned num_objects = start_object_count << memory.size();
			T *ptr = static_cast<T *>(malloc(num_objects * sizeof(T)));
			if (!ptr)
				return nullptr;

			for (unsigned i = 0; i < num_objects; i++)
				vacants.push_back(&ptr[i]);

			memory.emplace_back(ptr);
		}

		T *ptr = vacants.back();
		vacants.pop_back();
		new (ptr) T(std::forward<P>(p)...);
		return ptr;
	}

	void free(T *ptr)
	{
		ptr->~T();
		vacants.push_back(ptr);
	}

	void free_opaque(void *ptr) override
	{
		free(static_cast<T *>(ptr));
	}

	void clear()
	{
		vacants.clear();
		memory.clear();
	}

protected:
	Vector<T *> vacants;

	struct MallocDeleter
	{
		void operator()(T *ptr)
		{
			::free(ptr);
		}
	};

	SmallVector<std::unique_ptr<T, MallocDeleter>> memory;
	unsigned start_object_count;
};

template <size_t StackSize = 4096, size_t BlockSize = 4096>
class StringStream
{
public:
	StringStream()
	{
		reset();
	}

	~StringStream()
	{
		reset();
	}

	// Disable copies and moves. Makes it easier to implement, and we don't need it.
	StringStream(const StringStream &) = delete;
	void operator=(const StringStream &) = delete;

	template <typename T, typename std::enable_if<!std::is_floating_point<T>::value, int>::type = 0>
	StringStream &operator<<(const T &t)
	{
		auto s = std::to_string(t);
		append(s.data(), s.size());
		return *this;
	}

	// Only overload this to make float/double conversions ambiguous.
	StringStream &operator<<(uint32_t v)
	{
		auto s = std::to_string(v);
		append(s.data(), s.size());
		return *this;
	}

	StringStream &operator<<(char c)
	{
		append(&c, 1);
		return *this;
	}

	StringStream &operator<<(const std::string &str)
	{
		append(str.data(), str.size());
		return *this;
	}

	StringStream &operator<<(const char *s)
	{
		append(s, strlen(s));
		return *this;
	}

	template <size_t N>
	StringStream &operator<<(const char (&s)[N])
	{
		append(s, strlen(s));
		return *this;
	}

	std::string str() const
	{
		std::string ret;
		size_t target_size = 0;
		for (auto &saved : saved_buffers)
			target_size += saved.offset;
		target_size += current_buffer.offset;
		ret.reserve(target_size);

		for (auto &saved : saved_buffers)
			ret.insert(ret.end(), saved.buffer, saved.buffer + saved.offset);
		ret.insert(ret.end(), current_buffer.buffer, current_buffer.buffer + current_buffer.offset);
		return ret;
	}

	void reset()
	{
		for (auto &saved : saved_buffers)
			if (saved.buffer != stack_buffer)
				free(saved.buffer);
		if (current_buffer.buffer != stack_buffer)
			free(current_buffer.buffer);

		saved_buffers.clear();
		current_buffer.buffer = stack_buffer;
		current_buffer.offset = 0;
		current_buffer.size = sizeof(stack_buffer);
	}

private:
	struct Buffer
	{
		char *buffer;
		size_t offset;
		size_t size;
	};
	Buffer current_buffer = {};
	char stack_buffer[StackSize];
	SmallVector<Buffer> saved_buffers;

	void append(const char *str, size_t len)
	{
		size_t avail = current_buffer.size - current_buffer.offset;
		if (avail < len)
		{
			if (avail > 0)
			{
				memcpy(current_buffer.buffer + current_buffer.offset, str, avail);
				str += avail;
				len -= avail;
				current_buffer.offset += avail;
			}

			saved_buffers.push_back(current_buffer);
			size_t target_size = len > BlockSize ? len : BlockSize;
			current_buffer.buffer = static_cast<char *>(malloc(target_size));
			if (!current_buffer.buffer)
				SPIRV_CROSS_THROW("Out of memory.");

			memcpy(current_buffer.buffer, str, len);
			current_buffer.offset = len;
			current_buffer.size = target_size;
		}
		else
		{
			memcpy(current_buffer.buffer + current_buffer.offset, str, len);
			current_buffer.offset += len;
		}
	}
};

namespace inner
{
template <typename T>
void join_helper(StringStream<> &stream, T &&t)
{
	stream << std::forward<T>(t);
}

template <typename T, typename... Ts>
void join_helper(StringStream<> &stream, T &&t, Ts &&... ts)
{
	stream << std::forward<T>(t);
	join_helper(stream, std::forward<Ts>(ts)...);
}
} // namespace inner

class Bitset
{
public:
	Bitset() = default;
	explicit inline Bitset(uint64_t lower_)
	    : lower(lower_)
	{
	}

	inline bool get(uint32_t bit) const
	{
		if (bit < 64)
			return (lower & (1ull << bit)) != 0;
		else
			return higher.count(bit) != 0;
	}

	inline void set(uint32_t bit)
	{
		if (bit < 64)
			lower |= 1ull << bit;
		else
			higher.insert(bit);
	}

	inline void clear(uint32_t bit)
	{
		if (bit < 64)
			lower &= ~(1ull << bit);
		else
			higher.erase(bit);
	}

	inline uint64_t get_lower() const
	{
		return lower;
	}

	inline void reset()
	{
		lower = 0;
		higher.clear();
	}

	inline void merge_and(const Bitset &other)
	{
		lower &= other.lower;
		std::unordered_set<uint32_t> tmp_set;
		for (auto &v : higher)
			if (other.higher.count(v) != 0)
				tmp_set.insert(v);
		higher = std::move(tmp_set);
	}

	inline void merge_or(const Bitset &other)
	{
		lower |= other.lower;
		for (auto &v : other.higher)
			higher.insert(v);
	}

	inline bool operator==(const Bitset &other) const
	{
		if (lower != other.lower)
			return false;

		if (higher.size() != other.higher.size())
			return false;

		for (auto &v : higher)
			if (other.higher.count(v) == 0)
				return false;

		return true;
	}

	inline bool operator!=(const Bitset &other) const
	{
		return !(*this == other);
	}

	template <typename Op>
	void for_each_bit(const Op &op) const
	{
		// TODO: Add ctz-based iteration.
		for (uint32_t i = 0; i < 64; i++)
		{
			if (lower & (1ull << i))
				op(i);
		}

		if (higher.empty())
			return;

		// Need to enforce an order here for reproducible results,
		// but hitting this path should happen extremely rarely, so having this slow path is fine.
		SmallVector<uint32_t> bits;
		bits.reserve(higher.size());
		for (auto &v : higher)
			bits.push_back(v);
		std::sort(std::begin(bits), std::end(bits));

		for (auto &v : bits)
			op(v);
	}

	inline bool empty() const
	{
		return lower == 0 && higher.empty();
	}

private:
	// The most common bits to set are all lower than 64,
	// so optimize for this case. Bits spilling outside 64 go into a slower data structure.
	// In almost all cases, higher data structure will not be used.
	uint64_t lower = 0;
	std::unordered_set<uint32_t> higher;
};

// Helper template to avoid lots of nasty string temporary munging.
template <typename... Ts>
std::string join(Ts &&... ts)
{
	StringStream<> stream;
	inner::join_helper(stream, std::forward<Ts>(ts)...);
	return stream.str();
}

inline std::string merge(const SmallVector<std::string> &list)
{
	StringStream<> stream;
	for (auto &elem : list)
	{
		stream << elem;
		if (&elem != &list.back())
			stream << ", ";
	}
	return stream.str();
}

// Make sure we don't accidentally call this with float or doubles with SFINAE.
// Have to use the radix-aware overload.
template <typename T, typename std::enable_if<!std::is_floating_point<T>::value, int>::type = 0>
inline std::string convert_to_string(const T &t)
{
	return std::to_string(t);
}

// Allow implementations to set a convenient standard precision
#ifndef SPIRV_CROSS_FLT_FMT
#define SPIRV_CROSS_FLT_FMT "%.32g"
#endif

#ifdef _MSC_VER
// sprintf warning.
// We cannot rely on snprintf existing because, ..., MSVC.
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

static inline void fixup_radix_point(char *str, char radix_point)
{
	// Setting locales is a very risky business in multi-threaded program,
	// so just fixup locales instead. We only need to care about the radix point.
	if (radix_point != '.')
	{
		while (*str != '\0')
		{
			if (*str == radix_point)
				*str = '.';
			str++;
		}
	}
}

inline std::string convert_to_string(float t, char locale_radix_point)
{
	// std::to_string for floating point values is broken.
	// Fallback to something more sane.
	char buf[64];
	sprintf(buf, SPIRV_CROSS_FLT_FMT, t);
	fixup_radix_point(buf, locale_radix_point);

	// Ensure that the literal is float.
	if (!strchr(buf, '.') && !strchr(buf, 'e'))
		strcat(buf, ".0");
	return buf;
}

inline std::string convert_to_string(double t, char locale_radix_point)
{
	// std::to_string for floating point values is broken.
	// Fallback to something more sane.
	char buf[64];
	sprintf(buf, SPIRV_CROSS_FLT_FMT, t);
	fixup_radix_point(buf, locale_radix_point);

	// Ensure that the literal is float.
	if (!strchr(buf, '.') && !strchr(buf, 'e'))
		strcat(buf, ".0");
	return buf;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

struct Instruction
{
	uint16_t op = 0;
	uint16_t count = 0;
	uint32_t offset = 0;
	uint32_t length = 0;
};

// Helper for Variant interface.
struct IVariant
{
	virtual ~IVariant() = default;
	virtual IVariant *clone(ObjectPoolBase *pool) = 0;
	uint32_t self = 0;
};

#define SPIRV_CROSS_DECLARE_CLONE(T)                                \
	IVariant *clone(ObjectPoolBase *pool) override                  \
	{                                                               \
		return static_cast<ObjectPool<T> *>(pool)->allocate(*this); \
	}

enum Types
{
	TypeNone,
	TypeType,
	TypeVariable,
	TypeConstant,
	TypeFunction,
	TypeFunctionPrototype,
	TypeBlock,
	TypeExtension,
	TypeExpression,
	TypeConstantOp,
	TypeCombinedImageSampler,
	TypeAccessChain,
	TypeUndef,
	TypeCount
};

struct SPIRUndef : IVariant
{
	enum
	{
		type = TypeUndef
	};

	explicit SPIRUndef(uint32_t basetype_)
	    : basetype(basetype_)
	{
	}
	uint32_t basetype;

	SPIRV_CROSS_DECLARE_CLONE(SPIRUndef)
};

// This type is only used by backends which need to access the combined image and sampler IDs separately after
// the OpSampledImage opcode.
struct SPIRCombinedImageSampler : IVariant
{
	enum
	{
		type = TypeCombinedImageSampler
	};
	SPIRCombinedImageSampler(uint32_t type_, uint32_t image_, uint32_t sampler_)
	    : combined_type(type_)
	    , image(image_)
	    , sampler(sampler_)
	{
	}
	uint32_t combined_type;
	uint32_t image;
	uint32_t sampler;

	SPIRV_CROSS_DECLARE_CLONE(SPIRCombinedImageSampler)
};

struct SPIRConstantOp : IVariant
{
	enum
	{
		type = TypeConstantOp
	};

	SPIRConstantOp(uint32_t result_type, spv::Op op, const uint32_t *args, uint32_t length)
	    : opcode(op)
	    , arguments(args, args + length)
	    , basetype(result_type)
	{
	}

	spv::Op opcode;
	SmallVector<uint32_t> arguments;
	uint32_t basetype;

	SPIRV_CROSS_DECLARE_CLONE(SPIRConstantOp)
};

struct SPIRType : IVariant
{
	enum
	{
		type = TypeType
	};

	enum BaseType
	{
		Unknown,
		Void,
		Boolean,
		SByte,
		UByte,
		Short,
		UShort,
		Int,
		UInt,
		Int64,
		UInt64,
		AtomicCounter,
		Half,
		Float,
		Double,
		Struct,
		Image,
		SampledImage,
		Sampler,
		AccelerationStructureNV,

		// Keep internal types at the end.
		ControlPointArray,
		Char
	};

	// Scalar/vector/matrix support.
	BaseType basetype = Unknown;
	uint32_t width = 0;
	uint32_t vecsize = 1;
	uint32_t columns = 1;

	// Arrays, support array of arrays by having a vector of array sizes.
	SmallVector<uint32_t> array;

	// Array elements can be either specialization constants or specialization ops.
	// This array determines how to interpret the array size.
	// If an element is true, the element is a literal,
	// otherwise, it's an expression, which must be resolved on demand.
	// The actual size is not really known until runtime.
	SmallVector<bool> array_size_literal;

	// Pointers
	// Keep track of how many pointer layers we have.
	uint32_t pointer_depth = 0;
	bool pointer = false;

	spv::StorageClass storage = spv::StorageClassGeneric;

	SmallVector<uint32_t> member_types;

	struct ImageType
	{
		uint32_t type;
		spv::Dim dim;
		bool depth;
		bool arrayed;
		bool ms;
		uint32_t sampled;
		spv::ImageFormat format;
		spv::AccessQualifier access;
	} image;

	// Structs can be declared multiple times if they are used as part of interface blocks.
	// We want to detect this so that we only emit the struct definition once.
	// Since we cannot rely on OpName to be equal, we need to figure out aliases.
	uint32_t type_alias = 0;

	// Denotes the type which this type is based on.
	// Allows the backend to traverse how a complex type is built up during access chains.
	uint32_t parent_type = 0;

	// Used in backends to avoid emitting members with conflicting names.
	std::unordered_set<std::string> member_name_cache;

	SPIRV_CROSS_DECLARE_CLONE(SPIRType)
};

struct SPIRExtension : IVariant
{
	enum
	{
		type = TypeExtension
	};

	enum Extension
	{
		Unsupported,
		GLSL,
		SPV_AMD_shader_ballot,
		SPV_AMD_shader_explicit_vertex_parameter,
		SPV_AMD_shader_trinary_minmax,
		SPV_AMD_gcn_shader
	};

	explicit SPIRExtension(Extension ext_)
	    : ext(ext_)
	{
	}

	Extension ext;
	SPIRV_CROSS_DECLARE_CLONE(SPIRExtension)
};

// SPIREntryPoint is not a variant since its IDs are used to decorate OpFunction,
// so in order to avoid conflicts, we can't stick them in the ids array.
struct SPIREntryPoint
{
	SPIREntryPoint(uint32_t self_, spv::ExecutionModel execution_model, const std::string &entry_name)
	    : self(self_)
	    , name(entry_name)
	    , orig_name(entry_name)
	    , model(execution_model)
	{
	}
	SPIREntryPoint() = default;

	uint32_t self = 0;
	std::string name;
	std::string orig_name;
	SmallVector<uint32_t> interface_variables;

	Bitset flags;
	struct
	{
		uint32_t x = 0, y = 0, z = 0;
		uint32_t constant = 0; // Workgroup size can be expressed as a constant/spec-constant instead.
	} workgroup_size;
	uint32_t invocations = 0;
	uint32_t output_vertices = 0;
	spv::ExecutionModel model = spv::ExecutionModelMax;
};

struct SPIRExpression : IVariant
{
	enum
	{
		type = TypeExpression
	};

	// Only created by the backend target to avoid creating tons of temporaries.
	SPIRExpression(std::string expr, uint32_t expression_type_, bool immutable_)
	    : expression(move(expr))
	    , expression_type(expression_type_)
	    , immutable(immutable_)
	{
	}

	// If non-zero, prepend expression with to_expression(base_expression).
	// Used in amortizing multiple calls to to_expression()
	// where in certain cases that would quickly force a temporary when not needed.
	uint32_t base_expression = 0;

	std::string expression;
	uint32_t expression_type = 0;

	// If this expression is a forwarded load,
	// allow us to reference the original variable.
	uint32_t loaded_from = 0;

	// If this expression will never change, we can avoid lots of temporaries
	// in high level source.
	// An expression being immutable can be speculative,
	// it is assumed that this is true almost always.
	bool immutable = false;

	// Before use, this expression must be transposed.
	// This is needed for targets which don't support row_major layouts.
	bool need_transpose = false;

	// Whether or not this is an access chain expression.
	bool access_chain = false;

	// A list of expressions which this expression depends on.
	SmallVector<uint32_t> expression_dependencies;

	// By reading this expression, we implicitly read these expressions as well.
	// Used by access chain Store and Load since we read multiple expressions in this case.
	SmallVector<uint32_t> implied_read_expressions;

	SPIRV_CROSS_DECLARE_CLONE(SPIRExpression)
};

struct SPIRFunctionPrototype : IVariant
{
	enum
	{
		type = TypeFunctionPrototype
	};

	explicit SPIRFunctionPrototype(uint32_t return_type_)
	    : return_type(return_type_)
	{
	}

	uint32_t return_type;
	SmallVector<uint32_t> parameter_types;

	SPIRV_CROSS_DECLARE_CLONE(SPIRFunctionPrototype)
};

struct SPIRBlock : IVariant
{
	enum
	{
		type = TypeBlock
	};

	enum Terminator
	{
		Unknown,
		Direct, // Emit next block directly without a particular condition.

		Select, // Block ends with an if/else block.
		MultiSelect, // Block ends with switch statement.

		Return, // Block ends with return.
		Unreachable, // Noop
		Kill // Discard
	};

	enum Merge
	{
		MergeNone,
		MergeLoop,
		MergeSelection
	};

	enum Hints
	{
		HintNone,
		HintUnroll,
		HintDontUnroll,
		HintFlatten,
		HintDontFlatten
	};

	enum Method
	{
		MergeToSelectForLoop,
		MergeToDirectForLoop,
		MergeToSelectContinueForLoop
	};

	enum ContinueBlockType
	{
		ContinueNone,

		// Continue block is branchless and has at least one instruction.
		ForLoop,

		// Noop continue block.
		WhileLoop,

		// Continue block is conditional.
		DoWhileLoop,

		// Highly unlikely that anything will use this,
		// since it is really awkward/impossible to express in GLSL.
		ComplexLoop
	};

	enum
	{
		NoDominator = 0xffffffffu
	};

	Terminator terminator = Unknown;
	Merge merge = MergeNone;
	Hints hint = HintNone;
	uint32_t next_block = 0;
	uint32_t merge_block = 0;
	uint32_t continue_block = 0;

	uint32_t return_value = 0; // If 0, return nothing (void).
	uint32_t condition = 0;
	uint32_t true_block = 0;
	uint32_t false_block = 0;
	uint32_t default_block = 0;

	SmallVector<Instruction> ops;

	struct Phi
	{
		uint32_t local_variable; // flush local variable ...
		uint32_t parent; // If we're in from_block and want to branch into this block ...
		uint32_t function_variable; // to this function-global "phi" variable first.
	};

	// Before entering this block flush out local variables to magical "phi" variables.
	SmallVector<Phi> phi_variables;

	// Declare these temporaries before beginning the block.
	// Used for handling complex continue blocks which have side effects.
	SmallVector<std::pair<uint32_t, uint32_t>> declare_temporary;

	// Declare these temporaries, but only conditionally if this block turns out to be
	// a complex loop header.
	SmallVector<std::pair<uint32_t, uint32_t>> potential_declare_temporary;

	struct Case
	{
		uint32_t value;
		uint32_t block;
	};
	SmallVector<Case> cases;

	// If we have tried to optimize code for this block but failed,
	// keep track of this.
	bool disable_block_optimization = false;

	// If the continue block is complex, fallback to "dumb" for loops.
	bool complex_continue = false;

	// Do we need a ladder variable to defer breaking out of a loop construct after a switch block?
	bool need_ladder_break = false;

	// The dominating block which this block might be within.
	// Used in continue; blocks to determine if we really need to write continue.
	uint32_t loop_dominator = 0;

	// All access to these variables are dominated by this block,
	// so before branching anywhere we need to make sure that we declare these variables.
	SmallVector<uint32_t> dominated_variables;

	// These are variables which should be declared in a for loop header, if we
	// fail to use a classic for-loop,
	// we remove these variables, and fall back to regular variables outside the loop.
	SmallVector<uint32_t> loop_variables;

	// Some expressions are control-flow dependent, i.e. any instruction which relies on derivatives or
	// sub-group-like operations.
	// Make sure that we only use these expressions in the original block.
	SmallVector<uint32_t> invalidate_expressions;

	SPIRV_CROSS_DECLARE_CLONE(SPIRBlock)
};

struct SPIRFunction : IVariant
{
	enum
	{
		type = TypeFunction
	};

	SPIRFunction(uint32_t return_type_, uint32_t function_type_)
	    : return_type(return_type_)
	    , function_type(function_type_)
	{
	}

	struct Parameter
	{
		uint32_t type;
		uint32_t id;
		uint32_t read_count;
		uint32_t write_count;

		// Set to true if this parameter aliases a global variable,
		// used mostly in Metal where global variables
		// have to be passed down to functions as regular arguments.
		// However, for this kind of variable, we should not care about
		// read and write counts as access to the function arguments
		// is not local to the function in question.
		bool alias_global_variable;
	};

	// When calling a function, and we're remapping separate image samplers,
	// resolve these arguments into combined image samplers and pass them
	// as additional arguments in this order.
	// It gets more complicated as functions can pull in their own globals
	// and combine them with parameters,
	// so we need to distinguish if something is local parameter index
	// or a global ID.
	struct CombinedImageSamplerParameter
	{
		uint32_t id;
		uint32_t image_id;
		uint32_t sampler_id;
		bool global_image;
		bool global_sampler;
		bool depth;
	};

	uint32_t return_type;
	uint32_t function_type;
	SmallVector<Parameter> arguments;

	// Can be used by backends to add magic arguments.
	// Currently used by combined image/sampler implementation.

	SmallVector<Parameter> shadow_arguments;
	SmallVector<uint32_t> local_variables;
	uint32_t entry_block = 0;
	SmallVector<uint32_t> blocks;
	SmallVector<CombinedImageSamplerParameter> combined_parameters;

	void add_local_variable(uint32_t id)
	{
		local_variables.push_back(id);
	}

	void add_parameter(uint32_t parameter_type, uint32_t id, bool alias_global_variable = false)
	{
		// Arguments are read-only until proven otherwise.
		arguments.push_back({ parameter_type, id, 0u, 0u, alias_global_variable });
	}

	// Hooks to be run when the function returns.
	// Mostly used for lowering internal data structures onto flattened structures.
	// Need to defer this, because they might rely on things which change during compilation.
	// Intentionally not a small vector, this one is rare, and std::function can be large.
	Vector<std::function<void()>> fixup_hooks_out;

	// Hooks to be run when the function begins.
	// Mostly used for populating internal data structures from flattened structures.
	// Need to defer this, because they might rely on things which change during compilation.
	// Intentionally not a small vector, this one is rare, and std::function can be large.
	Vector<std::function<void()>> fixup_hooks_in;

	// On function entry, make sure to copy a constant array into thread addr space to work around
	// the case where we are passing a constant array by value to a function on backends which do not
	// consider arrays value types.
	SmallVector<uint32_t> constant_arrays_needed_on_stack;

	bool active = false;
	bool flush_undeclared = true;
	bool do_combined_parameters = true;

	SPIRV_CROSS_DECLARE_CLONE(SPIRFunction)
};

struct SPIRAccessChain : IVariant
{
	enum
	{
		type = TypeAccessChain
	};

	SPIRAccessChain(uint32_t basetype_, spv::StorageClass storage_, std::string base_, std::string dynamic_index_,
	                int32_t static_index_)
	    : basetype(basetype_)
	    , storage(storage_)
	    , base(std::move(base_))
	    , dynamic_index(std::move(dynamic_index_))
	    , static_index(static_index_)
	{
	}

	// The access chain represents an offset into a buffer.
	// Some backends need more complicated handling of access chains to be able to use buffers, like HLSL
	// which has no usable buffer type ala GLSL SSBOs.
	// StructuredBuffer is too limited, so our only option is to deal with ByteAddressBuffer which works with raw addresses.

	uint32_t basetype;
	spv::StorageClass storage;
	std::string base;
	std::string dynamic_index;
	int32_t static_index;

	uint32_t loaded_from = 0;
	uint32_t matrix_stride = 0;
	bool row_major_matrix = false;
	bool immutable = false;

	// By reading this expression, we implicitly read these expressions as well.
	// Used by access chain Store and Load since we read multiple expressions in this case.
	SmallVector<uint32_t> implied_read_expressions;

	SPIRV_CROSS_DECLARE_CLONE(SPIRAccessChain)
};

struct SPIRVariable : IVariant
{
	enum
	{
		type = TypeVariable
	};

	SPIRVariable() = default;
	SPIRVariable(uint32_t basetype_, spv::StorageClass storage_, uint32_t initializer_ = 0, uint32_t basevariable_ = 0)
	    : basetype(basetype_)
	    , storage(storage_)
	    , initializer(initializer_)
	    , basevariable(basevariable_)
	{
	}

	uint32_t basetype = 0;
	spv::StorageClass storage = spv::StorageClassGeneric;
	uint32_t decoration = 0;
	uint32_t initializer = 0;
	uint32_t basevariable = 0;

	SmallVector<uint32_t> dereference_chain;
	bool compat_builtin = false;

	// If a variable is shadowed, we only statically assign to it
	// and never actually emit a statement for it.
	// When we read the variable as an expression, just forward
	// shadowed_id as the expression.
	bool statically_assigned = false;
	uint32_t static_expression = 0;

	// Temporaries which can remain forwarded as long as this variable is not modified.
	SmallVector<uint32_t> dependees;
	bool forwardable = true;

	bool deferred_declaration = false;
	bool phi_variable = false;

	// Used to deal with Phi variable flushes. See flush_phi().
	bool allocate_temporary_copy = false;

	bool remapped_variable = false;
	uint32_t remapped_components = 0;

	// The block which dominates all access to this variable.
	uint32_t dominator = 0;
	// If true, this variable is a loop variable, when accessing the variable
	// outside a loop,
	// we should statically forward it.
	bool loop_variable = false;
	// Set to true while we're inside the for loop.
	bool loop_variable_enable = false;

	SPIRFunction::Parameter *parameter = nullptr;

	SPIRV_CROSS_DECLARE_CLONE(SPIRVariable)
};

struct SPIRConstant : IVariant
{
	enum
	{
		type = TypeConstant
	};

	union Constant {
		uint32_t u32;
		int32_t i32;
		float f32;

		uint64_t u64;
		int64_t i64;
		double f64;
	};

	struct ConstantVector
	{
		Constant r[4];
		// If != 0, this element is a specialization constant, and we should keep track of it as such.
		uint32_t id[4];
		uint32_t vecsize = 1;

		// Workaround for MSVC 2013, initializing an array breaks.
		ConstantVector()
		{
			memset(r, 0, sizeof(r));
			for (unsigned i = 0; i < 4; i++)
				id[i] = 0;
		}
	};

	struct ConstantMatrix
	{
		ConstantVector c[4];
		// If != 0, this column is a specialization constant, and we should keep track of it as such.
		uint32_t id[4];
		uint32_t columns = 1;

		// Workaround for MSVC 2013, initializing an array breaks.
		ConstantMatrix()
		{
			for (unsigned i = 0; i < 4; i++)
				id[i] = 0;
		}
	};

	static inline float f16_to_f32(uint16_t u16_value)
	{
		// Based on the GLM implementation.
		int s = (u16_value >> 15) & 0x1;
		int e = (u16_value >> 10) & 0x1f;
		int m = (u16_value >> 0) & 0x3ff;

		union {
			float f32;
			uint32_t u32;
		} u;

		if (e == 0)
		{
			if (m == 0)
			{
				u.u32 = uint32_t(s) << 31;
				return u.f32;
			}
			else
			{
				while ((m & 0x400) == 0)
				{
					m <<= 1;
					e--;
				}

				e++;
				m &= ~0x400;
			}
		}
		else if (e == 31)
		{
			if (m == 0)
			{
				u.u32 = (uint32_t(s) << 31) | 0x7f800000u;
				return u.f32;
			}
			else
			{
				u.u32 = (uint32_t(s) << 31) | 0x7f800000u | (m << 13);
				return u.f32;
			}
		}

		e += 127 - 15;
		m <<= 13;
		u.u32 = (uint32_t(s) << 31) | (e << 23) | m;
		return u.f32;
	}

	inline uint32_t specialization_constant_id(uint32_t col, uint32_t row) const
	{
		return m.c[col].id[row];
	}

	inline uint32_t specialization_constant_id(uint32_t col) const
	{
		return m.id[col];
	}

	inline uint32_t scalar(uint32_t col = 0, uint32_t row = 0) const
	{
		return m.c[col].r[row].u32;
	}

	inline int16_t scalar_i16(uint32_t col = 0, uint32_t row = 0) const
	{
		return int16_t(m.c[col].r[row].u32 & 0xffffu);
	}

	inline uint16_t scalar_u16(uint32_t col = 0, uint32_t row = 0) const
	{
		return uint16_t(m.c[col].r[row].u32 & 0xffffu);
	}

	inline int8_t scalar_i8(uint32_t col = 0, uint32_t row = 0) const
	{
		return int8_t(m.c[col].r[row].u32 & 0xffu);
	}

	inline uint8_t scalar_u8(uint32_t col = 0, uint32_t row = 0) const
	{
		return uint8_t(m.c[col].r[row].u32 & 0xffu);
	}

	inline float scalar_f16(uint32_t col = 0, uint32_t row = 0) const
	{
		return f16_to_f32(scalar_u16(col, row));
	}

	inline float scalar_f32(uint32_t col = 0, uint32_t row = 0) const
	{
		return m.c[col].r[row].f32;
	}

	inline int32_t scalar_i32(uint32_t col = 0, uint32_t row = 0) const
	{
		return m.c[col].r[row].i32;
	}

	inline double scalar_f64(uint32_t col = 0, uint32_t row = 0) const
	{
		return m.c[col].r[row].f64;
	}

	inline int64_t scalar_i64(uint32_t col = 0, uint32_t row = 0) const
	{
		return m.c[col].r[row].i64;
	}

	inline uint64_t scalar_u64(uint32_t col = 0, uint32_t row = 0) const
	{
		return m.c[col].r[row].u64;
	}

	inline const ConstantVector &vector() const
	{
		return m.c[0];
	}

	inline uint32_t vector_size() const
	{
		return m.c[0].vecsize;
	}

	inline uint32_t columns() const
	{
		return m.columns;
	}

	inline void make_null(const SPIRType &constant_type_)
	{
		m = {};
		m.columns = constant_type_.columns;
		for (auto &c : m.c)
			c.vecsize = constant_type_.vecsize;
	}

	inline bool constant_is_null() const
	{
		if (specialization)
			return false;
		if (!subconstants.empty())
			return false;

		for (uint32_t col = 0; col < columns(); col++)
			for (uint32_t row = 0; row < vector_size(); row++)
				if (scalar_u64(col, row) != 0)
					return false;

		return true;
	}

	explicit SPIRConstant(uint32_t constant_type_)
	    : constant_type(constant_type_)
	{
	}

	SPIRConstant() = default;

	SPIRConstant(uint32_t constant_type_, const uint32_t *elements, uint32_t num_elements, bool specialized)
	    : constant_type(constant_type_)
	    , specialization(specialized)
	{
		subconstants.insert(std::end(subconstants), elements, elements + num_elements);
		specialization = specialized;
	}

	// Construct scalar (32-bit).
	SPIRConstant(uint32_t constant_type_, uint32_t v0, bool specialized)
	    : constant_type(constant_type_)
	    , specialization(specialized)
	{
		m.c[0].r[0].u32 = v0;
		m.c[0].vecsize = 1;
		m.columns = 1;
	}

	// Construct scalar (64-bit).
	SPIRConstant(uint32_t constant_type_, uint64_t v0, bool specialized)
	    : constant_type(constant_type_)
	    , specialization(specialized)
	{
		m.c[0].r[0].u64 = v0;
		m.c[0].vecsize = 1;
		m.columns = 1;
	}

	// Construct vectors and matrices.
	SPIRConstant(uint32_t constant_type_, const SPIRConstant *const *vector_elements, uint32_t num_elements,
	             bool specialized)
	    : constant_type(constant_type_)
	    , specialization(specialized)
	{
		bool matrix = vector_elements[0]->m.c[0].vecsize > 1;

		if (matrix)
		{
			m.columns = num_elements;

			for (uint32_t i = 0; i < num_elements; i++)
			{
				m.c[i] = vector_elements[i]->m.c[0];
				if (vector_elements[i]->specialization)
					m.id[i] = vector_elements[i]->self;
			}
		}
		else
		{
			m.c[0].vecsize = num_elements;
			m.columns = 1;

			for (uint32_t i = 0; i < num_elements; i++)
			{
				m.c[0].r[i] = vector_elements[i]->m.c[0].r[0];
				if (vector_elements[i]->specialization)
					m.c[0].id[i] = vector_elements[i]->self;
			}
		}
	}

	uint32_t constant_type = 0;
	ConstantMatrix m;

	// If this constant is a specialization constant (i.e. created with OpSpecConstant*).
	bool specialization = false;
	// If this constant is used as an array length which creates specialization restrictions on some backends.
	bool is_used_as_array_length = false;

	// If true, this is a LUT, and should always be declared in the outer scope.
	bool is_used_as_lut = false;

	// For composites which are constant arrays, etc.
	SmallVector<uint32_t> subconstants;

	// Non-Vulkan GLSL, HLSL and sometimes MSL emits defines for each specialization constant,
	// and uses them to initialize the constant. This allows the user
	// to still be able to specialize the value by supplying corresponding
	// preprocessor directives before compiling the shader.
	std::string specialization_constant_macro_name;

	SPIRV_CROSS_DECLARE_CLONE(SPIRConstant)
};

// Variants have a very specific allocation scheme.
struct ObjectPoolGroup
{
	std::unique_ptr<ObjectPoolBase> pools[TypeCount];
};

class Variant
{
public:
	explicit Variant(ObjectPoolGroup *group_)
	    : group(group_)
	{
	}

	~Variant()
	{
		if (holder)
			group->pools[type]->free_opaque(holder);
	}

	// Marking custom move constructor as noexcept is important.
	Variant(Variant &&other) SPIRV_CROSS_NOEXCEPT
	{
		*this = std::move(other);
	}

	// We cannot copy from other variant without our own pool group.
	// Have to explicitly copy.
	Variant(const Variant &variant) = delete;

	// Marking custom move constructor as noexcept is important.
	Variant &operator=(Variant &&other) SPIRV_CROSS_NOEXCEPT
	{
		if (this != &other)
		{
			if (holder)
				group->pools[type]->free_opaque(holder);
			holder = other.holder;
			group = other.group;
			type = other.type;
			allow_type_rewrite = other.allow_type_rewrite;

			other.holder = nullptr;
			other.type = TypeNone;
		}
		return *this;
	}

	// This copy/clone should only be called in the Compiler constructor.
	// If this is called inside ::compile(), we invalidate any references we took higher in the stack.
	// This should never happen.
	Variant &operator=(const Variant &other)
	{
#ifdef SPIRV_CROSS_COPY_CONSTRUCTOR_SANITIZE
		abort();
#endif
		if (this != &other)
		{
			if (holder)
				group->pools[type]->free_opaque(holder);

			if (other.holder)
				holder = other.holder->clone(group->pools[other.type].get());

			type = other.type;
			allow_type_rewrite = other.allow_type_rewrite;
		}
		return *this;
	}

	void set(IVariant *val, Types new_type)
	{
		if (holder)
			group->pools[type]->free_opaque(holder);
		holder = val;
		if (!allow_type_rewrite && type != TypeNone && type != new_type)
			SPIRV_CROSS_THROW("Overwriting a variant with new type.");
		type = new_type;
		allow_type_rewrite = false;
	}

	template <typename T, typename... Ts>
	T *allocate_and_set(Types new_type, Ts &&... ts)
	{
		T *val = static_cast<ObjectPool<T> &>(*group->pools[new_type]).allocate(std::forward<Ts>(ts)...);
		set(val, new_type);
		return val;
	}

	template <typename T>
	T &get()
	{
		if (!holder)
			SPIRV_CROSS_THROW("nullptr");
		if (static_cast<Types>(T::type) != type)
			SPIRV_CROSS_THROW("Bad cast");
		return *static_cast<T *>(holder);
	}

	template <typename T>
	const T &get() const
	{
		if (!holder)
			SPIRV_CROSS_THROW("nullptr");
		if (static_cast<Types>(T::type) != type)
			SPIRV_CROSS_THROW("Bad cast");
		return *static_cast<const T *>(holder);
	}

	Types get_type() const
	{
		return type;
	}

	uint32_t get_id() const
	{
		return holder ? holder->self : 0;
	}

	bool empty() const
	{
		return !holder;
	}

	void reset()
	{
		if (holder)
			group->pools[type]->free_opaque(holder);
		holder = nullptr;
		type = TypeNone;
	}

	void set_allow_type_rewrite()
	{
		allow_type_rewrite = true;
	}

private:
	ObjectPoolGroup *group = nullptr;
	IVariant *holder = nullptr;
	Types type = TypeNone;
	bool allow_type_rewrite = false;
};

template <typename T>
T &variant_get(Variant &var)
{
	return var.get<T>();
}

template <typename T>
const T &variant_get(const Variant &var)
{
	return var.get<T>();
}

template <typename T, typename... P>
T &variant_set(Variant &var, P &&... args)
{
	auto *ptr = var.allocate_and_set<T>(static_cast<Types>(T::type), std::forward<P>(args)...);
	return *ptr;
}

struct AccessChainMeta
{
	uint32_t storage_packed_type = 0;
	bool need_transpose = false;
	bool storage_is_packed = false;
	bool storage_is_invariant = false;
};

struct Meta
{
	struct Decoration
	{
		std::string alias;
		std::string qualified_alias;
		std::string hlsl_semantic;
		Bitset decoration_flags;
		spv::BuiltIn builtin_type = spv::BuiltInMax;
		uint32_t location = 0;
		uint32_t component = 0;
		uint32_t set = 0;
		uint32_t binding = 0;
		uint32_t offset = 0;
		uint32_t array_stride = 0;
		uint32_t matrix_stride = 0;
		uint32_t input_attachment = 0;
		uint32_t spec_id = 0;
		uint32_t index = 0;
		spv::FPRoundingMode fp_rounding_mode = spv::FPRoundingModeMax;
		bool builtin = false;

		struct
		{
			uint32_t packed_type = 0;
			bool packed = false;
			uint32_t ib_member_index = ~(0u);
			uint32_t ib_orig_id = 0;
			uint32_t argument_buffer_id = ~(0u);
		} extended;
	};

	Decoration decoration;

	// Intentionally not a SmallVector. Decoration is large and somewhat rare.
	Vector<Decoration> members;

	std::unordered_map<uint32_t, uint32_t> decoration_word_offset;

	// For SPV_GOOGLE_hlsl_functionality1.
	bool hlsl_is_magic_counter_buffer = false;
	// ID for the sibling counter buffer.
	uint32_t hlsl_magic_counter_buffer = 0;
};

// A user callback that remaps the type of any variable.
// var_name is the declared name of the variable.
// name_of_type is the textual name of the type which will be used in the code unless written to by the callback.
using VariableTypeRemapCallback =
    std::function<void(const SPIRType &type, const std::string &var_name, std::string &name_of_type)>;

class Hasher
{
public:
	inline void u32(uint32_t value)
	{
		h = (h * 0x100000001b3ull) ^ value;
	}

	inline uint64_t get() const
	{
		return h;
	}

private:
	uint64_t h = 0xcbf29ce484222325ull;
};

static inline bool type_is_floating_point(const SPIRType &type)
{
	return type.basetype == SPIRType::Half || type.basetype == SPIRType::Float || type.basetype == SPIRType::Double;
}

static inline bool type_is_integral(const SPIRType &type)
{
	return type.basetype == SPIRType::SByte || type.basetype == SPIRType::UByte || type.basetype == SPIRType::Short ||
	       type.basetype == SPIRType::UShort || type.basetype == SPIRType::Int || type.basetype == SPIRType::UInt ||
	       type.basetype == SPIRType::Int64 || type.basetype == SPIRType::UInt64;
}

static inline SPIRType::BaseType to_signed_basetype(uint32_t width)
{
	switch (width)
	{
	case 8:
		return SPIRType::SByte;
	case 16:
		return SPIRType::Short;
	case 32:
		return SPIRType::Int;
	case 64:
		return SPIRType::Int64;
	default:
		SPIRV_CROSS_THROW("Invalid bit width.");
	}
}

static inline SPIRType::BaseType to_unsigned_basetype(uint32_t width)
{
	switch (width)
	{
	case 8:
		return SPIRType::UByte;
	case 16:
		return SPIRType::UShort;
	case 32:
		return SPIRType::UInt;
	case 64:
		return SPIRType::UInt64;
	default:
		SPIRV_CROSS_THROW("Invalid bit width.");
	}
}

// Returns true if an arithmetic operation does not change behavior depending on signedness.
static inline bool opcode_is_sign_invariant(spv::Op opcode)
{
	switch (opcode)
	{
	case spv::OpIEqual:
	case spv::OpINotEqual:
	case spv::OpISub:
	case spv::OpIAdd:
	case spv::OpIMul:
	case spv::OpShiftLeftLogical:
	case spv::OpBitwiseOr:
	case spv::OpBitwiseXor:
	case spv::OpBitwiseAnd:
		return true;

	default:
		return false;
	}
}
} // namespace SPIRV_CROSS_NAMESPACE

#endif
