; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 11
; Bound: 13
; Schema: 0
               OpCapability Shader
               OpCapability SampleRateShading
               OpCapability FragmentBarycentricKHR
               OpExtension "SPV_KHR_fragment_shader_barycentric"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %value %gl_BaryCoordEXT
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_fragment_shader_barycentric"
               OpName %main "main"
               OpName %value "value"
               OpName %gl_BaryCoordEXT "gl_BaryCoordEXT"
               OpDecorate %value Location 0
               OpDecorate %gl_BaryCoordEXT BuiltIn BaryCoordKHR
               OpDecorate %gl_BaryCoordEXT Sample
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
%_ptr_Output_v3float = OpTypePointer Output %v3float
      %value = OpVariable %_ptr_Output_v3float Output
%_ptr_Input_v3float = OpTypePointer Input %v3float
%gl_BaryCoordEXT = OpVariable %_ptr_Input_v3float Input
       %main = OpFunction %void None %3
          %5 = OpLabel
         %12 = OpLoad %v3float %gl_BaryCoordEXT
               OpStore %value %12
               OpReturn
               OpFunctionEnd
