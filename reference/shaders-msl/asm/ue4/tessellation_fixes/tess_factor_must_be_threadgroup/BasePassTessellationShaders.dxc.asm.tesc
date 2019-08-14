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

struct FVertexFactoryInterpolantsVSToPS
{
    float4 TangentToWorld0;
    float4 TangentToWorld2;
};

struct FVertexFactoryInterpolantsVSToDS
{
    FVertexFactoryInterpolantsVSToPS InterpolantsVSToPS;
};

struct FSharedBasePassInterpolants
{
    int empty_struct_member;
};

struct FBasePassInterpolantsVSToDS
{
    FSharedBasePassInterpolants _m0;
};

struct FBasePassVSToDS
{
    FVertexFactoryInterpolantsVSToDS FactoryInterpolants;
    FBasePassInterpolantsVSToDS BasePassInterpolants;
    float4 Position;
};

struct FFlatTessellationHSToDS
{
    FBasePassVSToDS PassSpecificData;
    float3 DisplacementScale;
    float TessellationMultiplier;
    float WorldDisplacementMultiplier;
};

struct type_Primitive
{
    float4x4 Primitive_LocalToWorld;
    float4 Primitive_InvNonUniformScaleAndDeterminantSign;
    float4 Primitive_ObjectWorldPositionAndRadius;
    float4x4 Primitive_WorldToLocal;
    float4x4 Primitive_PreviousLocalToWorld;
    float4x4 Primitive_PreviousWorldToLocal;
    packed_float3 Primitive_ActorWorldPosition;
    float Primitive_UseSingleSampleShadowFromStationaryLights;
    packed_float3 Primitive_ObjectBounds;
    float Primitive_LpvBiasMultiplier;
    float Primitive_DecalReceiverMask;
    float Primitive_PerObjectGBufferData;
    float Primitive_UseVolumetricLightmapShadowFromStationaryLights;
    float Primitive_DrawsVelocity;
    float4 Primitive_ObjectOrientation;
    float4 Primitive_NonUniformScale;
    packed_float3 Primitive_LocalObjectBoundsMin;
    uint Primitive_LightingChannelMask;
    packed_float3 Primitive_LocalObjectBoundsMax;
    uint Primitive_LightmapDataIndex;
    packed_float3 Primitive_PreSkinnedLocalBounds;
    int Primitive_SingleCaptureIndex;
    uint Primitive_OutputVelocity;
    uint PrePadding_Primitive_420;
    uint PrePadding_Primitive_424;
    uint PrePadding_Primitive_428;
    unsafe_array<float4,4> Primitive_CustomPrimitiveData;
};

struct type_Material
{
    unsafe_array<float4,3> Material_VectorExpressions;
    unsafe_array<float4,1> Material_ScalarExpressions;
};

constant float4 _85 = {};

struct main0_out
{
    float3 out_var_Flat_DisplacementScales;
    float out_var_Flat_TessellationMultiplier;
    float out_var_Flat_WorldDisplacementMultiplier;
    float4 out_var_TEXCOORD10_centroid;
    float4 out_var_TEXCOORD11_centroid;
    float4 out_var_VS_To_DS_Position;
};

struct main0_in
{
    float4 in_var_TEXCOORD10_centroid [[attribute(0)]];
    float4 in_var_TEXCOORD11_centroid [[attribute(1)]];
    float4 in_var_VS_To_DS_Position [[attribute(2)]];
};

kernel void main0(main0_in in [[stage_in]], constant type_Primitive& Primitive [[buffer(0)]], constant type_Material& Material [[buffer(1)]], uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device MTLTriangleTessellationFactorsHalf* spvTessLevel [[buffer(26)]], threadgroup main0_in* gl_in [[threadgroup(0)]])
{
    threadgroup FFlatTessellationHSToDS temp_var_hullMainRetVal[3];
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 3];
    if (gl_InvocationID < spvIndirectParams[0])
        gl_in[gl_InvocationID] = in;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (gl_InvocationID >= 3)
        return;
    unsafe_array<FBasePassVSToDS,3> _108 = { FBasePassVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ gl_in[0].in_var_TEXCOORD10_centroid, gl_in[0].in_var_TEXCOORD11_centroid } }, FBasePassInterpolantsVSToDS{ { } }, gl_in[0].in_var_VS_To_DS_Position }, FBasePassVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ gl_in[1].in_var_TEXCOORD10_centroid, gl_in[1].in_var_TEXCOORD11_centroid } }, FBasePassInterpolantsVSToDS{ { } }, gl_in[1].in_var_VS_To_DS_Position }, FBasePassVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ gl_in[2].in_var_TEXCOORD10_centroid, gl_in[2].in_var_TEXCOORD11_centroid } }, FBasePassInterpolantsVSToDS{ { } }, gl_in[2].in_var_VS_To_DS_Position } };
    unsafe_array<FBasePassVSToDS,3> param_var_I;
    param_var_I = _108;
    float3 _125 = Primitive.Primitive_NonUniformScale.xyz * float3x3(param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0.xyz, cross(param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz, param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0.xyz) * float3(param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.w), param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz);
    gl_out[gl_InvocationID].out_var_TEXCOORD10_centroid = param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0;
    gl_out[gl_InvocationID].out_var_TEXCOORD11_centroid = param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2;
    gl_out[gl_InvocationID].out_var_VS_To_DS_Position = param_var_I[gl_InvocationID].Position;
    gl_out[gl_InvocationID].out_var_Flat_DisplacementScales = _125;
    gl_out[gl_InvocationID].out_var_Flat_TessellationMultiplier = Material.Material_ScalarExpressions[0].x;
    gl_out[gl_InvocationID].out_var_Flat_WorldDisplacementMultiplier = 1.0;
    temp_var_hullMainRetVal[gl_InvocationID] = FFlatTessellationHSToDS{ param_var_I[gl_InvocationID], _125, Material.Material_ScalarExpressions[0].x, 1.0 };
    threadgroup_barrier(mem_flags::mem_device | mem_flags::mem_threadgroup);
    if (gl_InvocationID == 0u)
    {
        float4 _151 = _85;
        _151.x = 0.5 * (temp_var_hullMainRetVal[1u].TessellationMultiplier + temp_var_hullMainRetVal[2u].TessellationMultiplier);
        float4 _157 = _151;
        _157.y = 0.5 * (temp_var_hullMainRetVal[2u].TessellationMultiplier + temp_var_hullMainRetVal[0u].TessellationMultiplier);
        float4 _162 = _157;
        _162.z = 0.5 * (temp_var_hullMainRetVal[0u].TessellationMultiplier + temp_var_hullMainRetVal[1u].TessellationMultiplier);
        float4 _169 = _162;
        _169.w = 0.333000004291534423828125 * ((temp_var_hullMainRetVal[0u].TessellationMultiplier + temp_var_hullMainRetVal[1u].TessellationMultiplier) + temp_var_hullMainRetVal[2u].TessellationMultiplier);
        float4 _170 = fast::clamp(_169, float4(1.0), float4(15.0));
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0u] = half(_170.x);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1u] = half(_170.y);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2u] = half(_170.z);
        spvTessLevel[gl_PrimitiveID].insideTessellationFactor = half(_170.w);
    }
}

