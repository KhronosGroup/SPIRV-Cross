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

struct Foobar
{
    float a;
    float b;
};

constant unsafe_array<float4,3> _37 = unsafe_array<float4,3>({ float4(1.0), float4(2.0), float4(3.0) });
constant unsafe_array<float4,2> _49 = unsafe_array<float4,2>({ float4(1.0), float4(2.0) });
constant unsafe_array<float4,2> _54 = unsafe_array<float4,2>({ float4(8.0), float4(10.0) });
constant unsafe_array<unsafe_array<float4,2>,2> _55 = unsafe_array<unsafe_array<float4,2>,2>({ unsafe_array<float4,2>({ float4(1.0), float4(2.0) }), unsafe_array<float4,2>({ float4(8.0), float4(10.0) }) });

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    int index [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    unsafe_array<Foobar,2> _75 = unsafe_array<Foobar,2>({ Foobar{ 10.0, 40.0 }, Foobar{ 90.0, 70.0 } });
    
    main0_out out = {};
    out.FragColor = ((_37[in.index] + _55[in.index][in.index + 1]) + float4(30.0)) + float4(_75[in.index].a + _75[in.index].b);
    return out;
}

