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
    float4 FragColor0 [[color(0)]];
    float4 FragColor1 [[color(1)]];
};

struct main0_in
{
    float4 VertGeom [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> TextureBase [[texture(0)]], texture2d<float> TextureDetail [[texture(1)]], sampler TextureBaseSmplr [[sampler(0)]], sampler TextureDetailSmplr [[sampler(1)]])
{
    main0_out out = {};
    float4 _20 = TextureBase.sample(TextureBaseSmplr, in.VertGeom.xy);
    float4 _31 = TextureDetail.sample(TextureDetailSmplr, in.VertGeom.xy, int2(3, 2));
    out.FragColor0 = as_type<float4>(as_type<int4>(_20)) * as_type<float4>(as_type<int4>(_31));
    out.FragColor1 = as_type<float4>(as_type<uint4>(_20)) * as_type<float4>(as_type<uint4>(_31));
    return out;
}

