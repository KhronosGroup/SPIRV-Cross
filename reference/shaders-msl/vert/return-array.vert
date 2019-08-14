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

constant unsafe_array<float4,2> _20 = unsafe_array<float4,2>({ float4(10.0), float4(20.0) });

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 vInput0 [[attribute(0)]];
    float4 vInput1 [[attribute(1)]];
};

static inline __attribute__((always_inline))
void test(thread unsafe_array<float4,2> (&SPIRV_Cross_return_value))
{
    SPIRV_Cross_return_value = _20;
}

static inline __attribute__((always_inline))
void test2(thread unsafe_array<float4,2> (&SPIRV_Cross_return_value), thread float4& vInput0, thread float4& vInput1)
{
    unsafe_array<float4,2> foobar;
    foobar[0] = vInput0;
    foobar[1] = vInput1;
    SPIRV_Cross_return_value = foobar;
}

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    unsafe_array<float4,2> _42;
    test(_42);
    unsafe_array<float4,2> _44;
    test2(_44, in.vInput0, in.vInput1);
    out.gl_Position = _42[0] + _44[1];
    return out;
}

