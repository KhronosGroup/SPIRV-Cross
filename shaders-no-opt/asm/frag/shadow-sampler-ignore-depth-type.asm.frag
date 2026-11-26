; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 11
; Bound: 20
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %vUV
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %Blah "Blah"
               OpName %vUV "vUV"
               OpDecorate %FragColor Location 0
               OpDecorate %Blah Binding 0
               OpDecorate %Blah DescriptorSet 0
               OpDecorate %vUV Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
      %v4float = OpTypeVector %float 4
%_ptr_Output_float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_float Output
          %9 = OpTypeImage %float 2D 1 0 0 1 Unknown
         %10 = OpTypeSampledImage %9
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
       %Blah = OpVariable %_ptr_UniformConstant_10 UniformConstant
    %v3float = OpTypeVector %float 3
%_ptr_Input_v3float = OpTypePointer Input %v3float
        %vUV = OpVariable %_ptr_Input_v3float Input
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpLoad %10 %Blah
         %17 = OpLoad %v3float %vUV
         %18 = OpCompositeExtract %float %17 2
         %19 = OpImageSampleImplicitLod %v4float %13 %17
               OpStore %FragColor %19
               OpReturn
               OpFunctionEnd
