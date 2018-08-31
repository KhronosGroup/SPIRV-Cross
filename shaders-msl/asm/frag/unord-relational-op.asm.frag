; SPIR-V
; Version: 1.3
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 33
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %t0 "t0"
               OpName %a "a"
               OpName %t1 "t1"
               OpName %b "b"
               OpName %c21 "c21"
               OpName %c22 "c22"
               OpName %c23 "c23"
               OpName %c25 "c25"
               OpName %c27 "c27"
               OpName %c29 "c29"
               OpName %FragColor "FragColor"
               OpDecorate %t0 RelaxedPrecision
               OpDecorate %a RelaxedPrecision
               OpDecorate %a SpecId 1
               OpDecorate %t1 RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %b SpecId 2
               OpDecorate %FragColor RelaxedPrecision
               OpDecorate %FragColor Location 0
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
          %a = OpSpecConstant %float 1
          %b = OpSpecConstant %float 2
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %t0 = OpVariable %_ptr_Function_float Function
         %t1 = OpVariable %_ptr_Function_float Function
        %c21 = OpVariable %_ptr_Function_bool Function
        %c22 = OpVariable %_ptr_Function_bool Function
        %c23 = OpVariable %_ptr_Function_bool Function
        %c25 = OpVariable %_ptr_Function_bool Function
        %c27 = OpVariable %_ptr_Function_bool Function
        %c29 = OpVariable %_ptr_Function_bool Function
               OpStore %t0 %a
               OpStore %t1 %b
         %15 = OpFUnordEqual %bool %a %b
               OpStore %c21 %15
         %17 = OpFUnordNotEqual %bool %a %b
               OpStore %c22 %17
         %19 = OpFUnordLessThan %bool %a %b
               OpStore %c23 %19
         %21 = OpFUnordGreaterThan %bool %a %b
               OpStore %c25 %21
         %23 = OpFUnordLessThanEqual %bool %a %b
               OpStore %c27 %23
         %25 = OpFUnordGreaterThanEqual %bool %a %b
               OpStore %c29 %25
         %29 = OpLoad %float %t0
         %30 = OpLoad %float %t1
         %31 = OpFAdd %float %29 %30
         %32 = OpCompositeConstruct %v4float %31 %31 %31 %31
               OpStore %FragColor %32
               OpReturn
               OpFunctionEnd
