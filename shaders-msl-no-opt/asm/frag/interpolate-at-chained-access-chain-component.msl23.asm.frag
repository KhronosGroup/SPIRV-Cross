; SPIR-V
; Version: 1.3
; Generator: Khronos SPIR-V Tools Assembler; 0
; Bound: 26
; Schema: 0
               OpCapability Shader
               OpCapability InterpolationFunction
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %o %blk
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %o "o"
               OpName %Blk "Blk"
               OpMemberName %Blk 0 "a"
               OpName %blk "blk"
               OpDecorate %o Location 0
               OpDecorate %Blk Block
               OpDecorate %blk Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
          %o = OpVariable %_ptr_Output_v4float Output
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_v4float_uint_2 = OpTypeArray %v4float %uint_2
        %Blk = OpTypeStruct %_arr_v4float_uint_2
%_ptr_Input_Blk = OpTypePointer Input %Blk
        %blk = OpVariable %_ptr_Input_Blk Input
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
%_ptr_Input_float = OpTypePointer Input %float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %20 = OpAccessChain %_ptr_Input_float %blk %int_0 %int_1 %int_2
         %21 = OpAccessChain %_ptr_Input_float %20
         %22 = OpExtInst %float %1 InterpolateAtCentroid %21
         %23 = OpCompositeConstruct %v4float %22 %22 %22 %22
               OpStore %o %23
               OpReturn
               OpFunctionEnd
