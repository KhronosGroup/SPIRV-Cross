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

struct _15
{
    float3x4 _m0;
    float3x4 _m1;
};

struct _42
{
    float4x4 _m0;
    float4x4 _m1;
    float _m2;
    char _m3_pad[12];
    packed_float3 _m3;
    float _m4;
    packed_float3 _m5;
    float _m6;
    float _m7;
    float _m8;
    float2 _m9;
};

struct main0_out
{
    float3 m_72 [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 m_25 [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant _15& _17 [[buffer(0)]], constant _42& _44 [[buffer(1)]])
{
    main0_out out = {};
    float3 _91;
    float3 _13;
    do
    {
        _13 = normalize(float4(in.m_25.xyz, 0.0) * _17._m1);
        break;
    } while (false);
    float4 _39 = _44._m0 * float4(float3(_44._m3) + (in.m_25.xyz * (_44._m6 + _44._m7)), 1.0);
    out.m_72 = _13;
    float4 _74 = _39;
    _74.y = -_39.y;
    out.gl_Position = _74;
    return out;
}

