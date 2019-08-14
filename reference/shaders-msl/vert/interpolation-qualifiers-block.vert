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

struct Output
{
    float2 v0;
    float2 v1;
    float3 v2;
    float4 v3;
    float v4;
    float v5;
    float v6;
};

struct main0_out
{
    float2 Output_v0 [[user(locn0)]];
    float2 Output_v1 [[user(locn1)]];
    float3 Output_v2 [[user(locn2)]];
    float4 Output_v3 [[user(locn3)]];
    float Output_v4 [[user(locn4)]];
    float Output_v5 [[user(locn5)]];
    float Output_v6 [[user(locn6)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 Position [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    Output outp = {};
    outp.v0 = in.Position.xy;
    outp.v1 = in.Position.zw;
    outp.v2 = float3(in.Position.x, in.Position.z * in.Position.y, in.Position.x);
    outp.v3 = in.Position.xxyy;
    outp.v4 = in.Position.w;
    outp.v5 = in.Position.y;
    outp.v6 = in.Position.x * in.Position.w;
    out.gl_Position = in.Position;
    out.Output_v0 = outp.v0;
    out.Output_v1 = outp.v1;
    out.Output_v2 = outp.v2;
    out.Output_v3 = outp.v3;
    out.Output_v4 = outp.v4;
    out.Output_v5 = outp.v5;
    out.Output_v6 = outp.v6;
    return out;
}

