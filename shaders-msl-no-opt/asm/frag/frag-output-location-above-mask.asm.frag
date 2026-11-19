; SPIR-V
; Version: 1.0
; Generator: Khronos SPIR-V Tools Assembler; 0
; Bound: 11
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpDecorate %FragColor Location 40
       %void = OpTypeVoid
         %fn = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %ptr_out = OpTypePointer Output %v4float
  %FragColor = OpVariable %ptr_out Output
    %float_1 = OpConstant %float 1
       %one4 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
       %main = OpFunction %void None %fn
      %entry = OpLabel
               OpStore %FragColor %one4
               OpReturn
               OpFunctionEnd
