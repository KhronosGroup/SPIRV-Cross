#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"
#pragma clang diagnostic ignored "-Wunused-variable"

#include <metal_stdlib>
#include <simd/simd.h>
	
template <typename T, size_t Num>
struct unsafe_array
{
	T __Elements[Num ? Num : 1];
	
	constexpr size_t size() const thread { return Num; }
	constexpr size_t max_size() const thread { return Num; }
	constexpr bool empty() const thread { return Num == 0; }
	
	constexpr size_t size() const device { return Num; }
	constexpr size_t max_size() const device { return Num; }
	constexpr bool empty() const device { return Num == 0; }
	
	constexpr size_t size() const constant { return Num; }
	constexpr size_t max_size() const constant { return Num; }
	constexpr bool empty() const constant { return Num == 0; }
	
	constexpr size_t size() const threadgroup { return Num; }
	constexpr size_t max_size() const threadgroup { return Num; }
	constexpr bool empty() const threadgroup { return Num == 0; }
	
	thread T &operator[](size_t pos) thread
	{
		return __Elements[pos];
	}
	constexpr const thread T &operator[](size_t pos) const thread
	{
		return __Elements[pos];
	}
	
	device T &operator[](size_t pos) device
	{
		return __Elements[pos];
	}
	constexpr const device T &operator[](size_t pos) const device
	{
		return __Elements[pos];
	}
	
	constexpr const constant T &operator[](size_t pos) const constant
	{
		return __Elements[pos];
	}
	
	threadgroup T &operator[](size_t pos) threadgroup
	{
		return __Elements[pos];
	}
	constexpr const threadgroup T &operator[](size_t pos) const threadgroup
	{
		return __Elements[pos];
	}
};

using namespace metal;

constant unsafe_array<float4,4> _68 = unsafe_array<float4,4>({ float4(0.0), float4(1.0), float4(2.0), float4(3.0) });

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    int Index1 [[attribute(0)]];
    int Index2 [[attribute(1)]];
};

<<<<<<< HEAD
template<typename T, uint A>
inline void spvArrayCopyFromConstantToStack1(thread T (&dst)[A], constant T (&src)[A])
{
    for (uint i = 0; i < A; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint A>
inline void spvArrayCopyFromConstantToThreadGroup1(threadgroup T (&dst)[A], constant T (&src)[A])
{
    for (uint i = 0; i < A; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint A>
inline void spvArrayCopyFromStackToStack1(thread T (&dst)[A], thread const T (&src)[A])
{
    for (uint i = 0; i < A; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint A>
inline void spvArrayCopyFromStackToThreadGroup1(threadgroup T (&dst)[A], thread const T (&src)[A])
{
    for (uint i = 0; i < A; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint A>
inline void spvArrayCopyFromThreadGroupToStack1(thread T (&dst)[A], threadgroup const T (&src)[A])
{
    for (uint i = 0; i < A; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint A>
inline void spvArrayCopyFromThreadGroupToThreadGroup1(threadgroup T (&dst)[A], threadgroup const T (&src)[A])
{
    for (uint i = 0; i < A; i++)
    {
        dst[i] = src[i];
    }
}

inline float4 consume_constant_arrays2(thread const float4 (&positions)[4], thread const float4 (&positions2)[4], thread int& Index1, thread int& Index2)
{
    float4 indexable[4];
    spvArrayCopyFromStackToStack1(indexable, positions);
    float4 indexable_1[4];
    spvArrayCopyFromStackToStack1(indexable_1, positions2);
    return indexable[Index1] + indexable_1[Index2];
}

inline float4 consume_constant_arrays(thread const float4 (&positions)[4], thread const float4 (&positions2)[4], thread int& Index1, thread int& Index2)
=======
static inline __attribute__((always_inline))
float4 consume_constant_arrays2(thread const unsafe_array<float4,4> (&positions), thread const unsafe_array<float4,4> (&positions2), thread int& Index1, thread int& Index2)
{
    unsafe_array<float4,4> indexable;
    indexable = positions;
    unsafe_array<float4,4> indexable_1;
    indexable_1 = positions2;
    return indexable[Index1] + indexable_1[Index2];
}

static inline __attribute__((always_inline))
float4 consume_constant_arrays(thread const unsafe_array<float4,4> (&positions), thread const unsafe_array<float4,4> (&positions2), thread int& Index1, thread int& Index2)
>>>>>>> 22755a0c... Update the Metal shaders to account for changes in the shader compilation.
{
    return consume_constant_arrays2(positions, positions2, Index1, Index2);
}

vertex main0_out main0(main0_in in [[stage_in]])
{
    unsafe_array<float4,4> _68_array_copy = unsafe_array<float4,4>({ float4(0.0), float4(1.0), float4(2.0), float4(3.0) });
    main0_out out = {};
    unsafe_array<float4,4> LUT2;
    LUT2[0] = float4(10.0);
    LUT2[1] = float4(11.0);
    LUT2[2] = float4(12.0);
    LUT2[3] = float4(13.0);
    out.gl_Position = consume_constant_arrays(_68_array_copy, LUT2, in.Index1, in.Index2);
    return out;
}

