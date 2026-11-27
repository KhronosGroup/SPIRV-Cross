; SPIR-V
; Version: 1.0
; Generator: Khronos SPIR-V Tools Assembler; 0
; Bound: 55
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %vid %pos %out %out1 %out2 %out3
               OpDecorate %vid BuiltIn VertexIndex
               OpDecorate %pos BuiltIn Position
               OpDecorate %out Location 0
               OpDecorate %out1 Location 1
               OpDecorate %out2 Location 2
               OpDecorate %out3 Location 3
         %void = OpTypeVoid
           %fn = OpTypeFunction %void
        %float = OpTypeFloat 32
       %float4 = OpTypeVector %float 4
         %uint = OpTypeInt 32 0
        %uint2 = OpTypeVector %uint 2
         %bool = OpTypeBool
        %bool2 = OpTypeVector %bool 2
            %S = OpTypeStruct %float4 %bool2
       %uint_2 = OpConstant %uint 2
    %arr_bool2 = OpTypeArray %bool2 %uint_2
           %S2 = OpTypeStruct %arr_bool2
     %ptr_in_u = OpTypePointer Input %uint
   %ptr_out_f4 = OpTypePointer Output %float4
   %ptr_out_u2 = OpTypePointer Output %uint2
          %vid = OpVariable %ptr_in_u Input
          %pos = OpVariable %ptr_out_f4 Output
          %out = OpVariable %ptr_out_u2 Output
         %out1 = OpVariable %ptr_out_u2 Output
         %out2 = OpVariable %ptr_out_u2 Output
         %out3 = OpVariable %ptr_out_u2 Output
           %f0 = OpConstant %float 0
           %f1 = OpConstant %float 1
           %u0 = OpConstant %uint 0
           %u1 = OpConstant %uint 1
       %corner = OpConstantComposite %float4 %f0 %f0 %f0 %f1
         %ones = OpConstantComposite %uint2 %u1 %u1
        %zeros = OpConstantComposite %uint2 %u0 %u0
       %bfalse = OpConstantFalse %bool
      %bfalse2 = OpConstantComposite %bool2 %bfalse %bfalse
        %undef = OpUndef %S
      %S_const = OpConstantComposite %S %corner %bfalse2
       %undef2 = OpUndef %S2
         %main = OpFunction %void None %fn
        %entry = OpLabel
            %v = OpLoad %uint %vid
           %c0 = OpUGreaterThan %bool %v %u0
           %c1 = OpUGreaterThan %bool %v %u1
           %b2 = OpCompositeConstruct %bool2 %c0 %c1
           %s0 = OpCompositeInsert %S %corner %undef 0
            %s = OpCompositeInsert %S %b2 %s0 1
            %p = OpCompositeExtract %float4 %s 0
            %b = OpCompositeExtract %bool2 %s 1
            %u = OpSelect %uint2 %b %ones %zeros
       %s_copy = OpCompositeInsert %S %b2 %S_const 1
           %bk = OpCompositeExtract %bool2 %s_copy 1
           %uk = OpSelect %uint2 %bk %ones %zeros
           %s2 = OpCompositeInsert %S2 %b2 %undef2 0 1
           %ba = OpCompositeExtract %bool2 %s2 0 1
           %ua = OpSelect %uint2 %ba %ones %zeros
          %arr = OpCompositeConstruct %arr_bool2 %b2 %bfalse2
           %s3 = OpCompositeInsert %S2 %arr %undef2 0
           %bw = OpCompositeExtract %bool2 %s3 0 0
           %uw = OpSelect %uint2 %bw %ones %zeros
               OpStore %pos %p
               OpStore %out %u
               OpStore %out1 %uk
               OpStore %out2 %ua
               OpStore %out3 %uw
               OpReturn
               OpFunctionEnd
