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
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float3 vNormal [[user(locn1)]];
    float2 vUV [[user(locn2)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> samp [[texture(0)]], sampler sampSmplr [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor = float4(samp.sample(sampSmplr, in.vUV).xyz, 1.0);
    out.FragColor = float4(samp.sample(sampSmplr, in.vUV).xz, 1.0, 4.0);
    out.FragColor = float4(samp.sample(sampSmplr, in.vUV).xx, samp.sample(sampSmplr, (in.vUV + float2(0.100000001490116119384765625))).yy);
    out.FragColor = float4(in.vNormal, 1.0);
    out.FragColor = float4(in.vNormal + float3(1.7999999523162841796875), 1.0);
    out.FragColor = float4(in.vUV, in.vUV + float2(1.7999999523162841796875));
    return out;
}

