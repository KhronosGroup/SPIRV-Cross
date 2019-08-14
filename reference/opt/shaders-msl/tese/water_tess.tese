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
    float4x4 uMVP;
    float4 uScale;
    float2 uInvScale;
    float3 uCamPos;
    float2 uPatchSize;
    float2 uInvHeightmapSize;
};

struct main0_out
{
    float3 vWorld [[user(locn0)]];
    float4 vGradNormalTex [[user(locn1)]];
    float4 gl_Position [[position]];
};

struct main0_patchIn
{
    float2 vOutPatchPosBase [[attribute(0)]];
    float4 vPatchLods [[attribute(1)]];
};

[[ patch(quad, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]], constant UBO& _31 [[buffer(0)]], texture2d<float> uHeightmapDisplacement [[texture(0)]], sampler uHeightmapDisplacementSmplr [[sampler(0)]], float2 gl_TessCoord [[position_in_patch]])
{
    main0_out out = {};
    float2 _201 = patchIn.vOutPatchPosBase + (float3(gl_TessCoord, 0).xy * _31.uPatchSize);
    float2 _214 = mix(patchIn.vPatchLods.yx, patchIn.vPatchLods.zw, float2(float3(gl_TessCoord, 0).x));
    float _221 = mix(_214.x, _214.y, float3(gl_TessCoord, 0).y);
    float _223 = floor(_221);
    float2 _125 = _201 * _31.uInvHeightmapSize;
    float2 _141 = _31.uInvHeightmapSize * exp2(_223);
    out.vGradNormalTex = float4(_125 + (_31.uInvHeightmapSize * 0.5), _125 * _31.uScale.zw);
    float3 _253 = mix(uHeightmapDisplacement.sample(uHeightmapDisplacementSmplr, (_125 + (_141 * 0.5)), level(_223)).xyz, uHeightmapDisplacement.sample(uHeightmapDisplacementSmplr, (_125 + (_141 * 1.0)), level(_223 + 1.0)).xyz, float3(_221 - _223));
    float2 _171 = (_201 * _31.uScale.xy) + _253.yz;
    out.vWorld = float3(_171.x, _253.x, _171.y);
    out.gl_Position = _31.uMVP * float4(out.vWorld, 1.0);
    return out;
}

