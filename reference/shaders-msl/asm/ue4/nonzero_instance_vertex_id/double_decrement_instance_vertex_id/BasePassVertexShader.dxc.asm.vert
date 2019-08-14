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

constant unsafe_array<float2,2> _67 = { float2(0.0), float2(0.0) };

constant float3x3 _68 = {};
constant float4 _69 = {};

struct main0_out
{
    float4 out_var_TEXCOORD10_centroid [[user(locn0)]];
    float4 out_var_TEXCOORD11_centroid [[user(locn1)]];
    float4 out_var_COLOR0 [[user(locn2)]];
    float2 out_var_TEXCOORD0_0 [[user(locn3)]];
    float2 out_var_TEXCOORD0_1 [[user(locn4)]];
    float4 out_var_VS_To_DS_Position [[user(locn5)]];
};

struct main0_in
{
    float4 in_var_ATTRIBUTE0 [[attribute(0)]];
    float3 in_var_ATTRIBUTE1 [[attribute(1)]];
    float4 in_var_ATTRIBUTE2 [[attribute(2)]];
    uint4 in_var_ATTRIBUTE3 [[attribute(3)]];
    float4 in_var_ATTRIBUTE4 [[attribute(4)]];
    float2 in_var_ATTRIBUTE5_0 [[attribute(5)]];
    float2 in_var_ATTRIBUTE5_1 [[attribute(6)]];
    float4 in_var_ATTRIBUTE13 [[attribute(13)]];
};

// Returns 2D texture coords corresponding to 1D texel buffer coords
static inline __attribute__((always_inline))
uint2 spvTexelBufferCoord(uint tc)
{
    return uint2(tc % 4096, tc / 4096);
}

vertex main0_out main0(main0_in in [[stage_in]], constant type_View& View [[buffer(1)]], constant type_Primitive& Primitive [[buffer(2)]], texture2d<float> BoneMatrices [[texture(0)]])
{
    main0_out out = {};
    unsafe_array<float2,2> out_var_TEXCOORD0 = {};
    unsafe_array<float2,2> in_var_ATTRIBUTE5 = {};
    in_var_ATTRIBUTE5[0] = in.in_var_ATTRIBUTE5_0;
    in_var_ATTRIBUTE5[1] = in.in_var_ATTRIBUTE5_1;
    float4 _83 = float4(in.in_var_ATTRIBUTE4.x);
    int _86 = int(in.in_var_ATTRIBUTE3.x) * 3;
    float4 _100 = float4(in.in_var_ATTRIBUTE4.y);
    int _103 = int(in.in_var_ATTRIBUTE3.y) * 3;
    float4 _119 = float4(in.in_var_ATTRIBUTE4.z);
    int _122 = int(in.in_var_ATTRIBUTE3.z) * 3;
    float4 _138 = float4(in.in_var_ATTRIBUTE4.w);
    int _141 = int(in.in_var_ATTRIBUTE3.w) * 3;
    float3x4 _156 = float3x4((((_83 * BoneMatrices.read(spvTexelBufferCoord(uint(_86)))) + (_100 * BoneMatrices.read(spvTexelBufferCoord(uint(_103))))) + (_119 * BoneMatrices.read(spvTexelBufferCoord(uint(_122))))) + (_138 * BoneMatrices.read(spvTexelBufferCoord(uint(_141)))), (((_83 * BoneMatrices.read(spvTexelBufferCoord(uint(_86 + 1)))) + (_100 * BoneMatrices.read(spvTexelBufferCoord(uint(_103 + 1))))) + (_119 * BoneMatrices.read(spvTexelBufferCoord(uint(_122 + 1))))) + (_138 * BoneMatrices.read(spvTexelBufferCoord(uint(_141 + 1)))), (((_83 * BoneMatrices.read(spvTexelBufferCoord(uint(_86 + 2)))) + (_100 * BoneMatrices.read(spvTexelBufferCoord(uint(_103 + 2))))) + (_119 * BoneMatrices.read(spvTexelBufferCoord(uint(_122 + 2))))) + (_138 * BoneMatrices.read(spvTexelBufferCoord(uint(_141 + 2)))));
    float3 _161 = float4(in.in_var_ATTRIBUTE1, 0.0) * _156;
    float3x3 _162 = _68;
    _162[0] = _161;
    float3 _167 = float4(in.in_var_ATTRIBUTE2.xyz, 0.0) * _156;
    float3x3 _168 = _162;
    _168[2] = _167;
    float3x3 _173 = _168;
    _173[1] = cross(_167, _161) * float3(in.in_var_ATTRIBUTE2.w);
    float3 _178 = float4(in.in_var_ATTRIBUTE0.xyz, 1.0) * _156;
    float4 _205 = float4((((Primitive.Primitive_LocalToWorld[0u].xyz * _178.xxx) + (Primitive.Primitive_LocalToWorld[1u].xyz * _178.yyy)) + (Primitive.Primitive_LocalToWorld[2u].xyz * _178.zzz)) + (Primitive.Primitive_LocalToWorld[3u].xyz + float3(View.View_PreViewTranslation)), 1.0);
    unsafe_array<float2,2> _71;
    _71 = in_var_ATTRIBUTE5;
    unsafe_array<float2,2> _72 = { float2(0.0), float2(0.0) };
    for (int _207 = 0; _207 < 2; )
    {
        _72[_207] = _71[_207];
        _207++;
        continue;
    }
    float4 _217 = _69;
    _217.w = 0.0;
    float3x3 _231 = float3x3(Primitive.Primitive_LocalToWorld[0].xyz, Primitive.Primitive_LocalToWorld[1].xyz, Primitive.Primitive_LocalToWorld[2].xyz);
    _231[0] = Primitive.Primitive_LocalToWorld[0].xyz * float3(Primitive.Primitive_InvNonUniformScaleAndDeterminantSign.x);
    float3x3 _235 = _231;
    _235[1] = Primitive.Primitive_LocalToWorld[1].xyz * float3(Primitive.Primitive_InvNonUniformScaleAndDeterminantSign.y);
    float3x3 _239 = _235;
    _239[2] = Primitive.Primitive_LocalToWorld[2].xyz * float3(Primitive.Primitive_InvNonUniformScaleAndDeterminantSign.z);
    float3x3 _240 = _239 * _173;
    float3 _241 = _240[0];
    out.out_var_TEXCOORD10_centroid = float4(_241.x, _241.y, _241.z, _217.w);
    out.out_var_TEXCOORD11_centroid = float4(_240[2], in.in_var_ATTRIBUTE2.w * Primitive.Primitive_InvNonUniformScaleAndDeterminantSign.w);
    out.out_var_COLOR0 = in.in_var_ATTRIBUTE13;
    out_var_TEXCOORD0 = _72;
    out.out_var_VS_To_DS_Position = float4(_205.x, _205.y, _205.z, _205.w);
    out.out_var_TEXCOORD0_0 = out_var_TEXCOORD0[0];
    out.out_var_TEXCOORD0_1 = out_var_TEXCOORD0[1];
    return out;
}

