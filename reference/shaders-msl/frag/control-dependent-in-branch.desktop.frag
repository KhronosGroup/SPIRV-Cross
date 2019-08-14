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
    float4 vInput [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> uSampler [[texture(0)]], sampler uSamplerSmplr [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor = in.vInput;
    float4 t = uSampler.sample(uSamplerSmplr, in.vInput.xy);
    float4 d0 = dfdx(in.vInput);
    float4 d1 = dfdy(in.vInput);
    float4 d2 = fwidth(in.vInput);
    float4 d3 = dfdx(in.vInput);
    float4 d4 = dfdy(in.vInput);
    float4 d5 = fwidth(in.vInput);
    float4 d6 = dfdx(in.vInput);
    float4 d7 = dfdy(in.vInput);
    float4 d8 = fwidth(in.vInput);
    if (in.vInput.y > 10.0)
    {
        out.FragColor += t;
        out.FragColor += d0;
        out.FragColor += d1;
        out.FragColor += d2;
        out.FragColor += d3;
        out.FragColor += d4;
        out.FragColor += d5;
        out.FragColor += d6;
        out.FragColor += d7;
        out.FragColor += d8;
    }
    return out;
}

