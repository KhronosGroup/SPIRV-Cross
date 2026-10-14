; SPIR-V
; Version: 1.3
; Generator: Khronos Glslang Reference Front End; 11
; Bound: 31
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
%ptr_in_arr = OpTypePointer Input %_arr_v4float_uint_2
%_ptr_Input_Blk = OpTypePointer Input %Blk
        %blk = OpVariable %_ptr_Input_Blk Input
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_ptr_Input_v4float = OpTypePointer Input %v4float
    %v2float = OpTypeVector %float 2
%float_0_100000001 = OpConstant %float 0.100000001
         %28 = OpConstantComposite %v2float %float_0_100000001 %float_0_100000001
       %main = OpFunction %void None %3
          %5 = OpLabel
         %20_base = OpAccessChain %ptr_in_arr %blk %int_0

         %20 = OpAccessChain %_ptr_Input_v4float %20_base %int_1
         %21 = OpExtInst %v4float %1 InterpolateAtCentroid %20
         %22_base = OpAccessChain %ptr_in_arr %blk %int_0

         %22 = OpAccessChain %_ptr_Input_v4float %22_base %int_0
         %23 = OpExtInst %v4float %1 InterpolateAtSample %22 %int_1
         %24 = OpFAdd %v4float %21 %23
         %25_base = OpAccessChain %ptr_in_arr %blk %int_0

         %25 = OpAccessChain %_ptr_Input_v4float %25_base %int_1
         %29 = OpExtInst %v4float %1 InterpolateAtOffset %25 %28
         %30 = OpFAdd %v4float %24 %29
               OpStore %o %30
               OpReturn
               OpFunctionEnd
