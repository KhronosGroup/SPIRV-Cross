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
    float2 vTex [[user(locn0), flat]];
    int vIndex [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]], array<texture2d<float>, 4> uSampler [[texture(0)]], array<texture2d<float>, 4> uTextures [[texture(4)]], array<sampler, 4> uSamplerSmplr [[sampler(0)]], array<sampler, 4> uSamplers [[sampler(4)]])
{
    main0_out out = {};
    out.FragColor = float4(0.0);
    out.FragColor += uTextures[2].sample(uSamplers[1], in.vTex);
    out.FragColor += uSampler[in.vIndex].sample(uSamplerSmplr[in.vIndex], in.vTex);
    out.FragColor += uSampler[in.vIndex].sample(uSamplerSmplr[in.vIndex], (in.vTex + float2(0.100000001490116119384765625)));
    out.FragColor += uSampler[in.vIndex].sample(uSamplerSmplr[in.vIndex], (in.vTex + float2(0.20000000298023223876953125)));
    out.FragColor += uSampler[3].sample(uSamplerSmplr[3], (in.vTex + float2(0.300000011920928955078125)));
    return out;
}

