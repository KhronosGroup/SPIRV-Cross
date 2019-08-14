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
    float4 Color;
    unsafe_array<float2,2> TexCoords;
};

struct FVertexFactoryInterpolantsVSToDS
{
    FVertexFactoryInterpolantsVSToPS InterpolantsVSToPS;
};

struct FHitProxyVSToDS
{
    FVertexFactoryInterpolantsVSToDS FactoryInterpolants;
    float4 Position;
};

struct FPNTessellationHSToDS
{
    FHitProxyVSToDS PassSpecificData;
    unsafe_array<float4,3> WorldPosition;
    float3 DisplacementScale;
    float TessellationMultiplier;
    float WorldDisplacementMultiplier;
};

struct type_View
{
    float4x4 View_TranslatedWorldToClip;
    float4x4 View_WorldToClip;
    float4x4 View_ClipToWorld;
    float4x4 View_TranslatedWorldToView;
    float4x4 View_ViewToTranslatedWorld;
    float4x4 View_TranslatedWorldToCameraView;
    float4x4 View_CameraViewToTranslatedWorld;
    float4x4 View_ViewToClip;
    float4x4 View_ViewToClipNoAA;
    float4x4 View_ClipToView;
    float4x4 View_ClipToTranslatedWorld;
    float4x4 View_SVPositionToTranslatedWorld;
    float4x4 View_ScreenToWorld;
    float4x4 View_ScreenToTranslatedWorld;
    packed_float3 View_ViewForward;
    float PrePadding_View_908;
    packed_float3 View_ViewUp;
    float PrePadding_View_924;
    packed_float3 View_ViewRight;
    float PrePadding_View_940;
    packed_float3 View_HMDViewNoRollUp;
    float PrePadding_View_956;
    packed_float3 View_HMDViewNoRollRight;
    float PrePadding_View_972;
    float4 View_InvDeviceZToWorldZTransform;
    float4 View_ScreenPositionScaleBias;
    packed_float3 View_WorldCameraOrigin;
    float PrePadding_View_1020;
    packed_float3 View_TranslatedWorldCameraOrigin;
    float PrePadding_View_1036;
    packed_float3 View_WorldViewOrigin;
    float PrePadding_View_1052;
    packed_float3 View_PreViewTranslation;
    float PrePadding_View_1068;
    float4x4 View_PrevProjection;
    float4x4 View_PrevViewProj;
    float4x4 View_PrevViewRotationProj;
    float4x4 View_PrevViewToClip;
    float4x4 View_PrevClipToView;
    float4x4 View_PrevTranslatedWorldToClip;
    float4x4 View_PrevTranslatedWorldToView;
    float4x4 View_PrevViewToTranslatedWorld;
    float4x4 View_PrevTranslatedWorldToCameraView;
    float4x4 View_PrevCameraViewToTranslatedWorld;
    packed_float3 View_PrevWorldCameraOrigin;
    float PrePadding_View_1724;
    packed_float3 View_PrevWorldViewOrigin;
    float PrePadding_View_1740;
    packed_float3 View_PrevPreViewTranslation;
    float PrePadding_View_1756;
    float4x4 View_PrevInvViewProj;
    float4x4 View_PrevScreenToTranslatedWorld;
    float4x4 View_ClipToPrevClip;
    float4 View_TemporalAAJitter;
    float4 View_GlobalClippingPlane;
    float2 View_FieldOfViewWideAngles;
    float2 View_PrevFieldOfViewWideAngles;
    float4 View_ViewRectMin;
    float4 View_ViewSizeAndInvSize;
    float4 View_BufferSizeAndInvSize;
    float4 View_BufferBilinearUVMinMax;
    int View_NumSceneColorMSAASamples;
    float View_PreExposure;
    float View_OneOverPreExposure;
    float PrePadding_View_2076;
    float4 View_DiffuseOverrideParameter;
    float4 View_SpecularOverrideParameter;
    float4 View_NormalOverrideParameter;
    float2 View_RoughnessOverrideParameter;
    float View_PrevFrameGameTime;
    float View_PrevFrameRealTime;
    float View_OutOfBoundsMask;
    float PrePadding_View_2148;
    float PrePadding_View_2152;
    float PrePadding_View_2156;
    packed_float3 View_WorldCameraMovementSinceLastFrame;
    float View_CullingSign;
    float View_NearPlane;
    float View_AdaptiveTessellationFactor;
    float View_GameTime;
    float View_RealTime;
    float View_DeltaTime;
    float View_MaterialTextureMipBias;
    float View_MaterialTextureDerivativeMultiply;
    uint View_Random;
    uint View_FrameNumber;
    uint View_StateFrameIndexMod8;
    uint View_StateFrameIndex;
    float View_CameraCut;
    float View_UnlitViewmodeMask;
    float PrePadding_View_2228;
    float PrePadding_View_2232;
    float PrePadding_View_2236;
    float4 View_DirectionalLightColor;
    packed_float3 View_DirectionalLightDirection;
    float PrePadding_View_2268;
    unsafe_array<float4,2> View_TranslucencyLightingVolumeMin;
    unsafe_array<float4,2> View_TranslucencyLightingVolumeInvSize;
    float4 View_TemporalAAParams;
    float4 View_CircleDOFParams;
    float View_DepthOfFieldSensorWidth;
    float View_DepthOfFieldFocalDistance;
    float View_DepthOfFieldScale;
    float View_DepthOfFieldFocalLength;
    float View_DepthOfFieldFocalRegion;
    float View_DepthOfFieldNearTransitionRegion;
    float View_DepthOfFieldFarTransitionRegion;
    float View_MotionBlurNormalizedToPixel;
    float View_bSubsurfacePostprocessEnabled;
    float View_GeneralPurposeTweak;
    float View_DemosaicVposOffset;
    float PrePadding_View_2412;
    packed_float3 View_IndirectLightingColorScale;
    float View_HDR32bppEncodingMode;
    packed_float3 View_AtmosphericFogSunDirection;
    float View_AtmosphericFogSunPower;
    float View_AtmosphericFogPower;
    float View_AtmosphericFogDensityScale;
    float View_AtmosphericFogDensityOffset;
    float View_AtmosphericFogGroundOffset;
    float View_AtmosphericFogDistanceScale;
    float View_AtmosphericFogAltitudeScale;
    float View_AtmosphericFogHeightScaleRayleigh;
    float View_AtmosphericFogStartDistance;
    float View_AtmosphericFogDistanceOffset;
    float View_AtmosphericFogSunDiscScale;
    float View_AtmosphericFogSunDiscHalfApexAngleRadian;
    float PrePadding_View_2492;
    float4 View_AtmosphericFogSunDiscLuminance;
    uint View_AtmosphericFogRenderMask;
    uint View_AtmosphericFogInscatterAltitudeSampleNum;
    uint PrePadding_View_2520;
    uint PrePadding_View_2524;
    float4 View_AtmosphericFogSunColor;
    packed_float3 View_NormalCurvatureToRoughnessScaleBias;
    float View_RenderingReflectionCaptureMask;
    float4 View_AmbientCubemapTint;
    float View_AmbientCubemapIntensity;
    float View_SkyLightParameters;
    float PrePadding_View_2584;
    float PrePadding_View_2588;
    float4 View_SkyLightColor;
    unsafe_array<float4,7> View_SkyIrradianceEnvironmentMap;
    float View_MobilePreviewMode;
    float View_HMDEyePaddingOffset;
    float View_ReflectionCubemapMaxMip;
    float View_ShowDecalsMask;
    uint View_DistanceFieldAOSpecularOcclusionMode;
    float View_IndirectCapsuleSelfShadowingIntensity;
    float PrePadding_View_2744;
    float PrePadding_View_2748;
    packed_float3 View_ReflectionEnvironmentRoughnessMixingScaleBiasAndLargestWeight;
    int View_StereoPassIndex;
    unsafe_array<float4,4> View_GlobalVolumeCenterAndExtent;
    unsafe_array<float4,4> View_GlobalVolumeWorldToUVAddAndMul;
    float View_GlobalVolumeDimension;
    float View_GlobalVolumeTexelSize;
    float View_MaxGlobalDistance;
    float PrePadding_View_2908;
    int2 View_CursorPosition;
    float View_bCheckerboardSubsurfaceProfileRendering;
    float PrePadding_View_2924;
    packed_float3 View_VolumetricFogInvGridSize;
    float PrePadding_View_2940;
    packed_float3 View_VolumetricFogGridZParams;
    float PrePadding_View_2956;
    float2 View_VolumetricFogSVPosToVolumeUV;
    float View_VolumetricFogMaxDistance;
    float PrePadding_View_2972;
    packed_float3 View_VolumetricLightmapWorldToUVScale;
    float PrePadding_View_2988;
    packed_float3 View_VolumetricLightmapWorldToUVAdd;
    float PrePadding_View_3004;
    packed_float3 View_VolumetricLightmapIndirectionTextureSize;
    float View_VolumetricLightmapBrickSize;
    packed_float3 View_VolumetricLightmapBrickTexelSize;
    float View_StereoIPD;
    float View_IndirectLightingCacheShowFlag;
    float View_EyeToPixelSpreadAngle;
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

constant float4 _123 = {};

struct main0_out
{
    float4 out_var_COLOR0;
    float3 out_var_PN_DisplacementScales;
    unsafe_array<float4,3> out_var_PN_POSITION;
    float out_var_PN_TessellationMultiplier;
    float out_var_PN_WorldDisplacementMultiplier;
    unsafe_array<float2,2> out_var_TEXCOORD0;
    float4 out_var_TEXCOORD10_centroid;
    float4 out_var_TEXCOORD11_centroid;
    float4 out_var_VS_To_DS_Position;
};

struct main0_patchOut
{
    float4 out_var_PN_POSITION9;
};

struct main0_in
{
    float4 in_var_TEXCOORD10_centroid [[attribute(0)]];
    float4 in_var_TEXCOORD11_centroid [[attribute(1)]];
    float4 in_var_COLOR0 [[attribute(2)]];
    float2 in_var_TEXCOORD0_0 [[attribute(3)]];
    float2 in_var_TEXCOORD0_1 [[attribute(4)]];
    float4 in_var_VS_To_DS_Position [[attribute(5)]];
};

kernel void main0(main0_in in [[stage_in]], constant type_View& View [[buffer(0)]], constant type_Primitive& Primitive [[buffer(1)]], uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLTriangleTessellationFactorsHalf* spvTessLevel [[buffer(26)]], threadgroup main0_in* gl_in [[threadgroup(0)]])
{
    threadgroup FPNTessellationHSToDS temp_var_hullMainRetVal[3];
    unsafe_array<unsafe_array<float2,12>,2> in_var_TEXCOORD0 = {};
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 3];
    device main0_patchOut& patchOut = spvPatchOut[gl_PrimitiveID];
    if (gl_InvocationID < spvIndirectParams[0])
        gl_in[gl_InvocationID] = in;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (gl_InvocationID >= 3)
        return;
    unsafe_array<FHitProxyVSToDS,12> _226 = { FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ gl_in[0].in_var_TEXCOORD10_centroid, gl_in[0].in_var_TEXCOORD11_centroid, gl_in[0].in_var_COLOR0, { gl_in[0].in_var_TEXCOORD0_0, gl_in[0].in_var_TEXCOORD0_1 } } }, gl_in[0].in_var_VS_To_DS_Position }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ gl_in[1].in_var_TEXCOORD10_centroid, gl_in[1].in_var_TEXCOORD11_centroid, gl_in[1].in_var_COLOR0, { gl_in[1].in_var_TEXCOORD0_0, gl_in[1].in_var_TEXCOORD0_1 } } }, gl_in[1].in_var_VS_To_DS_Position }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ gl_in[2].in_var_TEXCOORD10_centroid, gl_in[2].in_var_TEXCOORD11_centroid, gl_in[2].in_var_COLOR0, { gl_in[2].in_var_TEXCOORD0_0, gl_in[2].in_var_TEXCOORD0_1 } } }, gl_in[2].in_var_VS_To_DS_Position }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ gl_in[3].in_var_TEXCOORD10_centroid, gl_in[3].in_var_TEXCOORD11_centroid, gl_in[3].in_var_COLOR0, { gl_in[3].in_var_TEXCOORD0_0, gl_in[3].in_var_TEXCOORD0_1 } } }, gl_in[3].in_var_VS_To_DS_Position }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ gl_in[4].in_var_TEXCOORD10_centroid, gl_in[4].in_var_TEXCOORD11_centroid, gl_in[4].in_var_COLOR0, { gl_in[4].in_var_TEXCOORD0_0, gl_in[4].in_var_TEXCOORD0_1 } } }, gl_in[4].in_var_VS_To_DS_Position }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ gl_in[5].in_var_TEXCOORD10_centroid, gl_in[5].in_var_TEXCOORD11_centroid, gl_in[5].in_var_COLOR0, { gl_in[5].in_var_TEXCOORD0_0, gl_in[5].in_var_TEXCOORD0_1 } } }, gl_in[5].in_var_VS_To_DS_Position }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ gl_in[6].in_var_TEXCOORD10_centroid, gl_in[6].in_var_TEXCOORD11_centroid, gl_in[6].in_var_COLOR0, { gl_in[6].in_var_TEXCOORD0_0, gl_in[6].in_var_TEXCOORD0_1 } } }, gl_in[6].in_var_VS_To_DS_Position }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ gl_in[7].in_var_TEXCOORD10_centroid, gl_in[7].in_var_TEXCOORD11_centroid, gl_in[7].in_var_COLOR0, { gl_in[7].in_var_TEXCOORD0_0, gl_in[7].in_var_TEXCOORD0_1 } } }, gl_in[7].in_var_VS_To_DS_Position }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ gl_in[8].in_var_TEXCOORD10_centroid, gl_in[8].in_var_TEXCOORD11_centroid, gl_in[8].in_var_COLOR0, { gl_in[8].in_var_TEXCOORD0_0, gl_in[8].in_var_TEXCOORD0_1 } } }, gl_in[8].in_var_VS_To_DS_Position }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ gl_in[9].in_var_TEXCOORD10_centroid, gl_in[9].in_var_TEXCOORD11_centroid, gl_in[9].in_var_COLOR0, { gl_in[9].in_var_TEXCOORD0_0, gl_in[9].in_var_TEXCOORD0_1 } } }, gl_in[9].in_var_VS_To_DS_Position }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ gl_in[10].in_var_TEXCOORD10_centroid, gl_in[10].in_var_TEXCOORD11_centroid, gl_in[10].in_var_COLOR0, { gl_in[10].in_var_TEXCOORD0_0, gl_in[10].in_var_TEXCOORD0_1 } } }, gl_in[10].in_var_VS_To_DS_Position }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ gl_in[11].in_var_TEXCOORD10_centroid, gl_in[11].in_var_TEXCOORD11_centroid, gl_in[11].in_var_COLOR0, { gl_in[11].in_var_TEXCOORD0_0, gl_in[11].in_var_TEXCOORD0_1 } } }, gl_in[11].in_var_VS_To_DS_Position } };
    unsafe_array<FHitProxyVSToDS,12> param_var_I;
    param_var_I = _226;
    float4 _243 = float4(param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz, 0.0);
    float3 _247 = Primitive.Primitive_NonUniformScale.xyz * float3x3(param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0.xyz, cross(param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz, param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0.xyz) * float3(param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.w), param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz);
    uint _250 = (gl_InvocationID < 2u) ? (gl_InvocationID + 1u) : 0u;
    uint _251 = 2u * gl_InvocationID;
    uint _252 = 3u + _251;
    uint _253 = _251 + 4u;
    float4 _265 = float4(param_var_I[_250].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz, 0.0);
    float4 _273 = float4(param_var_I[_252].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz, 0.0);
    float4 _281 = float4(param_var_I[_253].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz, 0.0);
    unsafe_array<float4,3> _329 = { param_var_I[gl_InvocationID].Position, (((((float4(2.0) * param_var_I[gl_InvocationID].Position) + param_var_I[_250].Position) - (float4(dot(param_var_I[_250].Position - param_var_I[gl_InvocationID].Position, _243)) * _243)) * float4(0.3333333432674407958984375)) + ((((float4(2.0) * param_var_I[_252].Position) + param_var_I[_253].Position) - (float4(dot(param_var_I[_253].Position - param_var_I[_252].Position, _273)) * _273)) * float4(0.3333333432674407958984375))) * float4(0.5), (((((float4(2.0) * param_var_I[_250].Position) + param_var_I[gl_InvocationID].Position) - (float4(dot(param_var_I[gl_InvocationID].Position - param_var_I[_250].Position, _265)) * _265)) * float4(0.3333333432674407958984375)) + ((((float4(2.0) * param_var_I[_253].Position) + param_var_I[_252].Position) - (float4(dot(param_var_I[_252].Position - param_var_I[_253].Position, _281)) * _281)) * float4(0.3333333432674407958984375))) * float4(0.5) };
    gl_out[gl_InvocationID].out_var_TEXCOORD10_centroid = param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0;
    gl_out[gl_InvocationID].out_var_TEXCOORD11_centroid = param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2;
    gl_out[gl_InvocationID].out_var_COLOR0 = param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.Color;
    gl_out[gl_InvocationID].out_var_TEXCOORD0 = param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TexCoords;
    gl_out[gl_InvocationID].out_var_VS_To_DS_Position = param_var_I[gl_InvocationID].Position;
    gl_out[gl_InvocationID].out_var_PN_POSITION = _329;
    gl_out[gl_InvocationID].out_var_PN_DisplacementScales = _247;
    gl_out[gl_InvocationID].out_var_PN_TessellationMultiplier = 1.0;
    gl_out[gl_InvocationID].out_var_PN_WorldDisplacementMultiplier = 1.0;
    temp_var_hullMainRetVal[gl_InvocationID] = FPNTessellationHSToDS{ param_var_I[gl_InvocationID], _329, _247, 1.0, 1.0 };
    threadgroup_barrier(mem_flags::mem_device | mem_flags::mem_threadgroup);
    if (gl_InvocationID == 0u)
    {
        float4 _359 = temp_var_hullMainRetVal[0u].WorldPosition[0];
        float4 _361 = temp_var_hullMainRetVal[0u].WorldPosition[1];
        float4 _363 = temp_var_hullMainRetVal[0u].WorldPosition[2];
        float4 _365 = temp_var_hullMainRetVal[1u].WorldPosition[0];
        float4 _367 = temp_var_hullMainRetVal[1u].WorldPosition[1];
        float4 _369 = temp_var_hullMainRetVal[1u].WorldPosition[2];
        float4 _371 = temp_var_hullMainRetVal[2u].WorldPosition[0];
        float4 _373 = temp_var_hullMainRetVal[2u].WorldPosition[1];
        float4 _375 = temp_var_hullMainRetVal[2u].WorldPosition[2];
        float4 _381 = (((((_361 + _363) + _367) + _369) + _373) + _375) * float4(0.16666667163372039794921875);
        float4 _394 = _123;
        _394.x = 0.5 * (temp_var_hullMainRetVal[1u].TessellationMultiplier + temp_var_hullMainRetVal[2u].TessellationMultiplier);
        float4 _400 = _394;
        _400.y = 0.5 * (temp_var_hullMainRetVal[2u].TessellationMultiplier + temp_var_hullMainRetVal[0u].TessellationMultiplier);
        float4 _405 = _400;
        _405.z = 0.5 * (temp_var_hullMainRetVal[0u].TessellationMultiplier + temp_var_hullMainRetVal[1u].TessellationMultiplier);
        float4 _412 = _405;
        _412.w = 0.333000004291534423828125 * ((temp_var_hullMainRetVal[0u].TessellationMultiplier + temp_var_hullMainRetVal[1u].TessellationMultiplier) + temp_var_hullMainRetVal[2u].TessellationMultiplier);
        float4 _520;
        for (;;)
        {
            float4 _420 = View.View_ViewToClip * float4(0.0);
            float4 _425 = View.View_TranslatedWorldToClip * float4(_359.xyz, 1.0);
            float3 _426 = _425.xyz;
            float3 _427 = _420.xyz;
            float _429 = _425.w;
            float _430 = _420.w;
            bool3 _433 = (_426 - _427) < float3(_429 + _430);
            bool3 _439 = (_426 + _427) > float3((-_429) - _430);
            float4 _447 = View.View_TranslatedWorldToClip * float4(_365.xyz, 1.0);
            float3 _448 = _447.xyz;
            float _450 = _447.w;
            bool3 _453 = (_448 - _427) < float3(_450 + _430);
            bool3 _459 = (_448 + _427) > float3((-_450) - _430);
            float4 _468 = View.View_TranslatedWorldToClip * float4(_371.xyz, 1.0);
            float3 _469 = _468.xyz;
            float _471 = _468.w;
            bool3 _474 = (_469 - _427) < float3(_471 + _430);
            bool3 _480 = (_469 + _427) > float3((-_471) - _430);
            if (any((((int3(_433.x ? int3(1).x : int3(0).x, _433.y ? int3(1).y : int3(0).y, _433.z ? int3(1).z : int3(0).z) + (int3(2) * int3(_439.x ? int3(1).x : int3(0).x, _439.y ? int3(1).y : int3(0).y, _439.z ? int3(1).z : int3(0).z))) | (int3(_453.x ? int3(1).x : int3(0).x, _453.y ? int3(1).y : int3(0).y, _453.z ? int3(1).z : int3(0).z) + (int3(2) * int3(_459.x ? int3(1).x : int3(0).x, _459.y ? int3(1).y : int3(0).y, _459.z ? int3(1).z : int3(0).z)))) | (int3(_474.x ? int3(1).x : int3(0).x, _474.y ? int3(1).y : int3(0).y, _474.z ? int3(1).z : int3(0).z) + (int3(2) * int3(_480.x ? int3(1).x : int3(0).x, _480.y ? int3(1).y : int3(0).y, _480.z ? int3(1).z : int3(0).z)))) != int3(3)))
            {
                _520 = float4(0.0);
                break;
            }
            float3 _489 = _359.xyz - _365.xyz;
            float3 _490 = _365.xyz - _371.xyz;
            float3 _491 = _371.xyz - _359.xyz;
            float3 _494 = (float3(0.5) * (_359.xyz + _365.xyz)) - float3(View.View_TranslatedWorldCameraOrigin);
            float3 _497 = (float3(0.5) * (_365.xyz + _371.xyz)) - float3(View.View_TranslatedWorldCameraOrigin);
            float3 _500 = (float3(0.5) * (_371.xyz + _359.xyz)) - float3(View.View_TranslatedWorldCameraOrigin);
            float _504 = sqrt(dot(_490, _490) / dot(_497, _497));
            float _508 = sqrt(dot(_491, _491) / dot(_500, _500));
            float _512 = sqrt(dot(_489, _489) / dot(_494, _494));
            float4 _517 = float4(_504, _508, _512, 1.0);
            _517.w = 0.333000004291534423828125 * ((_504 + _508) + _512);
            _520 = float4(View.View_AdaptiveTessellationFactor) * _517;
            break;
        }
        float4 _522 = fast::clamp(_412 * _520, float4(1.0), float4(15.0));
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0u] = half(_522.x);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1u] = half(_522.y);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2u] = half(_522.z);
        spvTessLevel[gl_PrimitiveID].insideTessellationFactor = half(_522.w);
        patchOut.out_var_PN_POSITION9 = _381 + ((_381 - (((_371 + _365) + _359) * float4(0.3333333432674407958984375))) * float4(0.5));
    }
}

