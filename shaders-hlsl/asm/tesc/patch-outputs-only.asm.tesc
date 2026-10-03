; SPIR-V
; Version: 1.3
; Generator: Khronos Glslang Reference Front End; 11
; Bound: 51
; Schema: 0
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %gl_InvocationID %pCenter %vPos %gl_TessLevelOuter %gl_TessLevelInner
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main Triangles
               OpExecutionMode %main SpacingEqual
               OpExecutionMode %main VertexOrderCcw
               OpSource GLSL 450
               OpName %main "main"
               OpName %gl_InvocationID "gl_InvocationID"
               OpName %pCenter "pCenter"
               OpName %vPos "vPos"
               OpName %gl_TessLevelOuter "gl_TessLevelOuter"
               OpName %gl_TessLevelInner "gl_TessLevelInner"
               OpDecorate %gl_InvocationID BuiltIn InvocationId
               OpDecorate %pCenter Patch
               OpDecorate %pCenter Location 0
               OpDecorate %vPos Location 0
               OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
               OpDecorate %gl_TessLevelOuter Patch
               OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner
               OpDecorate %gl_TessLevelInner Patch
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%gl_InvocationID = OpVariable %_ptr_Input_int Input
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
    %pCenter = OpVariable %_ptr_Output_v4float Output
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
%_arr_v4float_uint_32 = OpTypeArray %v4float %uint_32
%_ptr_Input__arr_v4float_uint_32 = OpTypePointer Input %_arr_v4float_uint_32
       %vPos = OpVariable %_ptr_Input__arr_v4float_uint_32 Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
    %float_3 = OpConstant %float 3
     %uint_4 = OpConstant %uint 4
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
%gl_TessLevelOuter = OpVariable %_ptr_Output__arr_float_uint_4 Output
%_ptr_Output_float = OpTypePointer Output %float
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
%gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output
       %main = OpFunction %void None %3
          %5 = OpLabel
          %9 = OpLoad %int %gl_InvocationID
         %12 = OpIEqual %bool %9 %int_0
               OpSelectionMerge %14 None
               OpBranchConditional %12 %13 %14
         %13 = OpLabel
         %25 = OpAccessChain %_ptr_Input_v4float %vPos %int_0
         %26 = OpLoad %v4float %25
         %28 = OpAccessChain %_ptr_Input_v4float %vPos %int_1
         %29 = OpLoad %v4float %28
         %30 = OpFAdd %v4float %26 %29
         %32 = OpAccessChain %_ptr_Input_v4float %vPos %int_2
         %33 = OpLoad %v4float %32
         %34 = OpFAdd %v4float %30 %33
         %36 = OpCompositeConstruct %v4float %float_3 %float_3 %float_3 %float_3
         %37 = OpFDiv %v4float %34 %36
               OpStore %pCenter %37
         %43 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_0
               OpStore %43 %float_3
         %44 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_1
               OpStore %44 %float_3
         %45 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_2
               OpStore %45 %float_3
         %50 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %int_0
               OpStore %50 %float_3
               OpBranch %14
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
