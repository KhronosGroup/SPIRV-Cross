; SPIR-V
; Version: 1.3
; Generator: Khronos Glslang Reference Front End; 11
; Bound: 78
; Schema: 0
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %gl_out %gl_InvocationID %gl_in %tUV %vUV %gl_TessLevelOuter %gl_TessLevelInner
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main Triangles
               OpExecutionMode %main SpacingEqual
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
               OpName %gl_PerVertex_0 "gl_PerVertex"
               OpMemberName %gl_PerVertex_0 0 "gl_Position"
               OpMemberName %gl_PerVertex_0 1 "gl_PointSize"
               OpMemberName %gl_PerVertex_0 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex_0 3 "gl_CullDistance"
               OpName %gl_in "gl_in"
               OpName %tUV "tUV"
               OpName %vUV "vUV"
               OpName %gl_TessLevelOuter "gl_TessLevelOuter"
               OpName %Params "Params"
               OpMemberName %Params 0 "level"
               OpName %u "u"
               OpName %gl_TessLevelInner "gl_TessLevelInner"
               OpDecorate %gl_PerVertex Block
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_InvocationID BuiltIn InvocationId
               OpDecorate %gl_PerVertex_0 Block
               OpMemberDecorate %gl_PerVertex_0 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex_0 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex_0 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex_0 3 BuiltIn CullDistance
               OpDecorate %tUV Location 0
               OpDecorate %vUV Location 0
               OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
               OpDecorate %gl_TessLevelOuter Patch
               OpDecorate %Params Block
               OpMemberDecorate %Params 0 Offset 0
               OpDecorate %u Binding 0
               OpDecorate %u DescriptorSet 0
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
%gl_PerVertex_0 = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
    %uint_32 = OpConstant %uint 32
%_arr_gl_PerVertex_0_uint_32 = OpTypeArray %gl_PerVertex_0 %uint_32
%_ptr_Input__arr_gl_PerVertex_0_uint_32 = OpTypePointer Input %_arr_gl_PerVertex_0_uint_32
      %gl_in = OpVariable %_ptr_Input__arr_gl_PerVertex_0_uint_32 Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
    %v2float = OpTypeVector %float 2
%_arr_v2float_uint_3 = OpTypeArray %v2float %uint_3
%_ptr_Output__arr_v2float_uint_3 = OpTypePointer Output %_arr_v2float_uint_3
        %tUV = OpVariable %_ptr_Output__arr_v2float_uint_3 Output
%_arr_v2float_uint_32 = OpTypeArray %v2float %uint_32
%_ptr_Input__arr_v2float_uint_32 = OpTypePointer Input %_arr_v2float_uint_32
        %vUV = OpVariable %_ptr_Input__arr_v2float_uint_32 Input
%_ptr_Input_v2float = OpTypePointer Input %v2float
%_ptr_Output_v2float = OpTypePointer Output %v2float
       %bool = OpTypeBool
     %uint_4 = OpConstant %uint 4
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
%gl_TessLevelOuter = OpVariable %_ptr_Output__arr_float_uint_4 Output
     %Params = OpTypeStruct %float
%_ptr_Uniform_Params = OpTypePointer Uniform %Params
          %u = OpVariable %_ptr_Uniform_Params Uniform
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_ptr_Output_float = OpTypePointer Output %float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
%gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpLoad %int %gl_InvocationID
         %26 = OpLoad %int %gl_InvocationID
         %28 = OpAccessChain %_ptr_Input_v4float %gl_in %26 %int_0
         %29 = OpLoad %v4float %28
         %31 = OpAccessChain %_ptr_Output_v4float %gl_out %19 %int_0
               OpStore %31 %29
         %36 = OpLoad %int %gl_InvocationID
         %40 = OpLoad %int %gl_InvocationID
         %42 = OpAccessChain %_ptr_Input_v2float %vUV %40
         %43 = OpLoad %v2float %42
         %45 = OpAccessChain %_ptr_Output_v2float %tUV %36
               OpStore %45 %43
         %46 = OpLoad %int %gl_InvocationID
         %48 = OpIEqual %bool %46 %int_0
               OpSelectionMerge %50 None
               OpBranchConditional %48 %49 %50
         %49 = OpLabel
         %59 = OpAccessChain %_ptr_Uniform_float %u %int_0
         %60 = OpLoad %float %59
         %62 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_0
               OpStore %62 %60
         %64 = OpAccessChain %_ptr_Uniform_float %u %int_0
         %65 = OpLoad %float %64
         %66 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_1
               OpStore %66 %65
         %68 = OpAccessChain %_ptr_Uniform_float %u %int_0
         %69 = OpLoad %float %68
         %70 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_2
               OpStore %70 %69
         %75 = OpAccessChain %_ptr_Uniform_float %u %int_0
         %76 = OpLoad %float %75
         %77 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %int_0
               OpStore %77 %76
               OpBranch %50
         %50 = OpLabel
               OpReturn
               OpFunctionEnd
