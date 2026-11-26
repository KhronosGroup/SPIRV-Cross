; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 11
; Bound: 46
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 450
               OpName %main "main"
               OpName %Foo "Foo"
               OpMemberName %Foo 0 "b1"
               OpMemberName %Foo 1 "b2"
               OpMemberName %Foo 2 "b3"
               OpMemberName %Foo 3 "b4"
               OpName %foo "foo"
               OpName %CONST_BOOL "CONST_BOOL"
               OpName %b1 "b1"
               OpName %b2 "b2"
               OpName %b3 "b3"
               OpName %b4 "b4"
               OpDecorate %CONST_BOOL SpecId 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %bool = OpTypeBool
     %v2bool = OpTypeVector %bool 2
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_bool_uint_2 = OpTypeArray %bool %uint_2
%_arr_v2bool_uint_2 = OpTypeArray %v2bool %uint_2
        %Foo = OpTypeStruct %bool %v2bool %_arr_bool_uint_2 %_arr_v2bool_uint_2
	 %undef  = OpUndef %Foo
%_ptr_Function_Foo = OpTypePointer Function %Foo
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
 %CONST_BOOL = OpSpecConstantFalse %bool
%_ptr_Function_bool = OpTypePointer Function %bool
      %int_1 = OpConstant %int 1
         %21 = OpSpecConstantComposite %v2bool %CONST_BOOL %CONST_BOOL
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
      %int_2 = OpConstant %int 2
%_ptr_Function__arr_bool_uint_2 = OpTypePointer Function %_arr_bool_uint_2
      %int_3 = OpConstant %int 3
         %29 = OpSpecConstantComposite %v2bool %CONST_BOOL %CONST_BOOL
         %30 = OpSpecConstantComposite %v2bool %CONST_BOOL %CONST_BOOL
%_ptr_Function__arr_v2bool_uint_2 = OpTypePointer Function %_arr_v2bool_uint_2
       %main = OpFunction %void None %3
          %5 = OpLabel
        %foo = OpVariable %_ptr_Function_Foo Function
         %b1 = OpVariable %_ptr_Function_bool Function
         %b2 = OpVariable %_ptr_Function_v2bool Function
         %b3 = OpVariable %_ptr_Function__arr_bool_uint_2 Function
         %b4 = OpVariable %_ptr_Function__arr_v2bool_uint_2 Function
         %19 = OpAccessChain %_ptr_Function_bool %foo %int_0
               OpStore %19 %CONST_BOOL
         %23 = OpAccessChain %_ptr_Function_v2bool %foo %int_1
               OpStore %23 %21
         %25 = OpCompositeConstruct %_arr_bool_uint_2 %CONST_BOOL %CONST_BOOL
         %27 = OpAccessChain %_ptr_Function__arr_bool_uint_2 %foo %int_2
               OpStore %27 %25
         %31 = OpCompositeConstruct %_arr_v2bool_uint_2 %29 %30
         %33 = OpAccessChain %_ptr_Function__arr_v2bool_uint_2 %foo %int_3
               OpStore %33 %31
         %35 = OpAccessChain %_ptr_Function_bool %foo %int_0
         %36 = OpLoad %bool %35
               OpStore %b1 %36
         %38 = OpAccessChain %_ptr_Function_v2bool %foo %int_1
         %39 = OpLoad %v2bool %38
               OpStore %b2 %39
         %41 = OpAccessChain %_ptr_Function__arr_bool_uint_2 %foo %int_2
         %42 = OpLoad %_arr_bool_uint_2 %41
               OpStore %b3 %42
         %44 = OpAccessChain %_ptr_Function__arr_v2bool_uint_2 %foo %int_3
         %45 = OpLoad %_arr_v2bool_uint_2 %44
               OpStore %b4 %45
		   %foo1 = OpCompositeInsert %Foo %CONST_BOOL %undef 0
		   %foo2 = OpCompositeInsert %Foo %29 %foo1 1
		   %foo3 = OpCompositeInsert %Foo %25 %foo2 2
		   %foo4 = OpCompositeInsert %Foo %31 %foo3 3

		   %ext1 = OpCompositeExtract %bool %foo4 0
		   %ext2 = OpCompositeExtract %v2bool %foo4 1
		   %ext3 = OpCompositeExtract %_arr_bool_uint_2 %foo4 2
		   %ext4 = OpCompositeExtract %_arr_v2bool_uint_2 %foo4 3
               OpStore %b1 %ext1
               OpStore %b2 %ext2
               OpStore %b3 %ext3
               OpStore %b4 %ext4

               OpReturn
               OpFunctionEnd
