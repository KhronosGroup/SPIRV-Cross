; SPIR-V
; Version: 1.3
; Generator: Khronos Glslang Reference Front End; 11
; Bound: 104
; Schema: 0
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %tPos %gl_InvocationID %vPos %gl_out %gl_in %gl_TessLevelOuter %gl_TessLevelInner %pColor %gl_PrimitiveID %gl_PatchVerticesIn
               OpExecutionMode %main OutputVertices 4
               OpExecutionMode %main Quads
               OpExecutionMode %main SpacingFractionalOdd
               OpExecutionMode %main VertexOrderCw
               OpSource GLSL 450
               OpName %main "main"
               OpName %tPos "tPos"
               OpName %gl_InvocationID "gl_InvocationID"
               OpName %vPos "vPos"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %gl_out "gl_out"
               OpName %gl_PerVertex_0 "gl_PerVertex"
               OpMemberName %gl_PerVertex_0 0 "gl_Position"
               OpMemberName %gl_PerVertex_0 1 "gl_PointSize"
               OpMemberName %gl_PerVertex_0 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex_0 3 "gl_CullDistance"
               OpName %gl_in "gl_in"
               OpName %edge "edge"
               OpName %gl_TessLevelOuter "gl_TessLevelOuter"
               OpName %gl_TessLevelInner "gl_TessLevelInner"
               OpName %pColor "pColor"
               OpName %gl_PrimitiveID "gl_PrimitiveID"
               OpName %gl_PatchVerticesIn "gl_PatchVerticesIn"
               OpDecorate %tPos Location 0
               OpDecorate %gl_InvocationID BuiltIn InvocationId
               OpDecorate %vPos Location 0
               OpDecorate %gl_PerVertex Block
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex_0 Block
               OpMemberDecorate %gl_PerVertex_0 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex_0 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex_0 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex_0 3 BuiltIn CullDistance
               OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
               OpDecorate %gl_TessLevelOuter Patch
               OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner
               OpDecorate %gl_TessLevelInner Patch
               OpDecorate %pColor Patch
               OpDecorate %pColor Location 1
               OpDecorate %gl_PrimitiveID BuiltIn PrimitiveId
               OpDecorate %gl_PatchVerticesIn BuiltIn PatchVertices
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_v4float_uint_4 = OpTypeArray %v4float %uint_4
%_ptr_Output__arr_v4float_uint_4 = OpTypePointer Output %_arr_v4float_uint_4
       %tPos = OpVariable %_ptr_Output__arr_v4float_uint_4 Output
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%gl_InvocationID = OpVariable %_ptr_Input_int Input
    %uint_32 = OpConstant %uint 32
%_arr_v4float_uint_32 = OpTypeArray %v4float %uint_32
%_ptr_Input__arr_v4float_uint_32 = OpTypePointer Input %_arr_v4float_uint_32
       %vPos = OpVariable %_ptr_Input__arr_v4float_uint_32 Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
    %float_2 = OpConstant %float 2
%_ptr_Output_v4float = OpTypePointer Output %v4float
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_arr_gl_PerVertex_uint_4 = OpTypeArray %gl_PerVertex %uint_4
%_ptr_Output__arr_gl_PerVertex_uint_4 = OpTypePointer Output %_arr_gl_PerVertex_uint_4
     %gl_out = OpVariable %_ptr_Output__arr_gl_PerVertex_uint_4 Output
      %int_0 = OpConstant %int 0
%gl_PerVertex_0 = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_arr_gl_PerVertex_0_uint_32 = OpTypeArray %gl_PerVertex_0 %uint_32
%_ptr_Input__arr_gl_PerVertex_0_uint_32 = OpTypePointer Input %_arr_gl_PerVertex_0_uint_32
      %gl_in = OpVariable %_ptr_Input__arr_gl_PerVertex_0_uint_32 Input
     %uint_2 = OpConstant %uint 2
     %uint_0 = OpConstant %uint 0
       %bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %int_2 = OpConstant %int 2
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
%gl_TessLevelOuter = OpVariable %_ptr_Output__arr_float_uint_4 Output
%_ptr_Output_float = OpTypePointer Output %float
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
%gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output
     %pColor = OpVariable %_ptr_Output_v4float Output
%gl_PrimitiveID = OpVariable %_ptr_Input_int Input
%gl_PatchVerticesIn = OpVariable %_ptr_Input_int Input
    %float_0 = OpConstant %float 0
       %main = OpFunction %void None %3
          %5 = OpLabel
       %edge = OpVariable %_ptr_Function_float Function
         %16 = OpLoad %int %gl_InvocationID
         %21 = OpLoad %int %gl_InvocationID
         %23 = OpAccessChain %_ptr_Input_v4float %vPos %21
         %24 = OpLoad %v4float %23
         %26 = OpVectorTimesScalar %v4float %24 %float_2
         %28 = OpAccessChain %_ptr_Output_v4float %tPos %16
               OpStore %28 %26
         %35 = OpLoad %int %gl_InvocationID
         %41 = OpLoad %int %gl_InvocationID
         %42 = OpAccessChain %_ptr_Input_v4float %gl_in %41 %int_0
         %43 = OpLoad %v4float %42
         %44 = OpAccessChain %_ptr_Output_v4float %gl_out %35 %int_0
               OpStore %44 %43
               OpControlBarrier %uint_2 %uint_4 %uint_0
         %47 = OpLoad %int %gl_InvocationID
         %49 = OpIEqual %bool %47 %int_0
               OpSelectionMerge %51 None
               OpBranchConditional %49 %50 %51
         %50 = OpLabel
         %56 = OpAccessChain %_ptr_Output_v4float %tPos %int_1
         %57 = OpLoad %v4float %56
         %58 = OpAccessChain %_ptr_Output_v4float %tPos %int_0
         %59 = OpLoad %v4float %58
         %60 = OpFSub %v4float %57 %59
         %61 = OpExtInst %float %1 Length %60
         %62 = OpFAdd %float %float_1 %61
         %64 = OpAccessChain %_ptr_Output_v4float %tPos %int_3
         %65 = OpLoad %v4float %64
         %67 = OpAccessChain %_ptr_Output_v4float %tPos %int_2
         %68 = OpLoad %v4float %67
         %69 = OpFSub %v4float %65 %68
         %70 = OpExtInst %float %1 Length %69
         %71 = OpFAdd %float %62 %70
               OpStore %edge %71
         %75 = OpLoad %float %edge
         %77 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_0
               OpStore %77 %75
         %78 = OpLoad %float %edge
         %79 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_1
               OpStore %79 %78
         %80 = OpLoad %float %edge
         %81 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_2
               OpStore %81 %80
         %82 = OpLoad %float %edge
         %83 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_3
               OpStore %83 %82
         %87 = OpLoad %float %edge
         %88 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %int_0
               OpStore %88 %87
         %89 = OpLoad %float %edge
         %90 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %int_1
               OpStore %90 %89
               OpBranch %51
         %51 = OpLabel
         %91 = OpLoad %int %gl_InvocationID
         %92 = OpIEqual %bool %91 %int_3
               OpSelectionMerge %94 None
               OpBranchConditional %92 %93 %94
         %93 = OpLabel
         %97 = OpLoad %int %gl_PrimitiveID
         %98 = OpConvertSToF %float %97
        %100 = OpLoad %int %gl_PatchVerticesIn
        %101 = OpConvertSToF %float %100
        %103 = OpCompositeConstruct %v4float %98 %101 %float_0 %float_1
               OpStore %pColor %103
               OpBranch %94
         %94 = OpLabel
               OpReturn
               OpFunctionEnd
