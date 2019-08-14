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
    unsafe_array<float4,64> v;
};

struct SSBO
{
    unsafe_array<float4,1> v;
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    int vIndex [[user(locn0)]];
    float2 vUV [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant UBO* ubos_0 [[buffer(0)]], constant UBO* ubos_1 [[buffer(1)]], const device SSBO* ssbos_0 [[buffer(2)]], const device SSBO* ssbos_1 [[buffer(3)]], array<texture2d<float>, 8> uSamplers [[texture(0)]], array<texture2d<float>, 8> uCombinedSamplers [[texture(8)]], array<sampler, 7> uSamps [[sampler(0)]], array<sampler, 8> uCombinedSamplersSmplr [[sampler(7)]])
{
    unsafe_array<constant UBO*,2> ubos =
    {
        ubos_0,
        ubos_1,
    };

    unsafe_array<const device SSBO*,2> ssbos =
    {
        ssbos_0,
        ssbos_1,
    };

    main0_out out = {};
    int _25 = in.vIndex + 10;
    int _37 = in.vIndex + 40;
    out.FragColor = uSamplers[_25].sample(uSamps[_37], in.vUV);
    out.FragColor = uCombinedSamplers[_25].sample(uCombinedSamplersSmplr[_25], in.vUV);
    int _69 = in.vIndex + 20;
    out.FragColor += ubos[(_69)]->v[_37];
    int _87 = in.vIndex + 50;
    int _91 = in.vIndex + 60;
    out.FragColor += ssbos[(_87)]->v[_91];
    return out;
}

