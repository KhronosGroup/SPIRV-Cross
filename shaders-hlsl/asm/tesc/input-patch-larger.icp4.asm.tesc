; SPIR-V
; Version: 1.3
; Generator: Khronos Glslang Reference Front End; 11
; Bound: 59
; Schema: 0
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %gl_out %gl_InvocationID %vPos %gl_TessLevelOuter %gl_TessLevelInner
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main Triangles
               OpExecutionMode %main SpacingFractionalEven
               OpExecutionMode %main VertexOrderCcw
               OpSource GLSL 450
               OpName %main "main"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %gl_out "gl_out"
               OpName %gl_InvocationID "gl_InvocationID"
               OpName %vPos "vPos"
               OpName %gl_TessLevelOuter "gl_TessLevelOuter"
               OpName %gl_TessLevelInner "gl_TessLevelInner"
               OpDecorate %gl_PerVertex Block
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_InvocationID BuiltIn InvocationId
               OpDecorate %vPos Location 0
               OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
               OpDecorate %gl_TessLevelOuter Patch
               OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner
               OpDecorate %gl_TessLevelInner Patch
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
     %uint_3 = OpConstant %uint 3
%_arr_gl_PerVertex_uint_3 = OpTypeArray %gl_PerVertex %uint_3
%_ptr_Output__arr_gl_PerVertex_uint_3 = OpTypePointer Output %_arr_gl_PerVertex_uint_3
     %gl_out = OpVariable %_ptr_Output__arr_gl_PerVertex_uint_3 Output
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%gl_InvocationID = OpVariable %_ptr_Input_int Input
      %int_0 = OpConstant %int 0
    %uint_32 = OpConstant %uint 32
%_arr_v4float_uint_32 = OpTypeArray %v4float %uint_32
%_ptr_Input__arr_v4float_uint_32 = OpTypePointer Input %_arr_v4float_uint_32
       %vPos = OpVariable %_ptr_Input__arr_v4float_uint_32 Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
      %int_1 = OpConstant %int 1
  %float_0_5 = OpConstant %float 0.5
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %bool = OpTypeBool
     %uint_4 = OpConstant %uint 4
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
%gl_TessLevelOuter = OpVariable %_ptr_Output__arr_float_uint_4 Output
    %float_4 = OpConstant %float 4
%_ptr_Output_float = OpTypePointer Output %float
      %int_2 = OpConstant %int 2
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
%gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpLoad %int %gl_InvocationID
         %25 = OpLoad %int %gl_InvocationID
         %27 = OpAccessChain %_ptr_Input_v4float %vPos %25
         %28 = OpLoad %v4float %27
         %29 = OpLoad %int %gl_InvocationID
         %31 = OpIAdd %int %29 %int_1
         %32 = OpAccessChain %_ptr_Input_v4float %vPos %31
         %33 = OpLoad %v4float %32
         %34 = OpFAdd %v4float %28 %33
         %36 = OpVectorTimesScalar %v4float %34 %float_0_5
         %38 = OpAccessChain %_ptr_Output_v4float %gl_out %19 %int_0
               OpStore %38 %36
         %39 = OpLoad %int %gl_InvocationID
         %41 = OpIEqual %bool %39 %int_0
               OpSelectionMerge %43 None
               OpBranchConditional %41 %42 %43
         %42 = OpLabel
         %50 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_0
               OpStore %50 %float_4
         %51 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_1
               OpStore %51 %float_4
         %53 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_2
               OpStore %53 %float_4
         %58 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %int_0
               OpStore %58 %float_4
               OpBranch %43
         %43 = OpLabel
               OpReturn
               OpFunctionEnd
