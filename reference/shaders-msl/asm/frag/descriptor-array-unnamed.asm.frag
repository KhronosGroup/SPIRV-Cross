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

struct _4
{
    float4 _m0;
};

struct _6
{
    int _m0;
};

struct _7
{
    float4 _m0;
};

struct main0_out
{
    float4 m_3 [[color(0)]];
};

fragment main0_out main0(const device _4* _5_0 [[buffer(0)]], const device _4* _5_1 [[buffer(1)]], const device _4* _5_2 [[buffer(2)]], const device _4* _5_3 [[buffer(3)]], constant _6& _20 [[buffer(4)]], constant _7* _8_0 [[buffer(5)]], constant _7* _8_1 [[buffer(6)]], constant _7* _8_2 [[buffer(7)]], constant _7* _8_3 [[buffer(8)]])
{
    unsafe_array<const device _4*,4> _5 =
    {
        _5_0,
        _5_1,
        _5_2,
        _5_3,
    };

    unsafe_array<constant _7*,4> _8 =
    {
        _8_0,
        _8_1,
        _8_2,
        _8_3,
    };

    main0_out out = {};
    out.m_3 = _5[_20._m0]->_m0 + (_8[_20._m0]->_m0 * float4(0.20000000298023223876953125));
    return out;
}

