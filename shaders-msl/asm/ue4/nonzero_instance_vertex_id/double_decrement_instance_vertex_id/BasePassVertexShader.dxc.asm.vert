; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 251
; Schema: 0
               OpCapability Shader
               OpCapability SampledBuffer
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %Main "main" %in_var_ATTRIBUTE0 %in_var_ATTRIBUTE1 %in_var_ATTRIBUTE2 %in_var_ATTRIBUTE3 %in_var_ATTRIBUTE4 %in_var_ATTRIBUTE5 %in_var_ATTRIBUTE13 %out_var_TEXCOORD10_centroid %out_var_TEXCOORD11_centroid %out_var_COLOR0 %out_var_TEXCOORD0 %out_var_VS_To_DS_Position
               OpSource HLSL 600
               OpName %type_buffer_image "type.buffer.image"
               OpName %type_View "type.View"
               OpMemberName %type_View 0 "View_TranslatedWorldToClip"
               OpMemberName %type_View 1 "View_WorldToClip"
               OpMemberName %type_View 2 "View_ClipToWorld"
               OpMemberName %type_View 3 "View_TranslatedWorldToView"
               OpMemberName %type_View 4 "View_ViewToTranslatedWorld"
               OpMemberName %type_View 5 "View_TranslatedWorldToCameraView"
               OpMemberName %type_View 6 "View_CameraViewToTranslatedWorld"
               OpMemberName %type_View 7 "View_ViewToClip"
               OpMemberName %type_View 8 "View_ViewToClipNoAA"
               OpMemberName %type_View 9 "View_ClipToView"
               OpMemberName %type_View 10 "View_ClipToTranslatedWorld"
               OpMemberName %type_View 11 "View_SVPositionToTranslatedWorld"
               OpMemberName %type_View 12 "View_ScreenToWorld"
               OpMemberName %type_View 13 "View_ScreenToTranslatedWorld"
               OpMemberName %type_View 14 "View_ViewForward"
               OpMemberName %type_View 15 "PrePadding_View_908"
               OpMemberName %type_View 16 "View_ViewUp"
               OpMemberName %type_View 17 "PrePadding_View_924"
               OpMemberName %type_View 18 "View_ViewRight"
               OpMemberName %type_View 19 "PrePadding_View_940"
               OpMemberName %type_View 20 "View_HMDViewNoRollUp"
               OpMemberName %type_View 21 "PrePadding_View_956"
               OpMemberName %type_View 22 "View_HMDViewNoRollRight"
               OpMemberName %type_View 23 "PrePadding_View_972"
               OpMemberName %type_View 24 "View_InvDeviceZToWorldZTransform"
               OpMemberName %type_View 25 "View_ScreenPositionScaleBias"
               OpMemberName %type_View 26 "View_WorldCameraOrigin"
               OpMemberName %type_View 27 "PrePadding_View_1020"
               OpMemberName %type_View 28 "View_TranslatedWorldCameraOrigin"
               OpMemberName %type_View 29 "PrePadding_View_1036"
               OpMemberName %type_View 30 "View_WorldViewOrigin"
               OpMemberName %type_View 31 "PrePadding_View_1052"
               OpMemberName %type_View 32 "View_PreViewTranslation"
               OpMemberName %type_View 33 "PrePadding_View_1068"
               OpMemberName %type_View 34 "View_PrevProjection"
               OpMemberName %type_View 35 "View_PrevViewProj"
               OpMemberName %type_View 36 "View_PrevViewRotationProj"
               OpMemberName %type_View 37 "View_PrevViewToClip"
               OpMemberName %type_View 38 "View_PrevClipToView"
               OpMemberName %type_View 39 "View_PrevTranslatedWorldToClip"
               OpMemberName %type_View 40 "View_PrevTranslatedWorldToView"
               OpMemberName %type_View 41 "View_PrevViewToTranslatedWorld"
               OpMemberName %type_View 42 "View_PrevTranslatedWorldToCameraView"
               OpMemberName %type_View 43 "View_PrevCameraViewToTranslatedWorld"
               OpMemberName %type_View 44 "View_PrevWorldCameraOrigin"
               OpMemberName %type_View 45 "PrePadding_View_1724"
               OpMemberName %type_View 46 "View_PrevWorldViewOrigin"
               OpMemberName %type_View 47 "PrePadding_View_1740"
               OpMemberName %type_View 48 "View_PrevPreViewTranslation"
               OpMemberName %type_View 49 "PrePadding_View_1756"
               OpMemberName %type_View 50 "View_PrevInvViewProj"
               OpMemberName %type_View 51 "View_PrevScreenToTranslatedWorld"
               OpMemberName %type_View 52 "View_ClipToPrevClip"
               OpMemberName %type_View 53 "View_TemporalAAJitter"
               OpMemberName %type_View 54 "View_GlobalClippingPlane"
               OpMemberName %type_View 55 "View_FieldOfViewWideAngles"
               OpMemberName %type_View 56 "View_PrevFieldOfViewWideAngles"
               OpMemberName %type_View 57 "View_ViewRectMin"
               OpMemberName %type_View 58 "View_ViewSizeAndInvSize"
               OpMemberName %type_View 59 "View_BufferSizeAndInvSize"
               OpMemberName %type_View 60 "View_BufferBilinearUVMinMax"
               OpMemberName %type_View 61 "View_NumSceneColorMSAASamples"
               OpMemberName %type_View 62 "View_PreExposure"
               OpMemberName %type_View 63 "View_OneOverPreExposure"
               OpMemberName %type_View 64 "PrePadding_View_2076"
               OpMemberName %type_View 65 "View_DiffuseOverrideParameter"
               OpMemberName %type_View 66 "View_SpecularOverrideParameter"
               OpMemberName %type_View 67 "View_NormalOverrideParameter"
               OpMemberName %type_View 68 "View_RoughnessOverrideParameter"
               OpMemberName %type_View 69 "View_PrevFrameGameTime"
               OpMemberName %type_View 70 "View_PrevFrameRealTime"
               OpMemberName %type_View 71 "View_OutOfBoundsMask"
               OpMemberName %type_View 72 "PrePadding_View_2148"
               OpMemberName %type_View 73 "PrePadding_View_2152"
               OpMemberName %type_View 74 "PrePadding_View_2156"
               OpMemberName %type_View 75 "View_WorldCameraMovementSinceLastFrame"
               OpMemberName %type_View 76 "View_CullingSign"
               OpMemberName %type_View 77 "View_NearPlane"
               OpMemberName %type_View 78 "View_AdaptiveTessellationFactor"
               OpMemberName %type_View 79 "View_GameTime"
               OpMemberName %type_View 80 "View_RealTime"
               OpMemberName %type_View 81 "View_DeltaTime"
               OpMemberName %type_View 82 "View_MaterialTextureMipBias"
               OpMemberName %type_View 83 "View_MaterialTextureDerivativeMultiply"
               OpMemberName %type_View 84 "View_Random"
               OpMemberName %type_View 85 "View_FrameNumber"
               OpMemberName %type_View 86 "View_StateFrameIndexMod8"
               OpMemberName %type_View 87 "View_StateFrameIndex"
               OpMemberName %type_View 88 "View_CameraCut"
               OpMemberName %type_View 89 "View_UnlitViewmodeMask"
               OpMemberName %type_View 90 "PrePadding_View_2228"
               OpMemberName %type_View 91 "PrePadding_View_2232"
               OpMemberName %type_View 92 "PrePadding_View_2236"
               OpMemberName %type_View 93 "View_DirectionalLightColor"
               OpMemberName %type_View 94 "View_DirectionalLightDirection"
               OpMemberName %type_View 95 "PrePadding_View_2268"
               OpMemberName %type_View 96 "View_TranslucencyLightingVolumeMin"
               OpMemberName %type_View 97 "View_TranslucencyLightingVolumeInvSize"
               OpMemberName %type_View 98 "View_TemporalAAParams"
               OpMemberName %type_View 99 "View_CircleDOFParams"
               OpMemberName %type_View 100 "View_DepthOfFieldSensorWidth"
               OpMemberName %type_View 101 "View_DepthOfFieldFocalDistance"
               OpMemberName %type_View 102 "View_DepthOfFieldScale"
               OpMemberName %type_View 103 "View_DepthOfFieldFocalLength"
               OpMemberName %type_View 104 "View_DepthOfFieldFocalRegion"
               OpMemberName %type_View 105 "View_DepthOfFieldNearTransitionRegion"
               OpMemberName %type_View 106 "View_DepthOfFieldFarTransitionRegion"
               OpMemberName %type_View 107 "View_MotionBlurNormalizedToPixel"
               OpMemberName %type_View 108 "View_bSubsurfacePostprocessEnabled"
               OpMemberName %type_View 109 "View_GeneralPurposeTweak"
               OpMemberName %type_View 110 "View_DemosaicVposOffset"
               OpMemberName %type_View 111 "PrePadding_View_2412"
               OpMemberName %type_View 112 "View_IndirectLightingColorScale"
               OpMemberName %type_View 113 "View_HDR32bppEncodingMode"
               OpMemberName %type_View 114 "View_AtmosphericFogSunDirection"
               OpMemberName %type_View 115 "View_AtmosphericFogSunPower"
               OpMemberName %type_View 116 "View_AtmosphericFogPower"
               OpMemberName %type_View 117 "View_AtmosphericFogDensityScale"
               OpMemberName %type_View 118 "View_AtmosphericFogDensityOffset"
               OpMemberName %type_View 119 "View_AtmosphericFogGroundOffset"
               OpMemberName %type_View 120 "View_AtmosphericFogDistanceScale"
               OpMemberName %type_View 121 "View_AtmosphericFogAltitudeScale"
               OpMemberName %type_View 122 "View_AtmosphericFogHeightScaleRayleigh"
               OpMemberName %type_View 123 "View_AtmosphericFogStartDistance"
               OpMemberName %type_View 124 "View_AtmosphericFogDistanceOffset"
               OpMemberName %type_View 125 "View_AtmosphericFogSunDiscScale"
               OpMemberName %type_View 126 "View_AtmosphericFogSunDiscHalfApexAngleRadian"
               OpMemberName %type_View 127 "PrePadding_View_2492"
               OpMemberName %type_View 128 "View_AtmosphericFogSunDiscLuminance"
               OpMemberName %type_View 129 "View_AtmosphericFogRenderMask"
               OpMemberName %type_View 130 "View_AtmosphericFogInscatterAltitudeSampleNum"
               OpMemberName %type_View 131 "PrePadding_View_2520"
               OpMemberName %type_View 132 "PrePadding_View_2524"
               OpMemberName %type_View 133 "View_AtmosphericFogSunColor"
               OpMemberName %type_View 134 "View_NormalCurvatureToRoughnessScaleBias"
               OpMemberName %type_View 135 "View_RenderingReflectionCaptureMask"
               OpMemberName %type_View 136 "View_AmbientCubemapTint"
               OpMemberName %type_View 137 "View_AmbientCubemapIntensity"
               OpMemberName %type_View 138 "View_SkyLightParameters"
               OpMemberName %type_View 139 "PrePadding_View_2584"
               OpMemberName %type_View 140 "PrePadding_View_2588"
               OpMemberName %type_View 141 "View_SkyLightColor"
               OpMemberName %type_View 142 "View_SkyIrradianceEnvironmentMap"
               OpMemberName %type_View 143 "View_MobilePreviewMode"
               OpMemberName %type_View 144 "View_HMDEyePaddingOffset"
               OpMemberName %type_View 145 "View_ReflectionCubemapMaxMip"
               OpMemberName %type_View 146 "View_ShowDecalsMask"
               OpMemberName %type_View 147 "View_DistanceFieldAOSpecularOcclusionMode"
               OpMemberName %type_View 148 "View_IndirectCapsuleSelfShadowingIntensity"
               OpMemberName %type_View 149 "PrePadding_View_2744"
               OpMemberName %type_View 150 "PrePadding_View_2748"
               OpMemberName %type_View 151 "View_ReflectionEnvironmentRoughnessMixingScaleBiasAndLargestWeight"
               OpMemberName %type_View 152 "View_StereoPassIndex"
               OpMemberName %type_View 153 "View_GlobalVolumeCenterAndExtent"
               OpMemberName %type_View 154 "View_GlobalVolumeWorldToUVAddAndMul"
               OpMemberName %type_View 155 "View_GlobalVolumeDimension"
               OpMemberName %type_View 156 "View_GlobalVolumeTexelSize"
               OpMemberName %type_View 157 "View_MaxGlobalDistance"
               OpMemberName %type_View 158 "PrePadding_View_2908"
               OpMemberName %type_View 159 "View_CursorPosition"
               OpMemberName %type_View 160 "View_bCheckerboardSubsurfaceProfileRendering"
               OpMemberName %type_View 161 "PrePadding_View_2924"
               OpMemberName %type_View 162 "View_VolumetricFogInvGridSize"
               OpMemberName %type_View 163 "PrePadding_View_2940"
               OpMemberName %type_View 164 "View_VolumetricFogGridZParams"
               OpMemberName %type_View 165 "PrePadding_View_2956"
               OpMemberName %type_View 166 "View_VolumetricFogSVPosToVolumeUV"
               OpMemberName %type_View 167 "View_VolumetricFogMaxDistance"
               OpMemberName %type_View 168 "PrePadding_View_2972"
               OpMemberName %type_View 169 "View_VolumetricLightmapWorldToUVScale"
               OpMemberName %type_View 170 "PrePadding_View_2988"
               OpMemberName %type_View 171 "View_VolumetricLightmapWorldToUVAdd"
               OpMemberName %type_View 172 "PrePadding_View_3004"
               OpMemberName %type_View 173 "View_VolumetricLightmapIndirectionTextureSize"
               OpMemberName %type_View 174 "View_VolumetricLightmapBrickSize"
               OpMemberName %type_View 175 "View_VolumetricLightmapBrickTexelSize"
               OpMemberName %type_View 176 "View_StereoIPD"
               OpMemberName %type_View 177 "View_IndirectLightingCacheShowFlag"
               OpMemberName %type_View 178 "View_EyeToPixelSpreadAngle"
               OpName %View "View"
               OpName %type_Primitive "type.Primitive"
               OpMemberName %type_Primitive 0 "Primitive_LocalToWorld"
               OpMemberName %type_Primitive 1 "Primitive_InvNonUniformScaleAndDeterminantSign"
               OpMemberName %type_Primitive 2 "Primitive_ObjectWorldPositionAndRadius"
               OpMemberName %type_Primitive 3 "Primitive_WorldToLocal"
               OpMemberName %type_Primitive 4 "Primitive_PreviousLocalToWorld"
               OpMemberName %type_Primitive 5 "Primitive_PreviousWorldToLocal"
               OpMemberName %type_Primitive 6 "Primitive_ActorWorldPosition"
               OpMemberName %type_Primitive 7 "Primitive_UseSingleSampleShadowFromStationaryLights"
               OpMemberName %type_Primitive 8 "Primitive_ObjectBounds"
               OpMemberName %type_Primitive 9 "Primitive_LpvBiasMultiplier"
               OpMemberName %type_Primitive 10 "Primitive_DecalReceiverMask"
               OpMemberName %type_Primitive 11 "Primitive_PerObjectGBufferData"
               OpMemberName %type_Primitive 12 "Primitive_UseVolumetricLightmapShadowFromStationaryLights"
               OpMemberName %type_Primitive 13 "Primitive_DrawsVelocity"
               OpMemberName %type_Primitive 14 "Primitive_ObjectOrientation"
               OpMemberName %type_Primitive 15 "Primitive_NonUniformScale"
               OpMemberName %type_Primitive 16 "Primitive_LocalObjectBoundsMin"
               OpMemberName %type_Primitive 17 "Primitive_LightingChannelMask"
               OpMemberName %type_Primitive 18 "Primitive_LocalObjectBoundsMax"
               OpMemberName %type_Primitive 19 "Primitive_LightmapDataIndex"
               OpMemberName %type_Primitive 20 "Primitive_PreSkinnedLocalBounds"
               OpMemberName %type_Primitive 21 "Primitive_SingleCaptureIndex"
               OpMemberName %type_Primitive 22 "Primitive_OutputVelocity"
               OpMemberName %type_Primitive 23 "PrePadding_Primitive_420"
               OpMemberName %type_Primitive 24 "PrePadding_Primitive_424"
               OpMemberName %type_Primitive 25 "PrePadding_Primitive_428"
               OpMemberName %type_Primitive 26 "Primitive_CustomPrimitiveData"
               OpName %Primitive "Primitive"
               OpName %BoneMatrices "BoneMatrices"
               OpName %in_var_ATTRIBUTE0 "in.var.ATTRIBUTE0"
               OpName %in_var_ATTRIBUTE1 "in.var.ATTRIBUTE1"
               OpName %in_var_ATTRIBUTE2 "in.var.ATTRIBUTE2"
               OpName %in_var_ATTRIBUTE3 "in.var.ATTRIBUTE3"
               OpName %in_var_ATTRIBUTE4 "in.var.ATTRIBUTE4"
               OpName %in_var_ATTRIBUTE5 "in.var.ATTRIBUTE5"
               OpName %in_var_ATTRIBUTE13 "in.var.ATTRIBUTE13"
               OpName %out_var_TEXCOORD10_centroid "out.var.TEXCOORD10_centroid"
               OpName %out_var_TEXCOORD11_centroid "out.var.TEXCOORD11_centroid"
               OpName %out_var_COLOR0 "out.var.COLOR0"
               OpName %out_var_TEXCOORD0 "out.var.TEXCOORD0"
               OpName %out_var_VS_To_DS_Position "out.var.VS_To_DS_Position"
               OpName %Main "Main"
               OpDecorateString %in_var_ATTRIBUTE0 UserSemantic "ATTRIBUTE0"
               OpDecorateString %in_var_ATTRIBUTE1 UserSemantic "ATTRIBUTE1"
               OpDecorateString %in_var_ATTRIBUTE2 UserSemantic "ATTRIBUTE2"
               OpDecorateString %in_var_ATTRIBUTE3 UserSemantic "ATTRIBUTE3"
               OpDecorateString %in_var_ATTRIBUTE4 UserSemantic "ATTRIBUTE4"
               OpDecorateString %in_var_ATTRIBUTE5 UserSemantic "ATTRIBUTE5"
               OpDecorateString %in_var_ATTRIBUTE13 UserSemantic "ATTRIBUTE13"
               OpDecorateString %out_var_TEXCOORD10_centroid UserSemantic "TEXCOORD10_centroid"
               OpDecorateString %out_var_TEXCOORD11_centroid UserSemantic "TEXCOORD11_centroid"
               OpDecorateString %out_var_COLOR0 UserSemantic "COLOR0"
               OpDecorateString %out_var_TEXCOORD0 UserSemantic "TEXCOORD0"
               OpDecorateString %out_var_VS_To_DS_Position UserSemantic "VS_To_DS_Position"
               OpDecorate %in_var_ATTRIBUTE0 Location 0
               OpDecorate %in_var_ATTRIBUTE1 Location 1
               OpDecorate %in_var_ATTRIBUTE2 Location 2
               OpDecorate %in_var_ATTRIBUTE3 Location 3
               OpDecorate %in_var_ATTRIBUTE4 Location 4
               OpDecorate %in_var_ATTRIBUTE5 Location 5
               OpDecorate %in_var_ATTRIBUTE13 Location 13
               OpDecorate %out_var_TEXCOORD10_centroid Location 0
               OpDecorate %out_var_TEXCOORD11_centroid Location 1
               OpDecorate %out_var_COLOR0 Location 2
               OpDecorate %out_var_TEXCOORD0 Location 3
               OpDecorate %out_var_VS_To_DS_Position Location 5
               OpDecorate %View DescriptorSet 0
               OpDecorate %View Binding 1
               OpDecorate %Primitive DescriptorSet 0
               OpDecorate %Primitive Binding 2
               OpDecorate %BoneMatrices DescriptorSet 0
               OpDecorate %BoneMatrices Binding 0
               OpDecorate %_arr_v4float_uint_4 ArrayStride 16
               OpDecorate %_arr_v4float_uint_2 ArrayStride 16
               OpDecorate %_arr_v4float_uint_7 ArrayStride 16
               OpMemberDecorate %type_View 0 Offset 0
               OpMemberDecorate %type_View 0 MatrixStride 16
               OpMemberDecorate %type_View 0 ColMajor
               OpMemberDecorate %type_View 1 Offset 64
               OpMemberDecorate %type_View 1 MatrixStride 16
               OpMemberDecorate %type_View 1 ColMajor
               OpMemberDecorate %type_View 2 Offset 128
               OpMemberDecorate %type_View 2 MatrixStride 16
               OpMemberDecorate %type_View 2 ColMajor
               OpMemberDecorate %type_View 3 Offset 192
               OpMemberDecorate %type_View 3 MatrixStride 16
               OpMemberDecorate %type_View 3 ColMajor
               OpMemberDecorate %type_View 4 Offset 256
               OpMemberDecorate %type_View 4 MatrixStride 16
               OpMemberDecorate %type_View 4 ColMajor
               OpMemberDecorate %type_View 5 Offset 320
               OpMemberDecorate %type_View 5 MatrixStride 16
               OpMemberDecorate %type_View 5 ColMajor
               OpMemberDecorate %type_View 6 Offset 384
               OpMemberDecorate %type_View 6 MatrixStride 16
               OpMemberDecorate %type_View 6 ColMajor
               OpMemberDecorate %type_View 7 Offset 448
               OpMemberDecorate %type_View 7 MatrixStride 16
               OpMemberDecorate %type_View 7 ColMajor
               OpMemberDecorate %type_View 8 Offset 512
               OpMemberDecorate %type_View 8 MatrixStride 16
               OpMemberDecorate %type_View 8 ColMajor
               OpMemberDecorate %type_View 9 Offset 576
               OpMemberDecorate %type_View 9 MatrixStride 16
               OpMemberDecorate %type_View 9 ColMajor
               OpMemberDecorate %type_View 10 Offset 640
               OpMemberDecorate %type_View 10 MatrixStride 16
               OpMemberDecorate %type_View 10 ColMajor
               OpMemberDecorate %type_View 11 Offset 704
               OpMemberDecorate %type_View 11 MatrixStride 16
               OpMemberDecorate %type_View 11 ColMajor
               OpMemberDecorate %type_View 12 Offset 768
               OpMemberDecorate %type_View 12 MatrixStride 16
               OpMemberDecorate %type_View 12 ColMajor
               OpMemberDecorate %type_View 13 Offset 832
               OpMemberDecorate %type_View 13 MatrixStride 16
               OpMemberDecorate %type_View 13 ColMajor
               OpMemberDecorate %type_View 14 Offset 896
               OpMemberDecorate %type_View 15 Offset 908
               OpMemberDecorate %type_View 16 Offset 912
               OpMemberDecorate %type_View 17 Offset 924
               OpMemberDecorate %type_View 18 Offset 928
               OpMemberDecorate %type_View 19 Offset 940
               OpMemberDecorate %type_View 20 Offset 944
               OpMemberDecorate %type_View 21 Offset 956
               OpMemberDecorate %type_View 22 Offset 960
               OpMemberDecorate %type_View 23 Offset 972
               OpMemberDecorate %type_View 24 Offset 976
               OpMemberDecorate %type_View 25 Offset 992
               OpMemberDecorate %type_View 26 Offset 1008
               OpMemberDecorate %type_View 27 Offset 1020
               OpMemberDecorate %type_View 28 Offset 1024
               OpMemberDecorate %type_View 29 Offset 1036
               OpMemberDecorate %type_View 30 Offset 1040
               OpMemberDecorate %type_View 31 Offset 1052
               OpMemberDecorate %type_View 32 Offset 1056
               OpMemberDecorate %type_View 33 Offset 1068
               OpMemberDecorate %type_View 34 Offset 1072
               OpMemberDecorate %type_View 34 MatrixStride 16
               OpMemberDecorate %type_View 34 ColMajor
               OpMemberDecorate %type_View 35 Offset 1136
               OpMemberDecorate %type_View 35 MatrixStride 16
               OpMemberDecorate %type_View 35 ColMajor
               OpMemberDecorate %type_View 36 Offset 1200
               OpMemberDecorate %type_View 36 MatrixStride 16
               OpMemberDecorate %type_View 36 ColMajor
               OpMemberDecorate %type_View 37 Offset 1264
               OpMemberDecorate %type_View 37 MatrixStride 16
               OpMemberDecorate %type_View 37 ColMajor
               OpMemberDecorate %type_View 38 Offset 1328
               OpMemberDecorate %type_View 38 MatrixStride 16
               OpMemberDecorate %type_View 38 ColMajor
               OpMemberDecorate %type_View 39 Offset 1392
               OpMemberDecorate %type_View 39 MatrixStride 16
               OpMemberDecorate %type_View 39 ColMajor
               OpMemberDecorate %type_View 40 Offset 1456
               OpMemberDecorate %type_View 40 MatrixStride 16
               OpMemberDecorate %type_View 40 ColMajor
               OpMemberDecorate %type_View 41 Offset 1520
               OpMemberDecorate %type_View 41 MatrixStride 16
               OpMemberDecorate %type_View 41 ColMajor
               OpMemberDecorate %type_View 42 Offset 1584
               OpMemberDecorate %type_View 42 MatrixStride 16
               OpMemberDecorate %type_View 42 ColMajor
               OpMemberDecorate %type_View 43 Offset 1648
               OpMemberDecorate %type_View 43 MatrixStride 16
               OpMemberDecorate %type_View 43 ColMajor
               OpMemberDecorate %type_View 44 Offset 1712
               OpMemberDecorate %type_View 45 Offset 1724
               OpMemberDecorate %type_View 46 Offset 1728
               OpMemberDecorate %type_View 47 Offset 1740
               OpMemberDecorate %type_View 48 Offset 1744
               OpMemberDecorate %type_View 49 Offset 1756
               OpMemberDecorate %type_View 50 Offset 1760
               OpMemberDecorate %type_View 50 MatrixStride 16
               OpMemberDecorate %type_View 50 ColMajor
               OpMemberDecorate %type_View 51 Offset 1824
               OpMemberDecorate %type_View 51 MatrixStride 16
               OpMemberDecorate %type_View 51 ColMajor
               OpMemberDecorate %type_View 52 Offset 1888
               OpMemberDecorate %type_View 52 MatrixStride 16
               OpMemberDecorate %type_View 52 ColMajor
               OpMemberDecorate %type_View 53 Offset 1952
               OpMemberDecorate %type_View 54 Offset 1968
               OpMemberDecorate %type_View 55 Offset 1984
               OpMemberDecorate %type_View 56 Offset 1992
               OpMemberDecorate %type_View 57 Offset 2000
               OpMemberDecorate %type_View 58 Offset 2016
               OpMemberDecorate %type_View 59 Offset 2032
               OpMemberDecorate %type_View 60 Offset 2048
               OpMemberDecorate %type_View 61 Offset 2064
               OpMemberDecorate %type_View 62 Offset 2068
               OpMemberDecorate %type_View 63 Offset 2072
               OpMemberDecorate %type_View 64 Offset 2076
               OpMemberDecorate %type_View 65 Offset 2080
               OpMemberDecorate %type_View 66 Offset 2096
               OpMemberDecorate %type_View 67 Offset 2112
               OpMemberDecorate %type_View 68 Offset 2128
               OpMemberDecorate %type_View 69 Offset 2136
               OpMemberDecorate %type_View 70 Offset 2140
               OpMemberDecorate %type_View 71 Offset 2144
               OpMemberDecorate %type_View 72 Offset 2148
               OpMemberDecorate %type_View 73 Offset 2152
               OpMemberDecorate %type_View 74 Offset 2156
               OpMemberDecorate %type_View 75 Offset 2160
               OpMemberDecorate %type_View 76 Offset 2172
               OpMemberDecorate %type_View 77 Offset 2176
               OpMemberDecorate %type_View 78 Offset 2180
               OpMemberDecorate %type_View 79 Offset 2184
               OpMemberDecorate %type_View 80 Offset 2188
               OpMemberDecorate %type_View 81 Offset 2192
               OpMemberDecorate %type_View 82 Offset 2196
               OpMemberDecorate %type_View 83 Offset 2200
               OpMemberDecorate %type_View 84 Offset 2204
               OpMemberDecorate %type_View 85 Offset 2208
               OpMemberDecorate %type_View 86 Offset 2212
               OpMemberDecorate %type_View 87 Offset 2216
               OpMemberDecorate %type_View 88 Offset 2220
               OpMemberDecorate %type_View 89 Offset 2224
               OpMemberDecorate %type_View 90 Offset 2228
               OpMemberDecorate %type_View 91 Offset 2232
               OpMemberDecorate %type_View 92 Offset 2236
               OpMemberDecorate %type_View 93 Offset 2240
               OpMemberDecorate %type_View 94 Offset 2256
               OpMemberDecorate %type_View 95 Offset 2268
               OpMemberDecorate %type_View 96 Offset 2272
               OpMemberDecorate %type_View 97 Offset 2304
               OpMemberDecorate %type_View 98 Offset 2336
               OpMemberDecorate %type_View 99 Offset 2352
               OpMemberDecorate %type_View 100 Offset 2368
               OpMemberDecorate %type_View 101 Offset 2372
               OpMemberDecorate %type_View 102 Offset 2376
               OpMemberDecorate %type_View 103 Offset 2380
               OpMemberDecorate %type_View 104 Offset 2384
               OpMemberDecorate %type_View 105 Offset 2388
               OpMemberDecorate %type_View 106 Offset 2392
               OpMemberDecorate %type_View 107 Offset 2396
               OpMemberDecorate %type_View 108 Offset 2400
               OpMemberDecorate %type_View 109 Offset 2404
               OpMemberDecorate %type_View 110 Offset 2408
               OpMemberDecorate %type_View 111 Offset 2412
               OpMemberDecorate %type_View 112 Offset 2416
               OpMemberDecorate %type_View 113 Offset 2428
               OpMemberDecorate %type_View 114 Offset 2432
               OpMemberDecorate %type_View 115 Offset 2444
               OpMemberDecorate %type_View 116 Offset 2448
               OpMemberDecorate %type_View 117 Offset 2452
               OpMemberDecorate %type_View 118 Offset 2456
               OpMemberDecorate %type_View 119 Offset 2460
               OpMemberDecorate %type_View 120 Offset 2464
               OpMemberDecorate %type_View 121 Offset 2468
               OpMemberDecorate %type_View 122 Offset 2472
               OpMemberDecorate %type_View 123 Offset 2476
               OpMemberDecorate %type_View 124 Offset 2480
               OpMemberDecorate %type_View 125 Offset 2484
               OpMemberDecorate %type_View 126 Offset 2488
               OpMemberDecorate %type_View 127 Offset 2492
               OpMemberDecorate %type_View 128 Offset 2496
               OpMemberDecorate %type_View 129 Offset 2512
               OpMemberDecorate %type_View 130 Offset 2516
               OpMemberDecorate %type_View 131 Offset 2520
               OpMemberDecorate %type_View 132 Offset 2524
               OpMemberDecorate %type_View 133 Offset 2528
               OpMemberDecorate %type_View 134 Offset 2544
               OpMemberDecorate %type_View 135 Offset 2556
               OpMemberDecorate %type_View 136 Offset 2560
               OpMemberDecorate %type_View 137 Offset 2576
               OpMemberDecorate %type_View 138 Offset 2580
               OpMemberDecorate %type_View 139 Offset 2584
               OpMemberDecorate %type_View 140 Offset 2588
               OpMemberDecorate %type_View 141 Offset 2592
               OpMemberDecorate %type_View 142 Offset 2608
               OpMemberDecorate %type_View 143 Offset 2720
               OpMemberDecorate %type_View 144 Offset 2724
               OpMemberDecorate %type_View 145 Offset 2728
               OpMemberDecorate %type_View 146 Offset 2732
               OpMemberDecorate %type_View 147 Offset 2736
               OpMemberDecorate %type_View 148 Offset 2740
               OpMemberDecorate %type_View 149 Offset 2744
               OpMemberDecorate %type_View 150 Offset 2748
               OpMemberDecorate %type_View 151 Offset 2752
               OpMemberDecorate %type_View 152 Offset 2764
               OpMemberDecorate %type_View 153 Offset 2768
               OpMemberDecorate %type_View 154 Offset 2832
               OpMemberDecorate %type_View 155 Offset 2896
               OpMemberDecorate %type_View 156 Offset 2900
               OpMemberDecorate %type_View 157 Offset 2904
               OpMemberDecorate %type_View 158 Offset 2908
               OpMemberDecorate %type_View 159 Offset 2912
               OpMemberDecorate %type_View 160 Offset 2920
               OpMemberDecorate %type_View 161 Offset 2924
               OpMemberDecorate %type_View 162 Offset 2928
               OpMemberDecorate %type_View 163 Offset 2940
               OpMemberDecorate %type_View 164 Offset 2944
               OpMemberDecorate %type_View 165 Offset 2956
               OpMemberDecorate %type_View 166 Offset 2960
               OpMemberDecorate %type_View 167 Offset 2968
               OpMemberDecorate %type_View 168 Offset 2972
               OpMemberDecorate %type_View 169 Offset 2976
               OpMemberDecorate %type_View 170 Offset 2988
               OpMemberDecorate %type_View 171 Offset 2992
               OpMemberDecorate %type_View 172 Offset 3004
               OpMemberDecorate %type_View 173 Offset 3008
               OpMemberDecorate %type_View 174 Offset 3020
               OpMemberDecorate %type_View 175 Offset 3024
               OpMemberDecorate %type_View 176 Offset 3036
               OpMemberDecorate %type_View 177 Offset 3040
               OpMemberDecorate %type_View 178 Offset 3044
               OpDecorate %type_View Block
               OpMemberDecorate %type_Primitive 0 Offset 0
               OpMemberDecorate %type_Primitive 0 MatrixStride 16
               OpMemberDecorate %type_Primitive 0 ColMajor
               OpMemberDecorate %type_Primitive 1 Offset 64
               OpMemberDecorate %type_Primitive 2 Offset 80
               OpMemberDecorate %type_Primitive 3 Offset 96
               OpMemberDecorate %type_Primitive 3 MatrixStride 16
               OpMemberDecorate %type_Primitive 3 ColMajor
               OpMemberDecorate %type_Primitive 4 Offset 160
               OpMemberDecorate %type_Primitive 4 MatrixStride 16
               OpMemberDecorate %type_Primitive 4 ColMajor
               OpMemberDecorate %type_Primitive 5 Offset 224
               OpMemberDecorate %type_Primitive 5 MatrixStride 16
               OpMemberDecorate %type_Primitive 5 ColMajor
               OpMemberDecorate %type_Primitive 6 Offset 288
               OpMemberDecorate %type_Primitive 7 Offset 300
               OpMemberDecorate %type_Primitive 8 Offset 304
               OpMemberDecorate %type_Primitive 9 Offset 316
               OpMemberDecorate %type_Primitive 10 Offset 320
               OpMemberDecorate %type_Primitive 11 Offset 324
               OpMemberDecorate %type_Primitive 12 Offset 328
               OpMemberDecorate %type_Primitive 13 Offset 332
               OpMemberDecorate %type_Primitive 14 Offset 336
               OpMemberDecorate %type_Primitive 15 Offset 352
               OpMemberDecorate %type_Primitive 16 Offset 368
               OpMemberDecorate %type_Primitive 17 Offset 380
               OpMemberDecorate %type_Primitive 18 Offset 384
               OpMemberDecorate %type_Primitive 19 Offset 396
               OpMemberDecorate %type_Primitive 20 Offset 400
               OpMemberDecorate %type_Primitive 21 Offset 412
               OpMemberDecorate %type_Primitive 22 Offset 416
               OpMemberDecorate %type_Primitive 23 Offset 420
               OpMemberDecorate %type_Primitive 24 Offset 424
               OpMemberDecorate %type_Primitive 25 Offset 428
               OpMemberDecorate %type_Primitive 26 Offset 432
               OpDecorate %type_Primitive Block
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%mat4v4float = OpTypeMatrix %v4float 4
    %v3float = OpTypeVector %float 3
    %v2float = OpTypeVector %float 2
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_7 = OpConstant %uint 7
     %uint_4 = OpConstant %uint 4
      %v2int = OpTypeVector %int 2
     %uint_0 = OpConstant %uint 0
      %int_2 = OpConstant %int 2
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
%mat3v3float = OpTypeMatrix %v3float 3
%_arr_v2float_uint_2 = OpTypeArray %v2float %uint_2
    %float_0 = OpConstant %float 0
     %int_32 = OpConstant %int 32
     %uint_1 = OpConstant %uint 1
    %float_1 = OpConstant %float 1
     %uint_3 = OpConstant %uint 3
