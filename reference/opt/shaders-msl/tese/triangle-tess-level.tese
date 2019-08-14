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
    float4 gl_Position [[position]];
};

struct main0_patchIn
{
    float4 gl_TessLevel [[attribute(0)]];
};

[[ patch(triangle, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]], float3 gl_TessCoord [[position_in_patch]])
{
    main0_out out = {};
    unsafe_array<float,2> gl_TessLevelInner = {};
    unsafe_array<float,4> gl_TessLevelOuter = {};
    gl_TessLevelInner[0] = patchIn.gl_TessLevel.w;
    gl_TessLevelOuter[0] = patchIn.gl_TessLevel.x;
    gl_TessLevelOuter[1] = patchIn.gl_TessLevel.y;
    gl_TessLevelOuter[2] = patchIn.gl_TessLevel.z;
    out.gl_Position = float4((gl_TessCoord.x * gl_TessLevelInner[0]) * gl_TessLevelOuter[0], (gl_TessCoord.y * gl_TessLevelInner[0]) * gl_TessLevelOuter[1], (gl_TessCoord.z * gl_TessLevelInner[0]) * gl_TessLevelOuter[2], 1.0);
    return out;
}

