; SPIR-V
; Version: 1.0
; Generator: Google Shaderc over Glslang; 10
; Bound: 24
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 320
               OpSourceExtension "GL_GOOGLE_cpp_style_line_directive"
               OpSourceExtension "GL_GOOGLE_include_directive"
               OpName %main "main"
               OpName %foo "foo"
               OpMemberName %foo 0 "a"
               OpName %a "a"
               OpName %b "b"
               OpMemberDecorate %foo 0 RelaxedPrecision
               OpMemberDecorate %foo 0 Offset 0
               OpDecorate %b RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
%_arr_float_uint_3 = OpTypeArray %float %uint_3
        %foo = OpTypeStruct %_arr_float_uint_3
%_ptr_Function_foo = OpTypePointer Function %foo
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %16 = OpConstantComposite %_arr_float_uint_3 %float_1 %float_2 %float_3
         %17 = OpConstantComposite %foo %16
%_ptr_Function__arr_float_uint_3 = OpTypePointer Function %_arr_float_uint_3
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %main = OpFunction %void None %3
          %5 = OpLabel
          %a = OpVariable %_ptr_Function_foo Function
          %b = OpVariable %_ptr_Function__arr_float_uint_3 Function
               OpStore %a %17
         %22 = OpAccessChain %_ptr_Function__arr_float_uint_3 %a %int_0
         %23 = OpLoad %_arr_float_uint_3 %22
               OpStore %b %23
               OpReturn
               OpFunctionEnd