%_arr_v4float_uint_4 = OpTypeArray %v4float %uint_4
%mat3v4float = OpTypeMatrix %v4float 3
%_arr_v4float_uint_2 = OpTypeArray %v4float %uint_2
%type_buffer_image = OpTypeImage %float Buffer 2 0 0 1 Rgba32f
%_ptr_UniformConstant_type_buffer_image = OpTypePointer UniformConstant %type_buffer_image
%_arr_v4float_uint_7 = OpTypeArray %v4float %uint_7
  %type_View = OpTypeStruct %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %v3float %float %v3float %float %v3float %float %v4float %v4float %v3float %float %v3float %float %v3float %float %v3float %float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %v3float %float %mat4v4float %mat4v4float %mat4v4float %v4float %v4float %v2float %v2float %v4float %v4float %v4float %v4float %int %float %float %float %v4float %v4float %v4float %v2float %float %float %float %float %float %float %v3float %float %float %float %float %float %float %float %float %uint %uint %uint %uint %float %float %float %float %float %v4float %v3float %float %_arr_v4float_uint_2 %_arr_v4float_uint_2 %v4float %v4float %float %float %float %float %float %float %float %float %float %float %float %float %v3float %float %v3float %float %float %float %float %float %float %float %float %float %float %float %float %float %v4float %uint %uint %uint %uint %v4float %v3float %float %v4float %float %float %float %float %v4float %_arr_v4float_uint_7 %float %float %float %float %uint %float %float %float %v3float %int %_arr_v4float_uint_4 %_arr_v4float_uint_4 %float %float %float %float %v2int %float %float %v3float %float %v3float %float %v2float %float %float %v3float %float %v3float %float %v3float %float %v3float %float %float %float
