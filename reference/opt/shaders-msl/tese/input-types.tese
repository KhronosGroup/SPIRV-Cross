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

struct Block
{
    float4 a;
    float4 b;
};

struct PatchBlock
{
    float4 a;
    float4 b;
};

struct Foo
{
    float4 a;
    float4 b;
};

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 vColor [[attribute(0)]];
    float4 Block_a [[attribute(4)]];
    float4 Block_b [[attribute(5)]];
    float4 Foo_a [[attribute(14)]];
    float4 Foo_b [[attribute(15)]];
};

struct main0_patchIn
{
    float4 vColors [[attribute(1)]];
    float4 PatchBlock_a [[attribute(6)]];
    float4 PatchBlock_b [[attribute(7)]];
    float4 Foo_a [[attribute(8)]];
    float4 Foo_b [[attribute(9)]];
    patch_control_point<main0_in> gl_in;
};

[[ patch(quad, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]])
{
    main0_out out = {};
    PatchBlock patch_block = {};
    Foo vFoo = {};
    patch_block.a = patchIn.PatchBlock_a;
    patch_block.b = patchIn.PatchBlock_b;
    vFoo.a = patchIn.Foo_a;
    vFoo.b = patchIn.Foo_b;
    out.gl_Position = patchIn.gl_in[0].Block_a;
    out.gl_Position += patchIn.gl_in[0].Block_b;
    out.gl_Position += patchIn.gl_in[1].Block_a;
    out.gl_Position += patchIn.gl_in[1].Block_b;
    out.gl_Position += patch_block.a;
    out.gl_Position += patch_block.b;
    out.gl_Position += patchIn.gl_in[0].vColor;
    out.gl_Position += patchIn.gl_in[1].vColor;
    out.gl_Position += patchIn.vColors;
    out.gl_Position += vFoo.a;
    out.gl_Position += vFoo.b;
    Foo vFoos_202;
    vFoos_202.a = patchIn.gl_in[0].Foo_a;
    vFoos_202.b = patchIn.gl_in[0].Foo_b;
    out.gl_Position += vFoos_202.a;
    out.gl_Position += vFoos_202.b;
    Foo vFoos_216;
    vFoos_216.a = patchIn.gl_in[1].Foo_a;
    vFoos_216.b = patchIn.gl_in[1].Foo_b;
    out.gl_Position += vFoos_216.a;
    out.gl_Position += vFoos_216.b;
    return out;
}

