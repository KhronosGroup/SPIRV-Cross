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
    float4 A;
    float2 B0;
    float2 B1;
    float C0;
    float3 C1;
    packed_float3 D0;
    float D1;
    float E0;
    float E1;
    float E2;
    float E3;
    float F0;
    float2 F1;
    float F2;
};

struct main0_out
{
    float4 oA [[user(locn0)]];
    float4 oB [[user(locn1)]];
    float4 oC [[user(locn2)]];
    float4 oD [[user(locn3)]];
    float4 oE [[user(locn4)]];
    float4 oF [[user(locn5)]];
    float4 gl_Position [[position]];
};

vertex main0_out main0(constant UBO& _22 [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = float4(0.0);
    out.oA = _22.A;
    out.oB = float4(_22.B0, _22.B1);
    out.oC = float4(_22.C0, _22.C1) + float4(_22.C1.xy, _22.C1.z, _22.C0);
    out.oD = float4(_22.D0[0], _22.D0[1], _22.D0[2], _22.D1) + float4(_22.D0[0], _22.D0[1], _22.D0[2u], _22.D1);
    out.oE = float4(_22.E0, _22.E1, _22.E2, _22.E3);
    out.oF = float4(_22.F0, _22.F1, _22.F2);
    return out;
}

