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

struct block
{
    short2 a;
    ushort2 b;
    char2 c;
    uchar2 d;
    half2 e;
};

struct storage
{
    short3 f;
    ushort3 g;
    char3 h;
    uchar3 i;
    half3 j;
};

struct main0_out
{
    short4 p [[user(locn0)]];
    ushort4 q [[user(locn1)]];
    half4 r [[user(locn2)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    short foo [[attribute(0)]];
    ushort bar [[attribute(1)]];
    half baz [[attribute(2)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant block& _26 [[buffer(0)]], const device storage& _53 [[buffer(1)]])
{
    main0_out out = {};
    out.p = short4((int4(int(in.foo)) + int4(int2(_26.a), int2(_26.c))) - int4(int3(_53.f) / int3(_53.h), 1));
    out.q = ushort4((uint4(uint(in.bar)) + uint4(uint2(_26.b), uint2(_26.d))) - uint4(uint3(_53.g) / uint3(_53.i), 1u));
    out.r = half4((float4(float(in.baz)) + float4(float2(_26.e), 0.0, 1.0)) - float4(float3(_53.j), 1.0));
    out.gl_Position = float4(0.0, 0.0, 0.0, 1.0);
    return out;
}

