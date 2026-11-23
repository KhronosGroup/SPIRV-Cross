; SPIR-V
; Version: 1.3
; Generator: Hand written
; Bound: 40
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %a %b %va %vb %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
               OpName %a "a"
               OpName %b "b"
               OpName %va "va"
               OpName %vb "vb"
               OpName %FragColor "FragColor"
               OpDecorate %a Location 0
               OpDecorate %b Location 1
               OpDecorate %va Location 2
               OpDecorate %vb Location 3
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4
%_ptr_Input_float = OpTypePointer Input %float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
          %a = OpVariable %_ptr_Input_float Input
          %b = OpVariable %_ptr_Input_float Input
         %va = OpVariable %_ptr_Input_v4float Input
         %vb = OpVariable %_ptr_Input_v4float Input
  %FragColor = OpVariable %_ptr_Output_v4float Output
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
     %v4_one = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
    %v4_zero = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
       %main = OpFunction %void None %3
          %5 = OpLabel
         %la = OpLoad %float %a
         %lb = OpLoad %float %b
        %lva = OpLoad %v4float %va
        %lvb = OpLoad %v4float %vb
         %ne = OpFOrdNotEqual %bool %la %lb
        %une = OpFUnordNotEqual %bool %la %lb
        %vne = OpFOrdNotEqual %v4bool %lva %lvb
       %vune = OpFUnordNotEqual %v4bool %lva %lvb
         %s0 = OpSelect %float %ne %float_1 %float_0
         %s1 = OpSelect %float %une %float_1 %float_0
         %v0 = OpSelect %v4float %vne %v4_one %v4_zero
         %v1 = OpSelect %v4float %vune %v4_one %v4_zero
         %v2 = OpFAdd %v4float %v0 %v1
         %s2 = OpFAdd %float %s0 %s1
         %v3 = OpVectorTimesScalar %v4float %v2 %s2
               OpStore %FragColor %v3
               OpReturn
               OpFunctionEnd
