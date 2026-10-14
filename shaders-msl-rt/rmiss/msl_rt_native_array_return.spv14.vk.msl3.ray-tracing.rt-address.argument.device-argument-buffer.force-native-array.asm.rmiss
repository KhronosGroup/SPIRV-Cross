OpCapability Shader
OpCapability RayTracingKHR
OpExtension "SPV_KHR_ray_tracing"
OpMemoryModel Logical GLSL450
OpEntryPoint MissKHR %main "main"
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%as = OpTypeAccelerationStructureKHR
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_2 = OpConstant %uint 2
%v2uint = OpTypeVector %uint 2
%addr0 = OpConstantComposite %v2uint %uint_0 %uint_0
%addr1 = OpConstantComposite %v2uint %uint_2 %uint_0
%arr2 = OpTypeArray %as %uint_2
%arr2_fn = OpTypeFunction %arr2
%main = OpFunction %void None %void_fn
%entry = OpLabel
%all = OpFunctionCall %arr2 %getScenes
%scene = OpCompositeExtract %as %all 0
OpReturn
OpFunctionEnd
%getScenes = OpFunction %arr2 None %arr2_fn
%get_entry = OpLabel
%scene0 = OpConvertUToAccelerationStructureKHR %as %addr0
%scene1 = OpConvertUToAccelerationStructureKHR %as %addr1
%loaded = OpCompositeConstruct %arr2 %scene0 %scene1
OpReturnValue %loaded
OpFunctionEnd
