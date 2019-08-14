#pragma clang diagnostic ignored "-Wmissing-prototypes"
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

struct Foo
{
    float3 a;
    float b;
};

struct Foo_1
{
    packed_float3 a;
    float b;
};

struct buf
{
    unsafe_array<Foo_1,16> results;
    float4 bar;
};

struct main0_out
{
    float4 _entryPointOutput [[color(0)]];
};

static inline __attribute__((always_inline))
float4 _main(thread const float4& pos, constant buf& v_11)
{
    int _46 = int(pos.x) % 16;
    Foo foo;
    foo.a = float3(v_11.results[_46].a);
    foo.b = v_11.results[_46].b;
    return float4(dot(foo.a, v_11.bar.xyz), foo.b, 0.0, 0.0);
}

fragment main0_out main0(constant buf& v_11 [[buffer(0)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    float4 pos = gl_FragCoord;
    float4 param = pos;
    out._entryPointOutput = _main(param, v_11);
    return out;
}