%_ptr_Uniform_type_View = OpTypePointer Uniform %type_View
%type_Primitive = OpTypeStruct %mat4v4float %v4float %v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %float %float %float %float %v4float %v4float %v3float %uint %v3float %uint %v3float %int %uint %uint %uint %uint %_arr_v4float_uint_4
%_ptr_Uniform_type_Primitive = OpTypePointer Uniform %type_Primitive
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Input_v3float = OpTypePointer Input %v3float
     %v4uint = OpTypeVector %uint 4
%_ptr_Input_v4uint = OpTypePointer Input %v4uint
%_ptr_Input__arr_v2float_uint_2 = OpTypePointer Input %_arr_v2float_uint_2
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_ptr_Output__arr_v2float_uint_2 = OpTypePointer Output %_arr_v2float_uint_2
       %void = OpTypeVoid
         %59 = OpTypeFunction %void
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
       %bool = OpTypeBool
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_ptr_Function__arr_v2float_uint_2 = OpTypePointer Function %_arr_v2float_uint_2
       %View = OpVariable %_ptr_Uniform_type_View Uniform
  %Primitive = OpVariable %_ptr_Uniform_type_Primitive Uniform
%BoneMatrices = OpVariable %_ptr_UniformConstant_type_buffer_image UniformConstant
%in_var_ATTRIBUTE0 = OpVariable %_ptr_Input_v4float Input
%in_var_ATTRIBUTE1 = OpVariable %_ptr_Input_v3float Input
%in_var_ATTRIBUTE2 = OpVariable %_ptr_Input_v4float Input
%in_var_ATTRIBUTE3 = OpVariable %_ptr_Input_v4uint Input
%in_var_ATTRIBUTE4 = OpVariable %_ptr_Input_v4float Input
%in_var_ATTRIBUTE5 = OpVariable %_ptr_Input__arr_v2float_uint_2 Input
%in_var_ATTRIBUTE13 = OpVariable %_ptr_Input_v4float Input
%out_var_TEXCOORD10_centroid = OpVariable %_ptr_Output_v4float Output
%out_var_TEXCOORD11_centroid = OpVariable %_ptr_Output_v4float Output
%out_var_COLOR0 = OpVariable %_ptr_Output_v4float Output
%out_var_TEXCOORD0 = OpVariable %_ptr_Output__arr_v2float_uint_2 Output
%out_var_VS_To_DS_Position = OpVariable %_ptr_Output_v4float Output
         %67 = OpConstantNull %_arr_v2float_uint_2
         %68 = OpUndef %mat3v3float
         %69 = OpUndef %v4float
       %Main = OpFunction %void None %59
         %70 = OpLabel
         %71 = OpVariable %_ptr_Function__arr_v2float_uint_2 Function
         %72 = OpVariable %_ptr_Function__arr_v2float_uint_2 Function
         %73 = OpLoad %v4float %in_var_ATTRIBUTE0
         %74 = OpLoad %v3float %in_var_ATTRIBUTE1
         %75 = OpLoad %v4float %in_var_ATTRIBUTE2
         %76 = OpLoad %v4uint %in_var_ATTRIBUTE3
         %77 = OpLoad %v4float %in_var_ATTRIBUTE4
         %78 = OpLoad %_arr_v2float_uint_2 %in_var_ATTRIBUTE5
         %79 = OpLoad %v4float %in_var_ATTRIBUTE13
         %80 = OpAccessChain %_ptr_Uniform_v3float %View %int_32
         %81 = OpLoad %v3float %80
         %82 = OpCompositeExtract %float %77 0
         %83 = OpCompositeConstruct %v4float %82 %82 %82 %82
         %84 = OpCompositeExtract %uint %76 0
         %85 = OpBitcast %int %84
         %86 = OpIMul %int %85 %int_3
         %87 = OpBitcast %uint %86
         %88 = OpLoad %type_buffer_image %BoneMatrices
         %89 = OpImageFetch %v4float %88 %87 None
         %90 = OpIAdd %int %86 %int_1
         %91 = OpBitcast %uint %90
         %92 = OpImageFetch %v4float %88 %91 None
         %93 = OpIAdd %int %86 %int_2
         %94 = OpBitcast %uint %93
         %95 = OpImageFetch %v4float %88 %94 None
         %96 = OpFMul %v4float %83 %89
         %97 = OpFMul %v4float %83 %92
         %98 = OpFMul %v4float %83 %95
         %99 = OpCompositeExtract %float %77 1
        %100 = OpCompositeConstruct %v4float %99 %99 %99 %99
        %101 = OpCompositeExtract %uint %76 1
        %102 = OpBitcast %int %101
        %103 = OpIMul %int %102 %int_3
        %104 = OpBitcast %uint %103
        %105 = OpImageFetch %v4float %88 %104 None
        %106 = OpIAdd %int %103 %int_1
        %107 = OpBitcast %uint %106
        %108 = OpImageFetch %v4float %88 %107 None
        %109 = OpIAdd %int %103 %int_2
        %110 = OpBitcast %uint %109
        %111 = OpImageFetch %v4float %88 %110 None
        %112 = OpFMul %v4float %100 %105
        %113 = OpFMul %v4float %100 %108
        %114 = OpFMul %v4float %100 %111
        %115 = OpFAdd %v4float %96 %112
        %116 = OpFAdd %v4float %97 %113
        %117 = OpFAdd %v4float %98 %114
        %118 = OpCompositeExtract %float %77 2
        %119 = OpCompositeConstruct %v4float %118 %118 %118 %118
        %120 = OpCompositeExtract %uint %76 2
        %121 = OpBitcast %int %120
        %122 = OpIMul %int %121 %int_3
        %123 = OpBitcast %uint %122
        %124 = OpImageFetch %v4float %88 %123 None
        %125 = OpIAdd %int %122 %int_1
        %126 = OpBitcast %uint %125
        %127 = OpImageFetch %v4float %88 %126 None
        %128 = OpIAdd %int %122 %int_2
        %129 = OpBitcast %uint %128
        %130 = OpImageFetch %v4float %88 %129 None
        %131 = OpFMul %v4float %119 %124
        %132 = OpFMul %v4float %119 %127
        %133 = OpFMul %v4float %119 %130
        %134 = OpFAdd %v4float %115 %131
        %135 = OpFAdd %v4float %116 %132
        %136 = OpFAdd %v4float %117 %133
        %137 = OpCompositeExtract %float %77 3
        %138 = OpCompositeConstruct %v4float %137 %137 %137 %137
        %139 = OpCompositeExtract %uint %76 3
        %140 = OpBitcast %int %139
        %141 = OpIMul %int %140 %int_3
        %142 = OpBitcast %uint %141
        %143 = OpImageFetch %v4float %88 %142 None
        %144 = OpIAdd %int %141 %int_1
        %145 = OpBitcast %uint %144
        %146 = OpImageFetch %v4float %88 %145 None
        %147 = OpIAdd %int %141 %int_2
        %148 = OpBitcast %uint %147
        %149 = OpImageFetch %v4float %88 %148 None
        %150 = OpFMul %v4float %138 %143
        %151 = OpFMul %v4float %138 %146
        %152 = OpFMul %v4float %138 %149
        %153 = OpFAdd %v4float %134 %150
        %154 = OpFAdd %v4float %135 %151
        %155 = OpFAdd %v4float %136 %152
        %156 = OpCompositeConstruct %mat3v4float %153 %154 %155
        %157 = OpCompositeExtract %float %74 0
        %158 = OpCompositeExtract %float %74 1
        %159 = OpCompositeExtract %float %74 2
        %160 = OpCompositeConstruct %v4float %157 %158 %159 %float_0
        %161 = OpVectorTimesMatrix %v3float %160 %156
        %162 = OpCompositeInsert %mat3v3float %161 %68 0
        %163 = OpCompositeExtract %float %75 0
        %164 = OpCompositeExtract %float %75 1
        %165 = OpCompositeExtract %float %75 2
        %166 = OpCompositeConstruct %v4float %163 %164 %165 %float_0
        %167 = OpVectorTimesMatrix %v3float %166 %156
        %168 = OpCompositeInsert %mat3v3float %167 %162 2
        %169 = OpExtInst %v3float %1 Cross %167 %161
        %170 = OpCompositeExtract %float %75 3
        %171 = OpCompositeConstruct %v3float %170 %170 %170
        %172 = OpFMul %v3float %169 %171
        %173 = OpCompositeInsert %mat3v3float %172 %168 1
        %174 = OpCompositeExtract %float %73 0
        %175 = OpCompositeExtract %float %73 1
        %176 = OpCompositeExtract %float %73 2
        %177 = OpCompositeConstruct %v4float %174 %175 %176 %float_1
        %178 = OpVectorTimesMatrix %v3float %177 %156
        %179 = OpAccessChain %_ptr_Uniform_mat4v4float %Primitive %int_0
        %180 = OpAccessChain %_ptr_Uniform_v4float %Primitive %int_0 %uint_0
        %181 = OpLoad %v4float %180
        %182 = OpVectorShuffle %v3float %181 %181 0 1 2
        %183 = OpVectorShuffle %v3float %178 %178 0 0 0
        %184 = OpFMul %v3float %182 %183
        %185 = OpAccessChain %_ptr_Uniform_v4float %Primitive %int_0 %uint_1
        %186 = OpLoad %v4float %185
        %187 = OpVectorShuffle %v3float %186 %186 0 1 2
        %188 = OpVectorShuffle %v3float %178 %178 1 1 1
        %189 = OpFMul %v3float %187 %188
        %190 = OpFAdd %v3float %184 %189
        %191 = OpAccessChain %_ptr_Uniform_v4float %Primitive %int_0 %uint_2
        %192 = OpLoad %v4float %191
        %193 = OpVectorShuffle %v3float %192 %192 0 1 2
        %194 = OpVectorShuffle %v3float %178 %178 2 2 2
        %195 = OpFMul %v3float %193 %194
        %196 = OpFAdd %v3float %190 %195
        %197 = OpAccessChain %_ptr_Uniform_v4float %Primitive %int_0 %uint_3
        %198 = OpLoad %v4float %197
        %199 = OpVectorShuffle %v3float %198 %198 0 1 2
        %200 = OpFAdd %v3float %199 %81
        %201 = OpFAdd %v3float %196 %200
        %202 = OpCompositeExtract %float %201 0
        %203 = OpCompositeExtract %float %201 1
        %204 = OpCompositeExtract %float %201 2
        %205 = OpCompositeConstruct %v4float %202 %203 %204 %float_1
               OpStore %71 %78
               OpStore %72 %67
               OpBranch %206
        %206 = OpLabel
        %207 = OpPhi %int %int_0 %70 %208 %209
        %210 = OpSLessThan %bool %207 %int_2
               OpLoopMerge %211 %209 None
               OpBranchConditional %210 %209 %211
        %209 = OpLabel
        %212 = OpAccessChain %_ptr_Function_v2float %71 %207
        %213 = OpLoad %v2float %212
        %214 = OpAccessChain %_ptr_Function_v2float %72 %207
               OpStore %214 %213
        %208 = OpIAdd %int %207 %int_1
               OpBranch %206
        %211 = OpLabel
        %215 = OpLoad %_arr_v2float_uint_2 %72
        %216 = OpVectorShuffle %v4float %205 %205 4 5 6 3
        %217 = OpCompositeInsert %v4float %float_0 %69 3
        %218 = OpLoad %mat4v4float %179
        %219 = OpCompositeExtract %v4float %218 0
        %220 = OpVectorShuffle %v3float %219 %219 0 1 2
        %221 = OpCompositeExtract %v4float %218 1
        %222 = OpVectorShuffle %v3float %221 %221 0 1 2
        %223 = OpCompositeExtract %v4float %218 2
        %224 = OpVectorShuffle %v3float %223 %223 0 1 2
        %225 = OpCompositeConstruct %mat3v3float %220 %222 %224
        %226 = OpAccessChain %_ptr_Uniform_v4float %Primitive %int_1
        %227 = OpLoad %v4float %226
        %228 = OpCompositeExtract %float %227 0
        %229 = OpCompositeConstruct %v3float %228 %228 %228
        %230 = OpFMul %v3float %220 %229
        %231 = OpCompositeInsert %mat3v3float %230 %225 0
        %232 = OpCompositeExtract %float %227 1
        %233 = OpCompositeConstruct %v3float %232 %232 %232
        %234 = OpFMul %v3float %222 %233
        %235 = OpCompositeInsert %mat3v3float %234 %231 1
        %236 = OpCompositeExtract %float %227 2
        %237 = OpCompositeConstruct %v3float %236 %236 %236
        %238 = OpFMul %v3float %224 %237
        %239 = OpCompositeInsert %mat3v3float %238 %235 2
        %240 = OpMatrixTimesMatrix %mat3v3float %239 %173
        %241 = OpCompositeExtract %v3float %240 0
        %242 = OpCompositeExtract %v3float %240 2
        %243 = OpAccessChain %_ptr_Uniform_float %Primitive %int_1 %int_3
        %244 = OpLoad %float %243
        %245 = OpFMul %float %170 %244
        %246 = OpCompositeExtract %float %242 0
        %247 = OpCompositeExtract %float %242 1
        %248 = OpCompositeExtract %float %242 2
        %249 = OpCompositeConstruct %v4float %246 %247 %248 %245
        %250 = OpVectorShuffle %v4float %217 %241 4 5 6 3
               OpStore %out_var_TEXCOORD10_centroid %250
               OpStore %out_var_TEXCOORD11_centroid %249
               OpStore %out_var_COLOR0 %79
               OpStore %out_var_TEXCOORD0 %215
               OpStore %out_var_VS_To_DS_Position %216
               OpReturn
               OpFunctionEnd
