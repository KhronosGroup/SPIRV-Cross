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

struct UBO
{
    float4x4 uMVP;
    float4 uFloatVec4;
    float3 uFloatVec3;
    float2 uFloatVec2;
    float uFloat;
    int4 uIntVec4;
    int3 uIntVec3;
    int2 uIntVec2;
    int uInt;
};

struct main0_out
{
    float4 vFloatVec4 [[user(locn0)]];
    float3 vFloatVec3 [[user(locn1)]];
    float2 vFloatVec2 [[user(locn2)]];
    float vFloat [[user(locn3)]];
    int4 vIntVec4 [[user(locn4)]];
    int3 vIntVec3 [[user(locn5)]];
    int2 vIntVec2 [[user(locn6)]];
    int vInt [[user(locn7)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 aVertex [[attribute(0)]];
};

// Implementation of the GLSL sign() function for integer types
template<typename T, typename E = typename enable_if<is_integral<T>::value>::type>
static inline __attribute__((always_inline))
T sign(T x)
{
    return select(select(select(x, T(0), x == T(0)), T(1), x > T(0)), T(-1), x < T(0));
}

vertex main0_out main0(main0_in in [[stage_in]], constant UBO& _21 [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = _21.uMVP * in.aVertex;
    out.vFloatVec4 = sign(_21.uFloatVec4);
    out.vFloatVec3 = sign(_21.uFloatVec3);
    out.vFloatVec2 = sign(_21.uFloatVec2);
    out.vFloat = sign(_21.uFloat);
    out.vIntVec4 = sign(_21.uIntVec4);
    out.vIntVec3 = sign(_21.uIntVec3);
    out.vIntVec2 = sign(_21.uIntVec2);
    out.vInt = sign(_21.uInt);
    return out;
}

