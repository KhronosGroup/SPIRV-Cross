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

struct main0_out
{
    float4 outFragColor [[color(0)]];
};

struct main0_in
{
    float3 inPos [[user(locn0)]];
    float3 inNormal [[user(locn1)]];
    float4 inInvModelView_0 [[user(locn2)]];
    float4 inInvModelView_1 [[user(locn3)]];
    float4 inInvModelView_2 [[user(locn4)]];
    float4 inInvModelView_3 [[user(locn5)]];
    float inLodBias [[user(locn6)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texturecube<float> samplerColor [[texture(0)]], sampler samplerColorSmplr [[sampler(0)]])
{
    main0_out out = {};
    float4x4 inInvModelView = {};
    inInvModelView[0] = in.inInvModelView_0;
    inInvModelView[1] = in.inInvModelView_1;
    inInvModelView[2] = in.inInvModelView_2;
    inInvModelView[3] = in.inInvModelView_3;
    float3 cI = normalize(in.inPos);
    float3 cR = reflect(cI, normalize(in.inNormal));
    cR = float3((inInvModelView * float4(cR, 0.0)).xyz);
    cR.x *= (-1.0);
    out.outFragColor = samplerColor.sample(samplerColorSmplr, cR, bias(in.inLodBias));
    return out;
}

